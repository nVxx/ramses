//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FLUSHINFORMATION_H
#define RAMSES_FLUSHINFORMATION_H

#include "SceneAPI/SceneSizeInformation.h"
#include "Scene/ResourceChanges.h"
#include "SceneReferencing/SceneReferenceAction.h"
#include "FlushTimeInformation.h"
#include "SceneAPI/SceneVersionTag.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    struct FlushInformation
    {
        FlushInformation() = default;
        FlushInformation(FlushInformation&&) = default;
        FlushInformation& operator=(FlushInformation&&) = default;

        FlushInformation(FlushInformation const&) = delete;
        FlushInformation& operator=(FlushInformation const&) = delete;

        FlushInformation copy() const
        {
            FlushInformation ret;
            ret.flushCounter = flushCounter;
            ret.versionTag = versionTag;
            ret.sizeInfo = sizeInfo;
            ret.resourceChanges = resourceChanges;
            ret.sceneReferences = sceneReferences;
            ret.flushTimeInfo = flushTimeInfo;
            ret.hasSizeInfo = hasSizeInfo;
            ret.containsValidInformation = containsValidInformation;

            return ret;
        }

        UInt64 flushCounter = 0;
        SceneVersionTag versionTag;
        SceneSizeInformation sizeInfo;
        ResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferences;
        FlushTimeInformation flushTimeInfo;
        bool hasSizeInfo = false;
        bool containsValidInformation = false;

        static constexpr size_t getMinimumSize() {
            return sizeof(containsValidInformation) +
                + sizeof(flushCounter)
                + sizeof(uint8_t) // flush flags
                + 3 * sizeof(uint32_t) // number of entries of three resource vectors
                + sizeof(uint32_t) // number of entries of ref action vector
                + 2 * sizeof(uint64_t) // timestamps each serialized as uint64_t milliseconds
                + sizeof(uint32_t) // clock type serialized as uint32_t
                + sizeof(SceneVersionTag::BaseType);
        }
    };

    inline bool operator==(const FlushInformation& a, const FlushInformation& b)
    {
    return a.containsValidInformation == b.containsValidInformation
            && a.flushCounter == b.flushCounter
            && a.versionTag == b.versionTag
            && a.sizeInfo == b.sizeInfo
            && a.resourceChanges == b.resourceChanges
            && a.sceneReferences == b.sceneReferences
            && a.flushTimeInfo == b.flushTimeInfo
            && a.hasSizeInfo == b.hasSizeInfo;
    }

    inline bool operator!=(const FlushInformation& a, const FlushInformation& b)
    {
        return !(a == b);
    }
}

template <>
struct fmt::formatter<ramses_internal::FlushInformation> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::FlushInformation& fi, FormatContext& ctx)
    {
        fmt::format_to(ctx.out(),
            "FlushInformation:[valid:{};flushcounter:{};version:{};resChanges[+:{};-:{};resActions:{}];refActions:{};time[{};sync:{};exp:{};int:{}];sizeInfo:",
            fi.containsValidInformation,
            fi.flushCounter,
            fi.versionTag,
            fi.resourceChanges.m_resourcesAdded.size(), fi.resourceChanges.m_resourcesRemoved.size(), fi.resourceChanges.m_sceneResourceActions.size(),
            fi.sceneReferences.size(),
            fi.flushTimeInfo.clock_type,
            (fi.flushTimeInfo.isEffectTimeSync ? 1 : 0),
            static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(fi.flushTimeInfo.expirationTimestamp).time_since_epoch().count()),
            static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(fi.flushTimeInfo.internalTimestamp).time_since_epoch().count()));
        if (fi.hasSizeInfo)
            fmt::format_to(ctx.out(), "{}", fi.sizeInfo);
        else
            fmt::format_to(ctx.out(), "none");
        return fmt::format_to(ctx.out(), "]");
    }
};
#endif
