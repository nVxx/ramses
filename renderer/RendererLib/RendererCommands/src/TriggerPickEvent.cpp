//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererCommands/TriggerPickEvent.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "Ramsh/RamshInput.h"
#include "Math3d/Vector2i.h"

namespace ramses_internal
{
    TriggerPickEvent::TriggerPickEvent(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "Pick scene at X and Y coordinates normalized to -1,1 with bottom left being -1,1";
        registerKeyword("pick");
        getArgument<0>().setDescription("scene id");
        getArgument<1>().setDescription("normalized pick coordinate X");
        getArgument<2>().setDescription("normalized pick coordinate Y");
    }

    Bool TriggerPickEvent::execute(UInt64& sceneId, Float& pickCoordX, Float& pickCoordY) const
    {
        m_rendererCommandBuffer.handlePickEvent(SceneId(sceneId), Vector2(pickCoordX, pickCoordY));
        return true;
    }
}
