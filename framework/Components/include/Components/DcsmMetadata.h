//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENTS_DCSMMETADATA_H
#define RAMSES_COMPONENTS_DCSMMETADATA_H

#include "ramses-framework-api/CarModelViewMetadata.h"
#include "Collections/StringOutputStream.h"
#include "Components/DcsmTypes.h"
#include "absl/types/span.h"
#include "Utils/AssertMovable.h"
#include <vector>
#include <string>
#include <cstdint>

namespace ramses_internal
{
    class DcsmMetadata
    {
    public:
        explicit DcsmMetadata(absl::Span<const Byte> data);
        DcsmMetadata() = default;

        DcsmMetadata(DcsmMetadata&&) noexcept = default;
        DcsmMetadata& operator=(DcsmMetadata&&) noexcept = default;

        // TODO(tobias): delete + explicit copy?
        DcsmMetadata(const DcsmMetadata&) = default;
        DcsmMetadata& operator=(const DcsmMetadata&) = default;

        bool empty() const;
        std::vector<Byte> toBinary() const;
        void updateFromOther(const DcsmMetadata& other);

        bool setPreviewImagePng(const void* data, size_t dataLength);
        bool setPreviewDescription(std::u32string previewDescription);
        bool setWidgetOrder(int32_t widgetOrder);
        bool setWidgetBackgroundID(int32_t widgetBackgroundID);
        bool setWidgetHUDLineID(int32_t widgetHUDlineID);
        bool setCarModel(int32_t carModel);
        bool setCarModelView(const ramses::CarModelViewMetadata& values, const AnimationInformation& timingInfo);
        bool setCarModelViewExtended(const ramses::CarModelViewMetadataExtended& values);
        bool setCarModelVisibility(bool visibility);
        bool setExclusiveBackground(bool state);
        bool setStreamID(int32_t streamID);
        bool setContentFlippedVertically(bool state);
        bool setDisplayedDataFlags(uint32_t flags);
        bool setLayoutAvailability(uint8_t flags);
        bool setConfiguratorPriority(uint8_t priority);

        bool hasPreviewImagePng() const;
        bool hasPreviewDescription() const;
        bool hasWidgetOrder() const;
        bool hasWidgetBackgroundID() const;
        bool hasWidgetHUDLineID() const;
        bool hasCarModel() const;
        bool hasCarModelView() const;
        bool hasCarModelViewExtended() const;
        bool hasCarModelVisibility() const;
        bool hasExclusiveBackground() const;
        bool hasStreamID() const;
        bool hasContentFlippedVertically() const;
        bool hasDisplayedDataFlags() const;
        bool hasLayoutAvailability() const;
        bool hasConfiguratorPriority() const;

        std::vector<unsigned char> getPreviewImagePng() const;
        std::u32string getPreviewDescription() const;
        int32_t getWidgetOrder() const;
        int32_t getWidgetBackgroundID() const;
        int32_t getWidgetHUDLineID() const;
        int32_t getCarModel() const;
        ramses::CarModelViewMetadata getCarModelView() const;
        ramses::CarModelViewMetadataExtended getCarModelViewExtended() const;
        AnimationInformation getCarModelViewAnimationInfo() const;
        bool getCarModelVisibility() const;
        bool getExclusiveBackground() const;
        int32_t getStreamID() const;
        bool getContentFlippedVertically() const;
        uint32_t getDisplayedDataFlags() const;
        uint8_t getLayoutAvailability() const;
        uint8_t getConfiguratorPriority() const;

        bool operator==(const DcsmMetadata& other) const;
        bool operator!=(const DcsmMetadata& other) const;

        static constexpr const size_t MaxPreviewImageSize = 500000;
        static constexpr const size_t MaxPreviewImageWidth = 1000;
        static constexpr const size_t MaxPreviewImageHeight = 1000;

    private:
        friend struct fmt::formatter<ramses_internal::DcsmMetadata>;

        void fromBinary(absl::Span<const Byte> data);

        std::vector<unsigned char> m_previewImagePng;
        std::u32string m_previewDescription;
        int32_t m_widgetOrder = 0;
        int32_t m_widgetBackgroundID = 0;
        int32_t m_widgetHUDLineID = 0;
        int32_t m_carModel = 0;
        int32_t m_streamID = 0;
        ramses::CarModelViewMetadata m_carModelView = {};
        ramses::CarModelViewMetadataExtended m_carModelViewExtended = {};
        AnimationInformation m_carModelViewTiming = {};
        bool m_exclusiveBackground = false;
        bool m_carModelVisibility = false;
        bool m_contentFlippedVertically = false;
        uint32_t m_displayedDataFlags = 0;
        uint8_t m_layoutAvailability = 0;
        uint8_t m_configuratorPriority = 0;

        bool m_hasCarModelView = false;
        bool m_hasCarModelViewExtended = false;
        bool m_hasPreviewImagePng = false;
        bool m_hasPreviewDescription = false;
        bool m_hasWidgetOrder = false;
        bool m_hasWidgetBackgroundID = false;
        bool m_hasWidgetHUDLineID = false;
        bool m_hasCarModel = false;
        bool m_hasCarModelVisibility = false;
        bool m_hasExclusiveBackground = false;
        bool m_hasStreamID = false;
        bool m_hasContentFlippedVertically = false;
        bool m_hasDisplayedDataFlags = false;
        bool m_hasLayoutAvailability = false;
        bool m_hasConfiguratorPriority = false;
    };

    ASSERT_MOVABLE(DcsmMetadata)
}

template<>
struct fmt::formatter<ramses::CarModelViewMetadataExtended> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext>
    constexpr auto format(const ramses::CarModelViewMetadataExtended& cm, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(),
                       "r:{},t({},{},{})",
                       cm.roll,
                       cm.cameraLocalTranslation_x,
                       cm.cameraLocalTranslation_y,
                       cm.cameraLocalTranslation_z);
    }
};

template <>
struct fmt::formatter<ramses_internal::DcsmMetadata> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext>
    constexpr auto format(const ramses_internal::DcsmMetadata& dm, FormatContext& ctx)
    {
        fmt::format_to(ctx.out(), "[");
        if (dm.hasPreviewImagePng())
            fmt::format_to(ctx.out(), "png:{}; ",  dm.m_previewImagePng.size());
        if (dm.hasPreviewDescription())
        {
            fmt::format_to(ctx.out(), "desc:{}[", dm.m_previewDescription.size());
            for (const auto& e : dm.m_previewDescription)
                fmt::format_to(ctx.out(), "{:X};", static_cast<uint32_t>(e));
            fmt::format_to(ctx.out(), "]");
        }
        if (dm.hasWidgetOrder())
            fmt::format_to(ctx.out(), "order:{}; ", dm.m_widgetOrder);
        if (dm.hasWidgetBackgroundID())
            fmt::format_to(ctx.out(), "bkgr:{}; ", dm.m_widgetBackgroundID);
        if (dm.hasWidgetHUDLineID())
            fmt::format_to(ctx.out(), "hudline:{}; ", dm.m_widgetHUDLineID);
        if (dm.hasCarModel())
            fmt::format_to(ctx.out(), "car:{}; ", dm.m_carModel);
        if (dm.hasCarModelView())
            fmt::format_to(ctx.out(), "carView:{},{},{},{},{},{},{},{},{},{},{}; ",
                           dm.m_carModelView.pitch,
                           dm.m_carModelView.yaw,
                           dm.m_carModelView.distance,
                           dm.m_carModelView.origin_x,
                           dm.m_carModelView.origin_y,
                           dm.m_carModelView.origin_z,
                           dm.m_carModelView.cameraFOV,
                           dm.m_carModelView.nearPlane,
                           dm.m_carModelView.farPlane,
                           dm.m_carModelViewTiming.startTimeStamp,
                           dm.m_carModelViewTiming.finishedTimeStamp);
        if (dm.hasCarModelViewExtended())
            fmt::format_to(ctx.out(), "carViewExt:{}; ", dm.m_carModelViewExtended);
        if (dm.hasCarModelVisibility())
            fmt::format_to(ctx.out(), "carVis:{}; ", dm.m_carModelVisibility);
        if (dm.hasExclusiveBackground())
            fmt::format_to(ctx.out(), "exclBG:{}; ", dm.m_exclusiveBackground);
        if (dm.hasStreamID())
            fmt::format_to(ctx.out(), "streamID:{}; ", dm.m_streamID);
        if (dm.hasContentFlippedVertically())
            fmt::format_to(ctx.out(), "contentFlippedVertically:{}; ", dm.m_contentFlippedVertically);
        if (dm.hasDisplayedDataFlags())
            fmt::format_to(ctx.out(), "displayedDataFlags:{}; ", dm.m_displayedDataFlags);
        if (dm.hasLayoutAvailability())
            fmt::format_to(ctx.out(), "layoutAvailability:{}; ", dm.m_layoutAvailability);
        if (dm.hasConfiguratorPriority())
            fmt::format_to(ctx.out(), "configuratorPriority:{}; ", dm.m_configuratorPriority);
        return fmt::format_to(ctx.out(), "]");
    }
};

#endif
