#pragma once

#include <lua.h>

int l_geometry_read_vertices(lua_State* state, int offset, float **vertices, int minVerts);
