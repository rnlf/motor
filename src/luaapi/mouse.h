/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once


#include "../mouse.h"
#include "tools.h"
bool l_mouse_isCursor(lua_State* state, int index);
mouse_Cursor* l_mouse_toCursor(lua_State* state, int index);
void l_mouse_register(lua_State* state);
