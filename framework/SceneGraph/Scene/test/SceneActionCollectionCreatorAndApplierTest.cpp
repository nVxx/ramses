//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "Scene/SceneActionApplier.h"
#include "Components/FlushTimeInformation.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace ramses_internal
{
    class ASceneActionCollectionCreatorAndApplier : public ::testing::Test
    {
    public:
        ASceneActionCollectionCreatorAndApplier()
            : creator(collection)
        {
        }

        void readFlushByIndex(UInt idx)
        {
            ASSERT_TRUE(idx < collection.numberOfActions());
            SceneActionApplier::ReadParameterForFlushAction(collection[idx], flushIdx, hasSizeInfo, sizeInfo, resourceChanges, timeInfo, versionTag);
        }

        SceneActionCollection collection;
        SceneActionCollectionCreator creator;

        const SceneSizeInformation sizeInfoIn{1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u, 17u, 18u};

        UInt64 flushIdx = 0;
        bool hasSizeInfo = false;
        SceneSizeInformation sizeInfo;
        SceneResourceChanges resourceChanges;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
    };

    TEST_F(ASceneActionCollectionCreatorAndApplier, createsExpectedNumberAndTypeOfActions)
    {
        creator.flush(1u, false);
        creator.allocateNode(0, NodeHandle(1u));
        creator.flush(1u, false);
        creator.flush(1u, false);
        creator.allocateRenderState(RenderStateHandle(2u));

        ASSERT_EQ(5u, collection.numberOfActions());
        EXPECT_EQ(ESceneActionId_Flush, collection[0].type());
        EXPECT_EQ(ESceneActionId_AllocateNode, collection[1].type());
        EXPECT_EQ(ESceneActionId_Flush, collection[2].type());
        EXPECT_EQ(ESceneActionId_Flush, collection[3].type());
        EXPECT_EQ(ESceneActionId_AllocateRenderState, collection[4].type());
    }

    TEST_F(ASceneActionCollectionCreatorAndApplier, createsAndReadsExpectedFlushIdx)
    {
        creator.flush(1u, false);
        creator.flush(3u, false);
        creator.flush(2u, false);

        readFlushByIndex(0);
        EXPECT_EQ(1u, flushIdx);

        readFlushByIndex(1);
        EXPECT_EQ(3u, flushIdx);

        readFlushByIndex(2);
        EXPECT_EQ(2u, flushIdx);
    }

    TEST_F(ASceneActionCollectionCreatorAndApplier, ignoresSizeInfoWhenFlagsSaisNotProvided)
    {
        creator.flush(1u, false, sizeInfoIn);
        readFlushByIndex(0);
        EXPECT_FALSE(hasSizeInfo);
        EXPECT_EQ(SceneSizeInformation(), sizeInfo);
    }

    TEST_F(ASceneActionCollectionCreatorAndApplier, hasExpectedSizeInfoWhenGiven)
    {
        creator.flush(1u, true, sizeInfoIn);
        creator.flush(1u, true, SceneSizeInformation());

        readFlushByIndex(0);
        EXPECT_TRUE(hasSizeInfo);
        EXPECT_EQ(sizeInfoIn, sizeInfo);

        readFlushByIndex(1);
        EXPECT_TRUE(hasSizeInfo);
        EXPECT_EQ(SceneSizeInformation(), sizeInfo);
    }

    TEST_F(ASceneActionCollectionCreatorAndApplier, canReadFlushTimeInfo)
    {
        const FlushTimeInformation timeInfo0{ FlushTime::Clock::time_point(std::chrono::milliseconds(20)), FlushTime::Clock::time_point(std::chrono::milliseconds(30)) };
        const FlushTimeInformation timeInfo1{ FlushTime::Clock::time_point(std::chrono::milliseconds(200)), FlushTime::Clock::time_point(std::chrono::milliseconds(300)) };

        creator.flush(1u, false, SceneSizeInformation(), SceneResourceChanges(), timeInfo0);
        creator.flush(2u, false, SceneSizeInformation(), SceneResourceChanges(), timeInfo1);

        readFlushByIndex(0);
        EXPECT_EQ(timeInfo0, timeInfo);

        readFlushByIndex(1);
        EXPECT_EQ(timeInfo1, timeInfo);
    }

    TEST_F(ASceneActionCollectionCreatorAndApplier, canReadFlushTimeInfoIfExpirationTimestampNotSet)
    {
        const FlushTimeInformation timeInfoIn{ FlushTime::InvalidTimestamp, FlushTime::Clock::time_point(std::chrono::milliseconds(30)) };
        creator.flush(1u, false, SceneSizeInformation(), SceneResourceChanges(), timeInfoIn);

        readFlushByIndex(0);
        EXPECT_EQ(timeInfoIn, timeInfo);
    }

    TEST_F(ASceneActionCollectionCreatorAndApplier, canReadVersionTagFromFlush)
    {
        const SceneVersionTag versionTagIn{ 333 };
        creator.flush(1u, false, SceneSizeInformation(), SceneResourceChanges(), {}, versionTagIn);

        readFlushByIndex(0);
        EXPECT_EQ(versionTagIn, versionTag);
    }
}
