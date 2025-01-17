//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_ANDROID_EGL_H
#define RAMSES_PLATFORM_ANDROID_EGL_H


#include "Platform_Android/Window_Android.h"
#include "Platform_EGL/Platform_EGL.h"
#include "Context_EGL/Context_EGL.h"

namespace ramses_internal
{
    class Platform_Android_EGL : public Platform_EGL<Window_Android>
    {
    protected:
        explicit Platform_Android_EGL(const RendererConfig& rendererConfig);

        virtual bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override;
        virtual uint32_t getSwapInterval() const override;
    };
}

#endif
