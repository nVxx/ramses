//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Windows_WGL_4_2_core/Platform_Windows_WGL_4_2_core.h"
#include "Context_WGL/Context_WGL.h"
#include "Device_GL/Device_GL.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Windows_WGL_4_2_core(rendererConfig);
    }

    Platform_Windows_WGL_4_2_core::Platform_Windows_WGL_4_2_core(const RendererConfig& rendererConfig)
        : Platform_Windows_WGL(rendererConfig)
    {
    }

    bool Platform_Windows_WGL_4_2_core::createDevice()
    {
        assert(m_context);
        auto device = std::make_unique<Device_GL>(*m_context, uint8_t{ 4 }, uint8_t{ 2 }, false, nullptr);
        if (device->init())
            m_device = std::move(device);

        return m_device.get() != nullptr;
    }

    bool Platform_Windows_WGL_4_2_core::createDeviceUploading()
    {
        assert(m_contextUploading);
        auto device = std::make_unique<Device_GL>(*m_contextUploading, uint8_t{ 4 }, uint8_t{ 2 }, false, nullptr);
        if (device->init())
            m_deviceUploading = std::move(device);

        return m_deviceUploading.get() != nullptr;
    }

    const Int32* Platform_Windows_WGL_4_2_core::getContextAttributes()
    {
        const Int32* returnValue = NULL;

        if(m_wglExtensions.isExtensionAvailable("create_context_profile"))
        {
            static const Int32 attribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                //WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                0
            };
            returnValue = attribs;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Windows_WGL_4_2_core::getContextAttributes:  could not load WGL context attributes");
        }

        return returnValue;
    }
}
