//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMANDTEST_H
#define RAMSES_COMMANDTEST_H

#include "CommandT.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses_internal
{
    enum E_TestCommandTypes
    {
        E_TestCommandTypes_DoThis = 0,
        E_TestCommandTypes_DoNothing
    };

    using CustomCommand = Command<E_TestCommandTypes>;

    struct DoThisCommand : public CustomCommand
    {
        DEFINE_COMMAND_TYPE(DoThisCommand, E_TestCommandTypes_DoThis)

        explicit DoThisCommand(uint32_t v)
        : CustomCommand(E_TestCommandTypes_DoThis)
        , value(v)
        {}

        uint32_t value;
    };


    struct DoNothingCommand : public CustomCommand
    {
        DEFINE_COMMAND_TYPE(DoNothingCommand, E_TestCommandTypes_DoNothing)
    };
}

#endif // RAMSES_COMMANDTEST_H
