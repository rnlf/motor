#pragma once

#include <stdbool.h>
#include <lua.h>
#include "../graphics/mesh.h"

typedef struct {
  graphics_Mesh mesh;
  int textureRef;
} l_graphics_Mesh;

int l_graphics_newMesh(lua_State* state);
void l_graphics_mesh_register(lua_State* state);
bool l_graphics_isMesh(lua_State* state, int index);
l_graphics_Mesh* l_graphics_toMesh(lua_State* state, int index);
