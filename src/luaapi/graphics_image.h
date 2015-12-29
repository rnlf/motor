/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <lua.h>
#include "image.h"
#include "../graphics/image.h"

typedef struct {
  graphics_Image image;
  int imageDataRef;
} l_graphics_Image;

void l_graphics_image_register(lua_State* state);
bool l_graphics_isImage(lua_State* state, int index);
l_graphics_Image* l_graphics_toImage(lua_State* state, int index);
int l_graphics_newImage(lua_State* state);
