//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Wayland_Shell_EGL/Platform_Wayland_Shell_EGL_ES_3_0.h"
#include "Window_Wayland_Shell/Window_Wayland_Shell.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"

namespace ramses_internal
{
    Platform_Wayland_Shell_EGL_ES_3_0::Platform_Wayland_Shell_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_Wayland_EGL(rendererConfig)
    {
    }

    bool Platform_Wayland_Shell_EGL_ES_3_0::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_Wayland_Shell>(displayConfig, windowEventHandler, 0u, m_frameCallbackMaxPollTime);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }
}
