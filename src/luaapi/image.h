/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <lua.h>
#include <stdbool.h>
#include "../image/imagedata.h"

bool l_image_isImageData(lua_State* state, int index);
image_ImageData* l_image_toImageData(lua_State* state, int index);
int l_image_register(lua_State* state);
int l_image_newImageData(lua_State* state);
