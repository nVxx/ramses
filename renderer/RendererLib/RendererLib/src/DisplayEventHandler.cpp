//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayEventHandler.h"
#include "RendererAPI/IWindow.h"
#include "RendererAPI/IWindowEventHandler.h"
#include "RendererLib/EKeyModifier.h"
#include "RendererEventCollector.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    DisplayEventHandler::DisplayEventHandler(DisplayHandle displayHandle, RendererEventCollector& eventCollector)
        : m_displayHandle(displayHandle)
        , m_eventCollector(eventCollector)
    {
    }

    DisplayEventHandler::~DisplayEventHandler()
    {
    }

    /* WindowEvent handlers */

    void DisplayEventHandler::onKeyEvent(EKeyEventType event, uint32_t modifiers, EKeyCode keyCode)
    {
        LOG_TRACE(CONTEXT_RENDERER, "DisplayController::onKeyEvent: [display: " << m_displayHandle.asMemoryHandle() <<
            "; eventType: " << EnumToString(event) << "; modifiers: " << KeyModifierToString(modifiers) << "; key: " << EnumToString(keyCode) << "]");

        // collect renderer event
        KeyEvent keyEvent;
        keyEvent.type = event;
        keyEvent.keyCode = keyCode;
        keyEvent.modifier = modifiers;
        m_eventCollector.addWindowEvent(ERendererEventType::WindowKeyEvent, m_displayHandle, keyEvent);
    }

    void DisplayEventHandler::onMouseEvent(EMouseEventType event, int32_t posX, int32_t posY)
    {
        LOG_TRACE(CONTEXT_RENDERER, "DisplayController::onMouseEvent: [display: " << m_displayHandle.asMemoryHandle() <<
            "; eventType: " << EnumToString(event) << "; posX: " << posX << "; posY: " << posY << "]");

        // collect renderer event
        MouseEvent mouseEvent;
        mouseEvent.type = event;
        mouseEvent.pos.x = posX;
        mouseEvent.pos.y = posY;
        m_eventCollector.addWindowEvent(ERendererEventType::WindowMouseEvent, m_displayHandle, mouseEvent);
    }

    void DisplayEventHandler::onClose()
    {
        LOG_TRACE(CONTEXT_RENDERER, "DisplayController::onClose: [displayId: " << m_displayHandle.asMemoryHandle() << "]");
        m_eventCollector.addDisplayEvent(ERendererEventType::WindowClosed, m_displayHandle);
    }

    void DisplayEventHandler::onResize(uint32_t width, uint32_t height)
    {
       m_eventCollector.addWindowEvent(ERendererEventType::WindowResizeEvent, m_displayHandle, ResizeEvent{width, height});
    }

    void DisplayEventHandler::onWindowMove(int32_t posX, int32_t posY)
    {
        m_eventCollector.addWindowEvent(ERendererEventType::WindowMoveEvent, m_displayHandle, WindowMoveEvent{ posX, posY });
    }
}
