//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEUPLOADER_H
#define RAMSES_RESOURCEUPLOADER_H

#include "RendererLib/IResourceUploader.h"
#include "Components/ManagedResource.h"

namespace ramses_internal
{
    class IBinaryShaderCache;
    class TextureResource;
    class EffectResource;
    class IDevice;
    struct ResourceDescriptor;

    class ResourceUploader : public IResourceUploader
    {
    public:
        explicit ResourceUploader(bool asyncEffectUploadEnabled, IBinaryShaderCache* binaryShaderCache = nullptr);

        virtual absl::optional<DeviceResourceHandle> uploadResource(IRenderBackend& renderBackend, const ResourceDescriptor& rd, UInt32& outVRAMSize) override;
        virtual void                 unloadResource(IRenderBackend& renderBackend, EResourceType type, ResourceContentHash hash, DeviceResourceHandle handle) override;
        void                         storeShaderInBinaryShaderCache(IRenderBackend& renderBackend, DeviceResourceHandle deviceHandle, const ResourceContentHash& hash, SceneId sceneid) override;

    private:
        DeviceResourceHandle uploadTexture(IDevice& device, const TextureResource& texture, UInt32& vramSize);
        DeviceResourceHandle queryBinaryShaderCache(IRenderBackend& renderBackend, const EffectResource& effect, ResourceContentHash hash);

        static UInt32 EstimateGPUAllocatedSizeOfTexture(const TextureResource& texture, UInt32 numMipLevelsToAllocate);

        const bool m_asyncEffectUploadEnabled;
        IBinaryShaderCache* const m_binaryShaderCache;
    };
}

#endif
