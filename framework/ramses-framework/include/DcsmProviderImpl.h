//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMPROVIDERIMPL_H
#define RAMSES_DCSMPROVIDERIMPL_H

#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/EDcsmOfferingMode.h"

#include "Components/IDcsmComponent.h"
#include "Components/IDcsmProviderEventHandler.h"

#include "StatusObjectImpl.h"

#include <unordered_map>

namespace ramses
{
    class DcsmClientImpl;
    class IDcsmProviderEventHandler;
    class DcsmMetadataCreator;
    class CategoryInfoUpdate;

    class DcsmProviderImpl : public ramses_internal::IDcsmProviderEventHandler, public StatusObjectImpl
    {
    public:
        explicit DcsmProviderImpl(ramses_internal::IDcsmComponent& dcsm);
        ~DcsmProviderImpl() override;

        status_t offerContent(ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode);
        status_t offerContentWithMetadata(ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode, const DcsmMetadataCreator& metadata);
        status_t offerContent(ContentID contentID, Category category, waylandIviSurfaceId_t surfaceId, EDcsmOfferingMode mode);
        status_t offerContentWithMetadata(ContentID contentID, Category category, waylandIviSurfaceId_t surfaceId, EDcsmOfferingMode mode, const DcsmMetadataCreator& metadata);
        status_t requestStopOfferContent(ContentID contentID);

        status_t updateContentMetadata(ContentID contentID, const DcsmMetadataCreator& metadata);

        status_t markContentReady(ContentID contentID);

        status_t enableFocusRequest(ContentID contentID, int32_t focusRequest);
        status_t disableFocusRequest(ContentID contentID, int32_t focusRequest);
        status_t requestContentFocus(ContentID contentID);

        status_t dispatchEvents(ramses::IDcsmProviderEventHandler& handler);
        status_t dispatchEvents(ramses::IDcsmProviderEventHandlerExtended& handler);

        virtual void contentSizeChange(ContentID, const CategoryInfoUpdate&, AnimationInformation) override;
        virtual void contentStateChange(ContentID, ramses_internal::EDcsmState, const CategoryInfoUpdate&, AnimationInformation) override;
        virtual void contentStatus(ContentID, std::unique_ptr<DcsmStatusMessageImpl>&&) override;

    private:
        struct DcsmProviderMapContent
        {
            Category                                    category;
            ramses_internal::ETechnicalContentType      type;
            ramses_internal::TechnicalContentDescriptor contentDescriptor;
            ramses_internal::EDcsmState                 status = ramses_internal::EDcsmState::Offered;
            bool                                        ready = false;
            bool                                        contentRequested = false;
        };

        status_t commonOfferContent(const char* callerMethod, ContentID contentID, Category category, ramses_internal::ETechnicalContentType contentType,
            ramses_internal::TechnicalContentDescriptor contentDescriptor, EDcsmOfferingMode mode);
        status_t commonOfferContentWithMetadata(ContentID contentID, Category category, ramses_internal::ETechnicalContentType contentType,
            ramses_internal::TechnicalContentDescriptor contentDescriptor, EDcsmOfferingMode mode, const DcsmMetadataCreator& metadata);

        ramses_internal::IDcsmComponent& m_dcsm;
        ramses::IDcsmProviderEventHandler* m_handler = nullptr;
        ramses::IDcsmProviderEventHandlerExtended* m_msgHandler = nullptr;

        std::unordered_map<ramses::ContentID, DcsmProviderMapContent> m_contents;
    };
}

#endif
