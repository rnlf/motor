/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "tools.h"

int l_geometry_read_vertices(lua_State* state, int offset, float **vertices, int minVerts) {
  return l_tools_readNumbers(state, offset, vertices, minVerts, 2);
}
