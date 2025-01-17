//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCECONTENTHASH_H
#define RAMSES_RESOURCECONTENTHASH_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/Hash.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "PlatformAbstraction/FmtBase.h"
#include "Resource/ResourceTypes.h"
#include <cinttypes>
#include <functional>
#include <vector>

namespace ramses_internal
{
    struct ResourceContentHash
    {
        constexpr ResourceContentHash()
            : lowPart(0)
            , highPart(0)
        {
        }

        constexpr ResourceContentHash(UInt64 low, UInt64 high)
            : lowPart(low)
            , highPart(high)
        {
        }

        static constexpr ResourceContentHash Invalid()
        {
            return ResourceContentHash();
        }

        constexpr inline bool isValid() const
        {
            return *this != Invalid();
        }

        constexpr inline bool operator==(const ResourceContentHash& rhs) const
        {
            return (lowPart == rhs.lowPart && highPart == rhs.highPart);
        }

        constexpr inline bool operator!=(const ResourceContentHash& rhs) const
        {
            return !(*this == rhs);
        }

        UInt64 lowPart;
        UInt64 highPart;
    };

    constexpr inline bool operator<(ResourceContentHash const& lhs, ResourceContentHash const& rhs)
    {
        return lhs.highPart == rhs.highPart ? lhs.lowPart < rhs.lowPart : lhs.highPart < rhs.highPart;
    }

    inline IOutputStream& operator<<(IOutputStream& stream, const ResourceContentHash& value)
    {
        return stream << value.lowPart << value.highPart;
    }

    inline IInputStream& operator>>(IInputStream& stream, ResourceContentHash& value)
    {
        return stream >> value.lowPart >> value.highPart;
    }

    using ResourceContentHashVector = std::vector<ResourceContentHash>;
}

template <>
struct fmt::formatter<ramses_internal::ResourceContentHash> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::ResourceContentHash& res, FormatContext& ctx)
    {
        const char* typeShortString = nullptr;
        const uint32_t type = (res.highPart >> 60LU) & 0xF;
        switch (type)
        {
        case static_cast<uint32_t>(ramses_internal::EResourceType_VertexArray):
            typeShortString = "vtx";
            break;
        case static_cast<uint32_t>(ramses_internal::EResourceType_IndexArray):
            typeShortString = "idx";
            break;
        case static_cast<uint32_t>(ramses_internal::EResourceType_Texture2D):
            typeShortString = "tx2";
            break;
        case static_cast<uint32_t>(ramses_internal::EResourceType_Texture3D):
            typeShortString = "tx3";
            break;
        case static_cast<uint32_t>(ramses_internal::EResourceType_TextureCube):
            typeShortString = "txc";
            break;
        case static_cast<uint32_t>(ramses_internal::EResourceType_Effect):
            typeShortString = "eff";
            break;
        default:
            typeShortString = "inv";
        }
        return fmt::format_to(ctx.out(), "{}_{:016X}{:016X}", typeShortString, res.highPart, res.lowPart);
    }
};

template <>
struct fmt::formatter<ramses_internal::ResourceContentHashVector> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::ResourceContentHashVector& hashes, FormatContext& ctx)
    {
        fmt::format_to(ctx.out(), "[{} resources:", hashes.size());
        for (auto const& hash : hashes)
            fmt::format_to(ctx.out(), " {}", hash);

        return fmt::format_to(ctx.out(), "]");
    }
};

// make hashable
namespace std
{
    template <>
    struct hash<ramses_internal::ResourceContentHash>
    {
    public:
        size_t operator()(const ramses_internal::ResourceContentHash& v) const
        {
            static_assert(sizeof(ramses_internal::ResourceContentHash) == 2*sizeof(uint64_t), "make sure resourceontenthash is just 2 64 values");
            return ramses_internal::HashMemoryRange(&v, 2 * sizeof(uint64_t));
        }
    };
}


#endif
