/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <lua.h>
#include "tools.h"
#include "../graphics/image.h"

int l_graphics_register(lua_State* state);

static const l_tools_Enum l_graphics_WrapMode[] = {
  {"clamp", graphics_WrapMode_clamp},
  {"repeat", graphics_WrapMode_repeat},
  {NULL, 0}
};

static const l_tools_Enum l_graphics_FilterMode[] = {
  {"nearest", graphics_FilterMode_nearest},
  {"linear",  graphics_FilterMode_linear},
  {NULL, 0}
};

