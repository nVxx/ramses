//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayConfigImpl.h"
#include "RendererLib/RendererConfigUtils.h"

namespace ramses
{
    DisplayConfigImpl::DisplayConfigImpl(int32_t argc, char const* const* argv)
        : StatusObjectImpl()
    {
        ramses_internal::CommandLineParser parser(argc, argv);
        ramses_internal::RendererConfigUtils::ApplyValuesFromCommandLine(parser, m_internalConfig);
    }

    status_t DisplayConfigImpl::setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        if (width == 0u || height == 0u)
        {
            return addErrorEntry("DisplayConfig::setWindowRectangle failed - width and/or height cannot be 0!");
        }

        m_internalConfig.setWindowPositionX(x);
        m_internalConfig.setWindowPositionY(y);
        m_internalConfig.setDesiredWindowWidth(width);
        m_internalConfig.setDesiredWindowHeight(height);

        return StatusOK;
    }

    status_t DisplayConfigImpl::getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const
    {
        x = m_internalConfig.getWindowPositionX();
        y = m_internalConfig.getWindowPositionY();
        width = m_internalConfig.getDesiredWindowWidth();
        height = m_internalConfig.getDesiredWindowHeight();
        return StatusOK;
    }

    status_t DisplayConfigImpl::setFullscreen(bool fullscreen)
    {
        m_internalConfig.setFullscreenState(fullscreen);
        return StatusOK;
    }

    bool DisplayConfigImpl::isFullscreen() const
    {
        return m_internalConfig.getFullscreenState();
    }

    status_t DisplayConfigImpl::setBorderless(bool borderless)
    {
        m_internalConfig.setBorderlessState(borderless);
        return StatusOK;
    }

    const ramses_internal::DisplayConfig& DisplayConfigImpl::getInternalDisplayConfig() const
    {
        return m_internalConfig;
    }

    status_t DisplayConfigImpl::setMultiSampling(uint32_t numSamples)
    {
        if (numSamples != 1u &&
            numSamples != 2u &&
            numSamples != 4u)
        {
            return addErrorEntry("DisplayConfig::setMultiSampling failed - currently the only valid sample count is 1, 2 or 4!");
        }

        if (numSamples > 1u)
        {
            m_internalConfig.setAntialiasingMethod(ramses_internal::EAntiAliasingMethod_MultiSampling);
        }
        else
        {
            m_internalConfig.setAntialiasingMethod(ramses_internal::EAntiAliasingMethod_PlainFramebuffer);
        }
        m_internalConfig.setAntialiasingSampleCount(numSamples);

        return StatusOK;
    }

    status_t DisplayConfigImpl::getMultiSamplingSamples(uint32_t& numSamples) const
    {
        const uint32_t sampleCount = m_internalConfig.getAntialiasingSampleCount();
        numSamples = sampleCount;

        return StatusOK;
    }

    status_t DisplayConfigImpl::enableWarpingPostEffect()
    {
        m_internalConfig.setWarpingEnabled(true);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID)
    {
        m_internalConfig.setWaylandIviLayerID(ramses_internal::WaylandIviLayerId(waylandIviLayerID.getValue()));
        return StatusOK;
    }

    waylandIviLayerId_t DisplayConfigImpl::getWaylandIviLayerID() const
    {
        return waylandIviLayerId_t(m_internalConfig.getWaylandIviLayerID().getValue());
    }

    status_t DisplayConfigImpl::setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID)
    {
        m_internalConfig.setWaylandIviSurfaceID(ramses_internal::WaylandIviSurfaceId(waylandIviSurfaceID.getValue()));
        return StatusOK;
    }

    waylandIviSurfaceId_t DisplayConfigImpl::getWaylandIviSurfaceID() const
    {
        return waylandIviSurfaceId_t(m_internalConfig.getWaylandIviSurfaceID().getValue());
    }

    status_t DisplayConfigImpl::setIntegrityRGLDeviceUnit(uint32_t rglDeviceUnit)
    {
        m_internalConfig.setIntegrityRGLDeviceUnit(ramses_internal::IntegrityRGLDeviceUnit(rglDeviceUnit));
        return StatusOK;
    }

    uint32_t DisplayConfigImpl::getIntegrityRGLDeviceUnit() const
    {
        return m_internalConfig.getIntegrityRGLDeviceUnit().getValue();
    }

    void* DisplayConfigImpl::getAndroidNativeWindow() const
    {
        return m_internalConfig.getAndroidNativeWindow().getValue();
    }

    status_t DisplayConfigImpl::setAndroidNativeWindow(void * nativeWindowPtr)
    {
        m_internalConfig.setAndroidNativeWindow(ramses_internal::AndroidNativeWindowPtr(nativeWindowPtr));
        return StatusOK;
    }

    status_t DisplayConfigImpl::setWindowIviVisible(bool visible)
    {
        m_internalConfig.setStartVisibleIvi(visible);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setResizable(bool resizable)
    {
        m_internalConfig.setResizable(resizable);
        return StatusOK;
    }

    status_t DisplayConfigImpl::keepEffectsUploaded(bool enable)
    {
        m_internalConfig.setKeepEffectsUploaded(enable);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setGPUMemoryCacheSize(uint64_t size)
    {
        m_internalConfig.setGPUMemoryCacheSize(size);
        return StatusOK;
    }

    status_t DisplayConfigImpl::setClearColor(float red, float green, float blue, float alpha)
    {
        m_internalConfig.setClearColor(ramses_internal::Vector4(red, green, blue, alpha));
        return StatusOK;
    }

    status_t DisplayConfigImpl::setWindowsWindowHandle(void* hwnd)
    {
        m_internalConfig.setWindowsWindowHandle(ramses_internal::WindowsWindowHandle(hwnd));
        return StatusOK;
    }

    void* DisplayConfigImpl::getWindowsWindowHandle() const
    {
        return m_internalConfig.getWindowsWindowHandle().getValue();
    }

    status_t DisplayConfigImpl::setWaylandDisplay(const char* waylandDisplay)
    {
        m_internalConfig.setWaylandDisplay(waylandDisplay);
        return StatusOK;
    }

    const char* DisplayConfigImpl::getWaylandDisplay() const
    {
        return m_internalConfig.getWaylandDisplay().c_str();
    }
}
