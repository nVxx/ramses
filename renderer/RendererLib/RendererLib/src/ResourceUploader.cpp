//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/ResourceUploader.h"
#include "Resource/IResource.h"
#include "Resource/ArrayResource.h"
#include "Resource/TextureResource.h"
#include "Resource/EffectResource.h"
#include "RendererAPI/IBinaryShaderCache.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/IRenderBackend.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Utils/TextureMathUtils.h"
#include "Components/ManagedResource.h"
#include "RendererLib/ResourceDescriptor.h"

namespace ramses_internal
{
    ResourceUploader::ResourceUploader(bool asyncEffectUploadEnabled, IBinaryShaderCache* binaryShaderCache)
        : m_asyncEffectUploadEnabled(asyncEffectUploadEnabled)
        , m_binaryShaderCache(binaryShaderCache)
    {
    }

    absl::optional<DeviceResourceHandle> ResourceUploader::uploadResource(IRenderBackend& renderBackend, const ResourceDescriptor& rd, UInt32& outVRAMSize)
    {
        ManagedResource res = rd.resource;
        const IResource& resourceObject = *res.get();
        IDevice& device = renderBackend.getDevice();
        outVRAMSize = resourceObject.getDecompressedDataSize();

        switch (resourceObject.getTypeID())
        {
        case EResourceType_VertexArray:
        {
            const ArrayResource* vertArray = resourceObject.convertTo<ArrayResource>();
            const DeviceResourceHandle deviceHandle = device.allocateVertexBuffer(vertArray->getDecompressedDataSize());
            device.uploadVertexBufferData(deviceHandle, vertArray->getResourceData().data(), vertArray->getDecompressedDataSize());
            return deviceHandle;
        }
        case EResourceType_IndexArray:
        {
            const ArrayResource* indexArray = resourceObject.convertTo<ArrayResource>();
            const DeviceResourceHandle deviceHandle = device.allocateIndexBuffer(indexArray->getElementType(), indexArray->getDecompressedDataSize());
            device.uploadIndexBufferData(deviceHandle, indexArray->getResourceData().data(), indexArray->getDecompressedDataSize());
            return deviceHandle;
        }
        case EResourceType_Texture2D:
        case EResourceType_Texture3D:
        case EResourceType_TextureCube:
            return uploadTexture(device, *resourceObject.convertTo<TextureResource>(), outVRAMSize);
        case EResourceType_Effect:
        {
            const EffectResource* effectRes = resourceObject.convertTo<EffectResource>();
            const ResourceContentHash hash = effectRes->getHash();
            const auto binaryShaderDeviceHandle = queryBinaryShaderCache(renderBackend, *effectRes, hash);
            if(binaryShaderDeviceHandle.isValid())
                return binaryShaderDeviceHandle;

            if(m_asyncEffectUploadEnabled)
                return {};

            auto effectGpuRes = renderBackend.getDevice().uploadShader(*effectRes);
            return renderBackend.getDevice().registerShader(std::move(effectGpuRes));
        }
        default:
            assert(false && "Unexpected resource type");
            return DeviceResourceHandle::Invalid();
        }
    }

    void ResourceUploader::unloadResource(IRenderBackend& renderBackend, EResourceType type, ResourceContentHash /*hash*/, const DeviceResourceHandle handle)
    {
        switch (type)
        {
        case EResourceType_VertexArray:
            renderBackend.getDevice().deleteVertexBuffer(handle);
            break;
        case EResourceType_IndexArray:
            renderBackend.getDevice().deleteIndexBuffer(handle);
            break;
        case EResourceType_Texture2D:
        case EResourceType_Texture3D:
        case EResourceType_TextureCube:
            renderBackend.getDevice().deleteTexture(handle);
            break;
        case EResourceType_Effect:
            renderBackend.getDevice().deleteShader(handle);
            break;
        default:
            assert(false && "Unexpected resource type");
        }
    }

    DeviceResourceHandle ResourceUploader::uploadTexture(IDevice& device, const TextureResource& texture, UInt32& vramSize)
    {
        const Bool generateMipsFlag = texture.getGenerateMipChainFlag();
        const auto& mipDataSizes = texture.getMipDataSizes();
        const UInt32 numProvidedMipLevels = static_cast<UInt32>(mipDataSizes.size());
        assert(numProvidedMipLevels == 1u || !generateMipsFlag);
        const UInt32 numMipLevelsToAllocate = generateMipsFlag ? TextureMathUtils::GetMipLevelCount(texture.getWidth(), texture.getHeight(), texture.getDepth()) : numProvidedMipLevels;
        vramSize = EstimateGPUAllocatedSizeOfTexture(texture, numMipLevelsToAllocate);

        // allocate texture
        DeviceResourceHandle textureDeviceHandle;
        switch (texture.getTypeID())
        {
        case EResourceType_Texture2D:
            textureDeviceHandle = device.allocateTexture2D(texture.getWidth(), texture.getHeight(), texture.getTextureFormat(), texture.getTextureSwizzle(), numMipLevelsToAllocate, vramSize);
            break;
        case EResourceType_Texture3D:
            textureDeviceHandle = device.allocateTexture3D(texture.getWidth(), texture.getHeight(), texture.getDepth(), texture.getTextureFormat(), numMipLevelsToAllocate, vramSize);
            break;
        case EResourceType_TextureCube:
            textureDeviceHandle = device.allocateTextureCube(texture.getWidth(), texture.getTextureFormat(), texture.getTextureSwizzle(), numMipLevelsToAllocate, vramSize);
            break;
        default:
            assert(false);
        }
        assert(textureDeviceHandle.isValid());

        // upload texture data
        const Byte* pData = texture.getResourceData().data();
        switch (texture.getTypeID())
        {
        case EResourceType_Texture2D:
        case EResourceType_Texture3D:
            for (UInt32 mipLevel = 0u; mipLevel < static_cast<uint32_t>(mipDataSizes.size()); ++mipLevel)
            {
                const UInt32 width = TextureMathUtils::GetMipSize(mipLevel, texture.getWidth());
                const UInt32 height = TextureMathUtils::GetMipSize(mipLevel, texture.getHeight());
                const UInt32 depth = TextureMathUtils::GetMipSize(mipLevel, texture.getDepth());
                device.uploadTextureData(textureDeviceHandle, mipLevel, 0u, 0u, 0u, width, height, depth, pData, mipDataSizes[mipLevel]);
                pData += mipDataSizes[mipLevel];
            }
            break;
        case EResourceType_TextureCube:
            for (UInt32 i = 0; i < 6u; ++i)
            {
                const ETextureCubeFace faceId = static_cast<ETextureCubeFace>(i);
                for (UInt32 mipLevel = 0u; mipLevel < static_cast<uint32_t>(mipDataSizes.size()); ++mipLevel)
                {
                    const UInt32 faceSize = TextureMathUtils::GetMipSize(mipLevel, texture.getWidth());
                    // texture faceID is encoded in Z offset
                    device.uploadTextureData(textureDeviceHandle, mipLevel, 0u, 0u, faceId, faceSize, faceSize, 1u, pData, mipDataSizes[mipLevel]);
                    pData += mipDataSizes[mipLevel];
                }
            }
            break;
        default:
            assert(false);
        }

        if (generateMipsFlag)
        {
            device.generateMipmaps(textureDeviceHandle);
        }

        return textureDeviceHandle;
    }

    DeviceResourceHandle ResourceUploader::queryBinaryShaderCache(IRenderBackend& renderBackend, const EffectResource& effect, ResourceContentHash hash)
    {
        LOG_TRACE(CONTEXT_RENDERER, "ResourceUploader::queryBinaryShaderCacheAndUploadEffect: effectid:" << effect.getHash());
        IDevice& device = renderBackend.getDevice();

        if (!m_binaryShaderCache)
            return {};

        std::call_once(m_binaryShaderCache->binaryShaderFormatsReported(), [this, &device]() {
                std::vector<BinaryShaderFormatID> supportedFormats;
                device.getSupportedBinaryProgramFormats(supportedFormats);
                m_binaryShaderCache->deviceSupportsBinaryShaderFormats(supportedFormats);
            });

        if (m_binaryShaderCache->hasBinaryShader(hash))
        {
            LOG_TRACE(CONTEXT_RENDERER, "ResourceUploader::queryBinaryShaderCacheAndUploadEffect: Cache has binary shader");
            const UInt32 binaryShaderSize = m_binaryShaderCache->getBinaryShaderSize(hash);
            const BinaryShaderFormatID binaryShaderFormat = m_binaryShaderCache->getBinaryShaderFormat(hash);

            UInt8Vector buffer(binaryShaderSize);
            m_binaryShaderCache->getBinaryShaderData(hash, &buffer.front(), binaryShaderSize);

            const DeviceResourceHandle binaryShaderHandle = device.uploadBinaryShader(effect, &buffer.front(), binaryShaderSize, binaryShaderFormat);

            // Always tell if the upload succeeded or not. This allows the user to know that the cache was broken (for whatever reason)
            m_binaryShaderCache->binaryShaderUploaded(hash, binaryShaderHandle.isValid());

            if (binaryShaderHandle.isValid())
            {
                return binaryShaderHandle;
            }
        }

        LOG_TRACE(CONTEXT_RENDERER, "ResourceUploader::queryBinaryShaderCacheAndUploadEffect: Cache does not have binary shader");
        // If this point is reached, we either have no cache or the cache was broken.
        return {};
    }

    void ResourceUploader::storeShaderInBinaryShaderCache(IRenderBackend& renderBackend, DeviceResourceHandle deviceHandle, const ResourceContentHash& hash, SceneId sceneid)
    {
        assert(deviceHandle.isValid());
        if (m_binaryShaderCache && m_binaryShaderCache->shouldBinaryShaderBeCached(hash, sceneid))
        {
            IDevice& device = renderBackend.getDevice();

            UInt8Vector binaryShader;
            BinaryShaderFormatID format;
            if (device.getBinaryShader(deviceHandle, binaryShader, format))
            {
                assert(binaryShader.size() != 0u);
                m_binaryShaderCache->storeBinaryShader(hash, sceneid, &binaryShader.front(), static_cast<UInt32>(binaryShader.size()), format);
            }
            else
                LOG_WARN_P(CONTEXT_RENDERER, "ResourceUploader::storeShaderInBinaryShaderCache: failed to retrieve binary shader from device, shader cannot be stored in cache (deviceHandle={} hash={} sceneId={})", deviceHandle, hash, sceneid);
        }
    }

    UInt32 ResourceUploader::EstimateGPUAllocatedSizeOfTexture(const TextureResource& texture, UInt32 numMipLevelsToAllocate)
    {
        if (IsFormatCompressed(texture.getTextureFormat()))
        {
            return texture.getDecompressedDataSize();
        }
        else
        {
            if (texture.getTypeID() == EResourceType_TextureCube)
                return 6u * TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(texture.getTextureFormat()), texture.getWidth(), texture.getWidth(), 1u, numMipLevelsToAllocate);
            else
                return TextureMathUtils::GetTotalMemoryUsedByMipmappedTexture(GetTexelSizeFromFormat(texture.getTextureFormat()), texture.getWidth(), texture.getHeight(), texture.getDepth(), numMipLevelsToAllocate);
        }
    }
}
