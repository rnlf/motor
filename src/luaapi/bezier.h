#pragma once

#include <stdbool.h>
#include <lua.h>
#include "../math/bezier.h"

void l_math_bezier_register(lua_State* state);
bool l_math_isBezierCurve(lua_State* state, int index);
math_BezierCurve* l_math_toBezierCurve(lua_State* state, int index);
