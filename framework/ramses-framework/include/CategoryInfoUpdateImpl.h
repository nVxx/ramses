//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CATEGORYINFOUPDATEIMPL_H
#define RAMSES_CATEGORYINFOUPDATEIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "StatusObjectImpl.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "Components/CategoryInfo.h"

namespace ramses
{
    class CategoryInfoUpdateImpl : public StatusObjectImpl
    {
    public:
        CategoryInfoUpdateImpl();
        ~CategoryInfoUpdateImpl() = default;

        const ramses_internal::CategoryInfo& getCategoryInfo() const;
        void setCategoryInfo(const ramses_internal::CategoryInfo& other);

        bool hasCategoryRectUpdate() const;
        Rect getCategoryRect() const;
        status_t setCategoryRect(Rect rect);

        bool hasRenderSizeUpdate() const;
        SizeInfo getRenderSize() const;
        status_t setRenderSize(SizeInfo sizeInfo);

        bool hasSafeRectUpdate() const;
        Rect getSafeRect() const;
        status_t setSafeRect(Rect rect);

        bool hasActiveLayoutUpdate() const;
        CategoryInfoUpdate::Layout getActiveLayout() const;
        status_t setActiveLayout(CategoryInfoUpdate::Layout layout);
    private:
        ramses_internal::CategoryInfo m_categoryInfo;
    };
}

#endif
