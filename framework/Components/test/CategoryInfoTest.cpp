//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "Components/DcsmTypes.h"
#include "Components/CategoryInfo.h"
#include "Utils/BinaryOutputStream.h"

namespace ramses_internal
{
    class ACategoryInfo : public testing::Test
    {
    public:
        ACategoryInfo()
        {
            filled.setCategoryRect(12, 34, 56, 78);
            filled.setRenderSize(88,99);
            filled.setSafeRect(4,3,2,1);
            filled.setActiveLayout(CategoryInfo::Layout::Focus);
        }

        CategoryInfo serializeDeserialize(const CategoryInfo& ref)
        {
            const auto vec = ref.toBinary();
            EXPECT_TRUE(vec.size() > 0);
            return CategoryInfo(vec);
        }

        CategoryInfo filled;
        CategoryInfo empty;
    };

    TEST(CategoryInfo, hasComparison)
    {
        CategoryInfo defaultConstructed;
        CategoryInfo explicitZero(0u, 0u);
        CategoryInfo explicitValuesSet;
        explicitValuesSet.setCategoryRect(0, 0, 3, 4);
        CategoryInfo explicitValuesConstructor{ 3, 4 };

        EXPECT_FALSE(defaultConstructed == explicitZero); // zero 'has value'
        EXPECT_TRUE(defaultConstructed != explicitZero);
        EXPECT_TRUE(explicitValuesSet == explicitValuesConstructor);
        EXPECT_FALSE(explicitValuesSet != explicitValuesConstructor);

        explicitValuesSet.setCategoryRect(1, 2, 3, 4);
        EXPECT_FALSE(explicitValuesSet == explicitValuesConstructor);
    }

    TEST(CategoryInfo, defaultValues)
    {
        CategoryInfo value;
        EXPECT_EQ(0u, value.getCategoryX());
        EXPECT_EQ(0u, value.getCategoryY());
        EXPECT_EQ(0u, value.getCategoryWidth());
        EXPECT_EQ(0u, value.getCategoryHeight());
        EXPECT_FALSE(value.hasCategoryRectChange());
        EXPECT_EQ(0u, value.getSafeRectX());
        EXPECT_EQ(0u, value.getSafeRectY());
        EXPECT_EQ(0u, value.getSafeRectWidth());
        EXPECT_EQ(0u, value.getSafeRectHeight());
        EXPECT_FALSE(value.hasSafeRectChange());
        EXPECT_EQ(0u, value.getRenderSizeWidth());
        EXPECT_EQ(0u, value.getRenderSizeHeight());
        EXPECT_FALSE(value.hasRenderSizeChange());
        EXPECT_EQ(CategoryInfo::Layout::Drive, value.getActiveLayout());
        EXPECT_FALSE(value.hasActiveLayoutChange());
    }

    TEST(CategoryInfo, setCategoryRect)
    {
        CategoryInfo value;
        EXPECT_FALSE(value.hasCategoryRectChange());

        value.setCategoryRect(1, 2, 3, 4);
        EXPECT_TRUE(value.hasCategoryRectChange());
        EXPECT_EQ(1u, value.getCategoryX());
        EXPECT_EQ(2u, value.getCategoryY());
        EXPECT_EQ(3u, value.getCategoryWidth());
        EXPECT_EQ(4u, value.getCategoryHeight());
    }

    TEST(CategoryInfo, setSafeRect)
    {
        CategoryInfo value;
        EXPECT_FALSE(value.hasSafeRectChange());

        value.setSafeRect(1, 2, 3, 4);
        EXPECT_TRUE(value.hasSafeRectChange());
        EXPECT_EQ(1u, value.getSafeRectX());
        EXPECT_EQ(2u, value.getSafeRectY());
        EXPECT_EQ(3u, value.getSafeRectWidth());
        EXPECT_EQ(4u, value.getSafeRectHeight());
    }

    TEST(CategoryInfo, setRenderSize)
    {
        CategoryInfo value;
        EXPECT_FALSE(value.hasRenderSizeChange());

        value.setRenderSize(1, 2);
        EXPECT_TRUE(value.hasRenderSizeChange());
        EXPECT_EQ(1u, value.getRenderSizeWidth());
        EXPECT_EQ(2u, value.getRenderSizeHeight());
    }

    TEST(CategoryInfo, setActiveLayout)
    {
        CategoryInfo value;
        EXPECT_FALSE(value.hasActiveLayoutChange());

        value.setActiveLayout(CategoryInfo::Layout::Sport_Track);
        EXPECT_TRUE(value.hasActiveLayoutChange());
        EXPECT_EQ(CategoryInfo::Layout::Sport_Track, value.getActiveLayout());
    }

    TEST_F(ACategoryInfo, canCopyConstruct)
    {
        CategoryInfo emptyCopy(empty);
        EXPECT_EQ(empty, emptyCopy);

        CategoryInfo withSizeCopy(filled);
        EXPECT_EQ(filled, withSizeCopy);
    }

    TEST_F(ACategoryInfo, canCopyAssign)
    {
        CategoryInfo emptyCopy;
        emptyCopy = empty;
        EXPECT_EQ(empty, emptyCopy);

        CategoryInfo withSizeCopy;
        withSizeCopy = filled;
        EXPECT_EQ(filled, withSizeCopy);
    }

    TEST_F(ACategoryInfo, canMoveContruct)
    {
        CategoryInfo emptyCopy(empty);
        CategoryInfo emptyMoved(std::move(emptyCopy));
        EXPECT_EQ(empty, emptyMoved);

        CategoryInfo filledCopy(filled);
        CategoryInfo filledMoved(std::move(filledCopy));
        EXPECT_EQ(filled, filledMoved);
    }

    TEST_F(ACategoryInfo, canMoveAssign)
    {
        CategoryInfo emptyCopy(empty);
        CategoryInfo emptyMoved;
        emptyMoved = std::move(emptyCopy);
        EXPECT_EQ(empty, emptyMoved);

        CategoryInfo filledCopy(filled);
        CategoryInfo filledMoved;
        filledMoved = std::move(filledCopy);
        EXPECT_EQ(filled, filledMoved);
    }

    TEST_F(ACategoryInfo, canSerializeDeserializeEmpty)
    {
        EXPECT_EQ(empty, serializeDeserialize(empty));
    }

    TEST_F(ACategoryInfo, canSerializeDeserializeFilled)
    {
        EXPECT_EQ(filled, serializeDeserialize(filled));
    }

    TEST_F(ACategoryInfo, canSerializeDeserializeSomeSet)
    {
        CategoryInfo ci;
        ci.setCategoryRect(4,3,2,1);
        ci.setRenderSize(5,6);
        ci.setSafeRect(7,8,9,0);
        ci.setActiveLayout(CategoryInfo::Layout::Autonomous);
        EXPECT_EQ(ci, serializeDeserialize(ci));
    }

    TEST_F(ACategoryInfo, canSkipDeserializeUnknownTypes)
    {
        BinaryOutputStream os;
        os << static_cast<uint32_t>(1) // version
            << static_cast<uint32_t>(2) // entries

            << static_cast<uint32_t>(55) // unknown type
            << static_cast<uint32_t>(16) // unknown size
            << static_cast<uint64_t>(2) // unknown data
            << static_cast<uint64_t>(3)

            << static_cast<uint32_t>(1) // category rect change type
            << static_cast<uint32_t>(2) // x
            << static_cast<uint32_t>(2) // y
            << static_cast<uint32_t>(2) // size
            << static_cast<uint32_t>(2) // width
            << static_cast<uint32_t>(123); // height

        CategoryInfo ci(os.release());
        EXPECT_TRUE(ci.hasCategoryRectChange());
        EXPECT_EQ(2u, ci.getCategoryWidth());
        EXPECT_EQ(123u, ci.getCategoryHeight());
    }

    TEST_F(ACategoryInfo, ignoresUnexpectedCategoryInfoDataVersion)
    {
        BinaryOutputStream os;
        os << static_cast<uint32_t>(100) // unsupported version
            << static_cast<uint32_t>(1) // entries

            << static_cast<uint32_t>(1) // size change type
            << static_cast<uint32_t>(2) // x
            << static_cast<uint32_t>(2) // y
            << static_cast<uint32_t>(2) // width
            << static_cast<uint8_t>(123); // height
        CategoryInfo ci(os.release());
        EXPECT_FALSE(ci.hasCategoryRectChange());
    }

    TEST_F(ACategoryInfo, canFormat)
    {
        EXPECT_EQ("[]", fmt::to_string(empty));
        EXPECT_EQ("[categoryRect:xy12:34 56x78;rendSize:88x99;safeRect:xy4:3 2x1;activeLayout:Layout::Focus]", fmt::to_string(filled));
    }

    TEST_F(ACategoryInfo, emptyInfoUpdatesSelfProperly)
    {
        CategoryInfo ci1;
        CategoryInfo ci2;
        ci2.setCategoryRect(4,3,2,1);
        ci2.setRenderSize(5,6);

        ci1.updateSelf(ci2);
        EXPECT_EQ(ci1, ci2);

        ci2.setSafeRect(7,8,9,0);
        ci2.setActiveLayout(CategoryInfo::Layout::Autonomous);

        ci1.updateSelf(ci2);
        EXPECT_EQ(ci1, ci2);
    }

    TEST_F(ACategoryInfo, nonemptyInfoUpdatesSelfProperly)
    {
        CategoryInfo ci1;
        ci1.setCategoryRect(44,33,22,11);
        ci1.setRenderSize(55,66);
        ci1.setSafeRect(77,88,99,90);
        ci1.setActiveLayout(CategoryInfo::Layout::Sport_Track);

        CategoryInfo ci2;
        ci2.setCategoryRect(4,3,2,1);
        ci2.setRenderSize(5,6);

        ci1.updateSelf(ci2);
        EXPECT_NE(ci1, ci2);
        EXPECT_EQ(ci1.getCategoryX(), ci2.getCategoryX());
        EXPECT_EQ(ci1.getCategoryY(), ci2.getCategoryY());
        EXPECT_EQ(ci1.getCategoryWidth(), ci2.getCategoryWidth());
        EXPECT_EQ(ci1.getCategoryHeight(), ci2.getCategoryHeight());
        EXPECT_EQ(ci1.getRenderSizeWidth(), ci2.getRenderSizeWidth());
        EXPECT_EQ(ci1.getRenderSizeHeight(), ci2.getRenderSizeHeight());
        EXPECT_NE(ci1.getSafeRectX(), ci2.getSafeRectX());
        EXPECT_NE(ci1.getSafeRectY(), ci2.getSafeRectY());
        EXPECT_NE(ci1.getSafeRectWidth(), ci2.getSafeRectWidth());
        EXPECT_NE(ci1.getSafeRectHeight(), ci2.getSafeRectHeight());
        EXPECT_NE(ci1.getActiveLayout(), ci2.getActiveLayout());
        EXPECT_EQ(ci1.getSafeRectX(), 77u);
        EXPECT_EQ(ci1.getSafeRectY(), 88u);
        EXPECT_EQ(ci1.getSafeRectWidth(), 99u);
        EXPECT_EQ(ci1.getSafeRectHeight(), 90u);
        EXPECT_EQ(ci1.getActiveLayout(), CategoryInfo::Layout::Sport_Track);

        ci2.setSafeRect(7,8,9,0);
        ci2.setActiveLayout(CategoryInfo::Layout::Autonomous);

        ci1.updateSelf(ci2);
        EXPECT_EQ(ci1, ci2);
    }
}
