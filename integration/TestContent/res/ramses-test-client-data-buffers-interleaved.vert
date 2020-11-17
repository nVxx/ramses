//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform highp mat4 mvpMatrix;

attribute float a_positionX;
attribute vec2 a_positionYZ;
attribute float a_positionW;

void main()
{
    gl_Position = mvpMatrix * vec4(a_positionX, a_positionYZ, a_positionW);
}
