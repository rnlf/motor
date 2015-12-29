/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include "tools.h"
#include "graphics_image.h"

graphics_Image* l_graphics_toTextureOrError(lua_State* state, int index);
