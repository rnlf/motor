#include <stdlib.h>
#include "tools.h"

static struct {
  float * vertices;
  int vertSize;
} moduleData;


int l_geometry_read_vertices(lua_State* state, int offset, float **vertices, int minVerts) {
  bool table = lua_istable(state, 1 + offset);

  int count;
  if(table) {
    count = lua_objlen(state, 1 + offset);
  } else {
    count = lua_gettop(state) - offset;
  }

  if(count % 2) {
    lua_pushstring(state, "Need even number of values");
    return lua_error(state);
  }

  if(count < minVerts) {
    lua_pushstring(state, "Need at least ");
    lua_pushnumber(state, minVerts);
    lua_pushstring(state, " numbers");
    lua_concat(state, 3);
    return lua_error(state);
  }

  if(count > moduleData.vertSize) {
    moduleData.vertices = realloc(moduleData.vertices, count * sizeof(float));
    moduleData.vertSize = count;
  }

  if(table) {
    for(int i = 0; i < count; ++i) {
      lua_rawgeti(state, 1+offset, i+1);
      moduleData.vertices[i] = l_tools_toNumberOrError(state, -1);
      lua_pop(state, 1);
    }
  } else {
    for(int i = 0; i < count; ++i) {
      moduleData.vertices[i] = l_tools_toNumberOrError(state, 1 + i + offset);
    }
  }

  *vertices = moduleData.vertices;

  return count / 2;
}
