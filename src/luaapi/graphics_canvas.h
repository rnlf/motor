/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <lua.h>
#include "../graphics/canvas.h"

void l_graphics_canvas_register(lua_State * state);
int l_graphics_newCanvas(lua_State* state);
bool l_graphics_isCanvas(lua_State* state, int index);
graphics_Canvas* l_graphics_toCanvas(lua_State* state, int index);
