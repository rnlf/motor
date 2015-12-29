/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <stdbool.h>
#include <lua.h>
#include "../math/bezier.h"

void l_math_bezier_register(lua_State* state);
bool l_math_isBezierCurve(lua_State* state, int index);
math_BezierCurve* l_math_toBezierCurve(lua_State* state, int index);
