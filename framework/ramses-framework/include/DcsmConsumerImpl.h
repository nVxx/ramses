//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONSUMERIMPL_H
#define RAMSES_DCSMCONSUMERIMPL_H

#include "IDcsmConsumerImpl.h"
#include "StatusObjectImpl.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "Components/DcsmComponent.h"

namespace ramses
{
    class RamsesFrameworkImpl;
    class CategoryInfoUpdate;

    class DcsmConsumerImpl final : public IDcsmConsumerImpl, public StatusObjectImpl
    {
    public:
        explicit DcsmConsumerImpl(RamsesFrameworkImpl& framework);
        virtual ~DcsmConsumerImpl() override;

        virtual status_t dispatchEvents(IDcsmConsumerEventHandler& handler) override;
        virtual status_t assignContentToConsumer(ContentID contentID, const CategoryInfoUpdate& size) override;
        virtual status_t contentSizeChange(ContentID contentID, const CategoryInfoUpdate& size, AnimationInformation animationInformation) override;
        virtual status_t contentStateChange(ContentID contentID, EDcsmState state, AnimationInformation animationInformation) override;
        virtual status_t acceptStopOffer(ContentID contentID, AnimationInformation animationInformation) override;
        virtual status_t sendContentStatus(ContentID contentID, DcsmStatusMessage const& message) override;

    private:
        ramses_internal::DcsmComponent& m_component;
    };
}

#endif
