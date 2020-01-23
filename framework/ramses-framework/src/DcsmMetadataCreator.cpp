//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmMetadataCreator.h"
#include "DcsmMetadataCreatorImpl.h"
#include "APILoggingMacros.h"

namespace ramses
{
    DcsmMetadataCreator::DcsmMetadataCreator()
        : StatusObject(*new DcsmMetadataCreatorImpl)
        , impl(static_cast<DcsmMetadataCreatorImpl&>(StatusObject::impl))
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    DcsmMetadataCreator::~DcsmMetadataCreator()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    status_t DcsmMetadataCreator::setPreviewImagePng(const void* data, size_t dataLength)
    {
        const auto status = impl.setPreviewImagePng(data, dataLength);
        LOG_HL_CLIENT_API2(status, LOG_API_GENERIC_PTR_STRING(data), dataLength);
        return status;
    }

    status_t DcsmMetadataCreator::setPreviewDescription(std::u32string previewDescription)
    {
        const auto status = impl.setPreviewDescription(std::move(previewDescription));
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t DcsmMetadataCreator::setWidgetOrder(int32_t widgetOrder)
    {
        const auto status = impl.setWidgetOrder(widgetOrder);
        LOG_HL_CLIENT_API1(status, widgetOrder);
        return status;
    }

    status_t DcsmMetadataCreator::setWidgetBackgroundID(int32_t backgroundID)
    {
        const auto status = impl.setWidgetBackgroundID(backgroundID);
        LOG_HL_CLIENT_API1(status, backgroundID);
        return status;
    }

    status_t DcsmMetadataCreator::setWidgetHUDLineID(int32_t hudLineID)
    {
        const auto status = impl.setWidgetHUDLineID(hudLineID);
        LOG_HL_CLIENT_API1(status, hudLineID);
        return status;
    }

    status_t DcsmMetadataCreator::setCarModel(int32_t carModel)
    {
        const auto status = impl.setCarModel(carModel);
        LOG_HL_CLIENT_API1(status, carModel);
        return status;
    }

    status_t DcsmMetadataCreator::setCarModelView(const CarModelViewMetadata& data)
    {
        const auto status = impl.setCarModelView(data);
        LOG_HL_CLIENT_API6(status, data.pitch, data.yaw, data.distance, data.origin_x, data.origin_y, data.origin_z);
        return status;
    }

    status_t DcsmMetadataCreator::setCarModelVisibility(bool visibility)
    {
        const auto status = impl.setCarModelVisibility(visibility);
        LOG_HL_CLIENT_API1(status, visibility);
        return status;
    }

}
