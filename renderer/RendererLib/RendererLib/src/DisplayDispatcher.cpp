//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayDispatcher.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "RendererLib/RendererCommandUtils.h"
#include "Platform_Base/Platform_Base.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal
{
    DisplayDispatcher::DisplayDispatcher(
        const RendererConfig& config,
        IRendererSceneEventSender& rendererSceneSender,
        IThreadAliveNotifier& notifier)
        : m_rendererConfig{ config }
        , m_rendererSceneSender{ rendererSceneSender }
        , m_notifier{ notifier }
    {
    }

    void DisplayDispatcher::dispatchCommands(RendererCommandBuffer& cmds)
    {
        m_tmpCommands.clear();
        cmds.swapCommands(m_tmpCommands);
        dispatchCommands(m_tmpCommands);
    }

    void DisplayDispatcher::dispatchCommands(RendererCommands& cmds)
    {
        // log only if there are commands other than scene update or periodic log
        const bool logCommands = std::any_of(cmds.cbegin(), cmds.cend(), [&](const auto& c) {
            return !absl::holds_alternative<RendererCommand::UpdateScene>(c) && !absl::holds_alternative<RendererCommand::LogInfo>(c);
        });
        if (logCommands)
            LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: dispatching {} commands (only other than scene update commands will be logged)", cmds.size());

        std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
        for (auto&& cmd : cmds)
        {
            if (logCommands && !absl::holds_alternative<RendererCommand::UpdateScene>(cmd))
                LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: dispatching command [{}]", RendererCommandUtils::ToString(cmd));

            preprocessCommand(cmd);
            dispatchCommand(std::move(cmd));
        }

        for (auto& display : m_displays)
        {
            auto& displayBundle = *display.second.displayBundle;
            auto& pendingCmds = display.second.pendingCommands;
            if (!pendingCmds.empty())
                displayBundle.pushAndConsumeCommands(pendingCmds);
        }

        if (++m_cmdDispatchLoopsSinceLastEventDispatch > 300)
        {
            LOG_WARN_P(CONTEXT_RENDERER, "DisplayDispatcher: detected no renderer events dispatched in more than {} loops, this could result in wrong behavior!"
                " Use RamsesRenderer::dispatchEvents regularly to avoid this problem.", m_cmdDispatchLoopsSinceLastEventDispatch);
            m_cmdDispatchLoopsSinceLastEventDispatch = 0; // do not spam
        }

        // TODO vaclav remove, for debugging if display thread stuck
        const auto estNumFramesWithinWatchdogTimeoutPeriod = std::chrono::seconds{ 1 } / m_generalMinFrameDuration;
        if (m_threadedDisplays && m_displayThreadsUpdating && m_loopCounter++ > estNumFramesWithinWatchdogTimeoutPeriod / 2)
        {
            m_loopCounter = 0;
            for (auto& display : m_displays)
            {
                const auto frameCounter = display.second.displayThread->getFrameCounter();
                if (display.second.lastFrameCounter == frameCounter)
                    LOG_WARN_P(CONTEXT_RENDERER, "Display {} potentially stuck at trace ID {}", display.first, display.second.displayBundle->traceId());
                display.second.lastFrameCounter = frameCounter;
            }
        }
    }

    void DisplayDispatcher::doOneLoop(std::chrono::microseconds sleepTime)
    {
        std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
        assert(!m_threadedDisplays);
        assert(std::all_of(m_displays.cbegin(), m_displays.cend(), [](const auto& d) { return !d.second.displayThread; }));
        for (auto& display : m_displays)
        {
            // in non-threaded mode overwrite the TLS log prefix before each display update
            ThreadLocalLog::SetPrefix(static_cast<int>(display.first.asMemoryHandle()));

            if (m_displays.size() > 1u)
                display.second.displayBundle->enableContext();
            display.second.displayBundle->doOneLoop(m_loopMode, sleepTime);
        }
    }

    void DisplayDispatcher::preprocessCommand(const RendererCommand::Variant& cmd)
    {
        if (absl::holds_alternative<RendererCommand::CreateDisplay>(cmd))
        {
            // create display thread
            const auto& cmdData = absl::get<RendererCommand::CreateDisplay>(cmd);
            const auto displayHandle = cmdData.display;
            assert(m_displays.count(displayHandle) == 0u);
            auto displayBundle = createDisplayBundle(displayHandle);
            {
                if (displayBundle.displayThread)
                {
                    displayBundle.displayThread->setLoopMode(m_loopMode);
                    std::chrono::microseconds minFrameDuration = m_generalMinFrameDuration;
                    // use display specific value if set
                    if (m_minFrameDurationsPerDisplay.count(displayHandle))
                    {
                        minFrameDuration = m_minFrameDurationsPerDisplay[displayHandle];
                        m_minFrameDurationsPerDisplay.erase(displayHandle);
                    }
                    displayBundle.displayThread->setMinFrameDuration(minFrameDuration);
                    if (m_displayThreadsUpdating)
                        displayBundle.displayThread->startUpdating();
                }
                m_displays.insert(std::make_pair(displayHandle, std::move(displayBundle)));
            }

            // copy and push stashed broadcast commands to new display so that display receives all relevant commands
            // received until now (e.g. un/publish, limits, SC, etc.)
            RendererCommands stashedCommands{ m_stashedBroadcastCommandsForNewDisplays.size() };
            std::transform(m_stashedBroadcastCommandsForNewDisplays.cbegin(), m_stashedBroadcastCommandsForNewDisplays.cend(), stashedCommands.begin(), [](const auto& c)
            {
                return RendererCommandUtils::Copy(c);
            });
            LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: pushing {} stashed broadcast commands to newly created display {}", stashedCommands.size(), displayHandle);
            m_displays[displayHandle].displayBundle->pushAndConsumeCommands(stashedCommands);

            // push commands stashed for this specific display (e.g. set scene state)
            LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: pushing {} stashed commands to newly created display {}", m_stashedCommandsForNewDisplays[displayHandle].size(), displayHandle);
            m_displays[displayHandle].displayBundle->pushAndConsumeCommands(m_stashedCommandsForNewDisplays[displayHandle]);
            m_stashedCommandsForNewDisplays.erase(displayHandle);
        }
        else if (absl::holds_alternative<RendererCommand::SetSceneMapping>(cmd))
        {
            // set scene ownership so that future commands are dispatched to its display
            const auto& cmdData = absl::get<RendererCommand::SetSceneMapping>(cmd);
            m_sceneDisplayTrackerForCommands.setSceneOwnership(cmdData.scene, cmdData.display);
        }
        else if (absl::holds_alternative<RendererCommand::ReceiveScene>(cmd))
        {
            // Special handling of referenced scenes, refScenes are fully handled by internal logic within DisplayBundle/SceneRefLogic,
            // therefore their ownership is not known at dispatcher level. When a subscription of referenced scene arrives its master
            // is queried from a thread-safe shared ownership registry.
            if (!m_sceneDisplayTrackerForCommands.determineDisplayFromRendererCommand(cmd)->isValid())
            {
                const auto refScene = absl::get<RendererCommand::ReceiveScene>(cmd).info.sceneID;
                LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: missing scene {} display ownership when processing {}, assuming a referenced scene.", refScene, RendererCommandUtils::ToString(cmd));
                for (const auto& display : m_displays)
                {
                    const auto& displayBundle = *display.second.displayBundle;
                    const auto masterScene = displayBundle.findMasterSceneForReferencedScene(refScene);
                    if (masterScene.isValid())
                    {
                        const auto masterDisplay = m_sceneDisplayTrackerForCommands.getSceneOwnership(masterScene);
                        LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: found master scene {} for referenced scene {} when processing {}, setting display ownership to display {}",
                            masterScene, refScene, RendererCommandUtils::ToString(cmd), masterDisplay);
                        m_sceneDisplayTrackerForCommands.setSceneOwnership(refScene, masterDisplay);
                    }
                }
                if (!m_sceneDisplayTrackerForCommands.getSceneOwnership(refScene).isValid())
                {
                    LOG_ERROR_P(CONTEXT_RENDERER, "DisplayDispatcher: could not find master scene for referenced scene {} when processing {}", refScene, RendererCommandUtils::ToString(cmd));
                }
            }
        }
    }

    DisplayDispatcher::Display DisplayDispatcher::createDisplayBundle(DisplayHandle displayHandle)
    {
        Display bundle;
        LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: creating platform for display {}", displayHandle);
        bundle.platform.reset(Platform_Base::CreatePlatform(m_rendererConfig));

        LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: creating display bundle of components for display {}", displayHandle);
        const bool firstDisplay = m_displays.empty(); // allow time report and KPI monitoring only for 1st display
        bundle.displayBundle = DisplayBundleShared{ std::make_unique<DisplayBundle>(
            displayHandle,
            m_rendererSceneSender,
            *bundle.platform,
            m_notifier,
            m_rendererConfig.getRenderThreadLoopTimingReportingPeriod(),
            firstDisplay,
            firstDisplay ? m_rendererConfig.getKPIFileName() : String{})
        };
        if (m_threadedDisplays)
        {
            LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher: creating update/render thread for display {}", displayHandle);
            bundle.displayThread = std::make_unique<DisplayThread>(bundle.displayBundle, displayHandle, m_notifier);
        }

        return bundle;
    }

    void DisplayDispatcher::dispatchCommand(RendererCommand::Variant&& cmd)
    {
        const auto cmdDisplay = m_sceneDisplayTrackerForCommands.determineDisplayFromRendererCommand(cmd);
        if (cmdDisplay)
        {
            if (m_displays.count(*cmdDisplay) == 0)
            {
                if (absl::holds_alternative<RendererCommand::SetSceneMapping>(cmd) ||
                    absl::holds_alternative<RendererCommand::SetSceneState>(cmd))
                {
                    // Special case for commands that are to be dispatched only after their corresponding display is created, therefore cannot fail.
                    // This makes it possible that scene mapping/state can be set before display is even created.
                    LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher cannot dispatch command yet, display does not exist, will dispatch when display created. Command=[{}]", RendererCommandUtils::ToString(cmd));
                    m_stashedCommandsForNewDisplays[*cmdDisplay].push_back(std::move(cmd));
                }
                else
                {
                    // cannot dispatch, generate fail event
                    LOG_ERROR_P(CONTEXT_RENDERER, "DisplayDispatcher cannot dispatch command, display unknown. Command=[{}]", RendererCommandUtils::ToString(cmd));
                    auto failEvent = RendererCommandUtils::GenerateFailEventForCommand(cmd);
                    if (failEvent.eventType != ERendererEventType::Invalid)
                    {
                        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
                        m_injectedRendererEvents.push_back(std::move(failEvent));
                    }
                }
            }
            else
            {
                // dispatch command to display
                assert(m_displays.count(*cmdDisplay));
                m_displays[*cmdDisplay].pendingCommands.push_back(std::move(cmd));
            }
        }
        else
        {
            // command is to be broadcast, dispatch a copy to each display
            for (auto& display : m_displays)
                display.second.pendingCommands.push_back(RendererCommandUtils::Copy(cmd));

            RendererCommandUtils::AddAndConsolidateCommandToStash(std::move(cmd), m_stashedBroadcastCommandsForNewDisplays);
        }
    }

    bool DisplayDispatcher::isSceneStateChangeEmittedFromOwningDisplay(SceneId sceneId, DisplayHandle emittingDisplay) const
    {
        const bool isFirstDisplay = (m_displays.cbegin()->first == emittingDisplay);
        const auto owningDisplay = m_sceneDisplayTrackerForEvents.getSceneOwnership(sceneId);
        if (owningDisplay.isValid())
            return owningDisplay == emittingDisplay;
        else
            return isFirstDisplay;
    }

    void DisplayDispatcher::dispatchRendererEvents(RendererEventVector& events)
    {
        {
            std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
            std::vector<DisplayHandle> destroyedDisplays;
            for (auto& display : m_displays)
            {
                m_tmpEvents.clear();
                display.second.displayBundle->dispatchRendererEvents(m_tmpEvents);
                for (auto&& evt : m_tmpEvents)
                {
                    if (evt.eventType == ERendererEventType::DisplayDestroyed)
                    {
                        assert(evt.displayHandle == display.first);
                        destroyedDisplays.push_back(evt.displayHandle);
                        m_sceneDisplayTrackerForCommands.unregisterDisplay(evt.displayHandle);
                        m_sceneDisplayTrackerForEvents.unregisterDisplay(evt.displayHandle);
                    }

                    events.push_back(std::move(evt));
                }
            }

            for (const auto& display : destroyedDisplays)
                m_displays.erase(display);

            m_cmdDispatchLoopsSinceLastEventDispatch = 0;
        }

        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
        events.insert(events.end(), std::make_move_iterator(m_injectedRendererEvents.begin()), std::make_move_iterator(m_injectedRendererEvents.end()));
        m_injectedRendererEvents.clear();
    }

    void DisplayDispatcher::dispatchSceneControlEvents(RendererEventVector& events)
    {
        {
            std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
            for (auto& display : m_displays)
            {
                m_tmpEvents.clear();
                display.second.displayBundle->dispatchSceneControlEvents(m_tmpEvents);

                for (auto&& evt : m_tmpEvents)
                {
                    if (evt.eventType == ERendererEventType::SceneStateChanged)
                    {
                        // Available state can mean published or unsubscribed, the first will come from all displays (result of broadcast command publish),
                        // the latter can only come from an owning display. To distinguish that and also avoid races with ownership of commands (aync flow of commands and events),
                        // events have own tracking of ownership - scene is owned by display simply when it reached Ready on that display.
                        // Scene state events are emitted only if coming from owning display or first display if not owned by any display.
                        if (evt.state == RendererSceneState::Ready)
                            m_sceneDisplayTrackerForEvents.setSceneOwnership(evt.sceneId, display.first);
                        if (isSceneStateChangeEmittedFromOwningDisplay(evt.sceneId, display.first))
                            events.push_back(std::move(evt));
                        else
                            LOG_INFO_P(CONTEXT_RENDERER, "DisplayDispatcher::dispatchSceneControlEvents: filtering scene state change event from non-owner display {}, scene {} change state to {}.",
                                display.first, evt.sceneId, EnumToString(evt.state));
                    }
                    else
                        events.push_back(std::move(evt));
                }
            }
        }

        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
        events.insert(events.end(), std::make_move_iterator(m_injectedSceneControlEvents.begin()), std::make_move_iterator(m_injectedSceneControlEvents.end()));
        m_injectedSceneControlEvents.clear();
    }

    void DisplayDispatcher::injectRendererEvent(RendererEvent&& event)
    {
        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
        m_injectedRendererEvents.push_back(std::move(event));
    }

    void DisplayDispatcher::injectSceneControlEvent(RendererEvent&& event)
    {
        std::lock_guard<std::mutex> g{ m_injectedEventsLock };
        m_injectedSceneControlEvents.push_back(std::move(event));
    }

    void DisplayDispatcher::startDisplayThreadsUpdating()
    {
        std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
        // there cannot be any displays if starting thread for first time
        assert(m_threadedDisplays || m_displays.empty());
        // all displays created must be threaded
        assert(std::all_of(m_displays.cbegin(), m_displays.cend(), [](const auto& d) { return d.second.displayThread.get() != nullptr; }));
        m_threadedDisplays = true;
        m_displayThreadsUpdating = true;
        for (auto& display : m_displays)
            display.second.displayThread->startUpdating();
    }

    void DisplayDispatcher::stopDisplayThreadsUpdating()
    {
        std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
        assert(std::all_of(m_displays.cbegin(), m_displays.cend(), [](const auto& d)->bool { return d.second.displayThread != nullptr; }));
        m_displayThreadsUpdating = false;
        for (auto& display : m_displays)
            display.second.displayThread->stopUpdating();
    }

    void DisplayDispatcher::setLoopMode(ELoopMode loopMode)
    {
        std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
        m_loopMode = loopMode;
        for (auto& display : m_displays)
            if (display.second.displayThread)
                display.second.displayThread->setLoopMode(loopMode);
    }

    void DisplayDispatcher::setMinFrameDuration(std::chrono::microseconds minFrameDuration, DisplayHandle display)
    {
        std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
        // keep value for possible later display creation
        m_minFrameDurationsPerDisplay[display] = minFrameDuration;

        // try to set value directly if already created/running
        const auto& displayIter = m_displays.find(display);
        if (displayIter != m_displays.end())
        {
            if (displayIter->second.displayThread)
                displayIter->second.displayThread->setMinFrameDuration(minFrameDuration);
        }
    }

    void DisplayDispatcher::setMinFrameDuration(std::chrono::microseconds minFrameDuration )
    {
        std::lock_guard<std::mutex> lock{ m_displaysAccessLock };
        m_generalMinFrameDuration = minFrameDuration;
        for (auto& display : m_displays)
            if (display.second.displayThread)
                display.second.displayThread->setMinFrameDuration(minFrameDuration);
    }

    IEmbeddedCompositingManager& DisplayDispatcher::getECManager(DisplayHandle display)
    {
        assert(!m_threadedDisplays);
        assert(m_displays.count(display) != 0);
        return m_displays[display].displayBundle->getECManager();
    }

    IEmbeddedCompositor& DisplayDispatcher::getEC(DisplayHandle display)
    {
        assert(!m_threadedDisplays);
        assert(m_displays.count(display) != 0);
        return m_displays[display].displayBundle->getEC();
    }

    bool DisplayDispatcher::hasSystemCompositorController() const
    {
        assert(!m_threadedDisplays);
        assert(m_displays.size() != 0);
        const IDisplayBundle& displayBundle = *m_displays.cbegin()->second.displayBundle;
        return displayBundle.hasSystemCompositorController();
    }

    const RendererConfig& DisplayDispatcher::getRendererConfig() const
    {
        return m_rendererConfig;
    }
}
