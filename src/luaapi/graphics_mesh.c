#include <stdlib.h>
#include <tgmath.h>
#include <lauxlib.h>
#include "graphics_mesh.h"
#include "graphics_texture.h"
#include "tools.h"


static const l_tools_Enum l_graphics_MeshDrawMode[] = {
  {"fan",       graphics_MeshDrawMode_fan},
  {"strip",     graphics_MeshDrawMode_strip},
  {"triangles", graphics_MeshDrawMode_triangles},
  {"points",    graphics_MeshDrawMode_points},
  {NULL, 0}
};


static struct {
  int meshMT;
  graphics_Vertex* vertexBuffer;
  size_t vertexBufferSize;
} moduleData;


static void ensureVertexBufferSize(size_t count) {
  if(moduleData.vertexBufferSize < count) {
    free(moduleData.vertexBuffer);
    moduleData.vertexBuffer = malloc(count * sizeof(graphics_Vertex));
  }
}


static void readVertex(lua_State* state, graphics_Vertex* out, bool *hasVertexColor) {
  if(!lua_istable(state, -1) || lua_objlen(state, -1) < 4) {
    lua_pushstring(state, "Table entry is not a vertex");
    lua_error(state); // does not return
    return;           // hint the compiler
  }

  _Static_assert(sizeof(graphics_Vertex) == 8*sizeof(float), "");
  float *t = (float*)out;

  for(int i = 0; i < 4; ++i) {
    lua_rawgeti(state, -1, i+1);
    t[i] = l_tools_toNumberOrError(state, -1);
    lua_pop(state, 1);
  }

  for(int i = 4; i < 8; ++i) {
    lua_rawgeti(state, -1, i+1);
    t[i] = luaL_optnumber(state, -1, 255.0f) / 255.0f;
    *hasVertexColor = (*hasVertexColor) || t[i] != 1.0f;
    lua_pop(state, 1);
  }
}


static size_t readVertices(lua_State* state, bool *hasVertexColor) {
  if(!lua_istable(state, 1)) {
    lua_pushstring(state, "Need table of vertices");
    lua_error(state); // does not return
    return 0;         // hint the compiler
  }

  size_t count = lua_objlen(state, 1);
  ensureVertexBufferSize(count);

  *hasVertexColor = false;
  for(size_t i = 0; i < count; ++i) {
    lua_rawgeti(state, 1, i+1);
    readVertex(state, moduleData.vertexBuffer + i, hasVertexColor);
    lua_pop(state, 1);
  }

  return count;
}


static int l_graphics_newMesh(lua_State* state) {
  bool useVertexColor;
  size_t count = readVertices(state, &useVertexColor);
  graphics_Image const* texture = l_graphics_toTextureOrError(state, 2);
  graphics_MeshDrawMode mode = l_tools_toEnumOrError(state, 3, l_graphics_MeshDrawMode);

  l_graphics_Mesh* mesh = lua_newuserdata(state, sizeof(l_graphics_Mesh));
  graphics_Mesh_new(&mesh->mesh, count, moduleData.vertexBuffer, texture, mode, useVertexColor);

  lua_pushvalue(state, 2);
  mesh->textureRef = luaL_ref(state, LUA_REGISTRYINDEX);

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.meshMT);
  lua_setmetatable(state, -2);
  return 1;
}


static int l_graphics_gcMesh(lua_State* state) {
  l_graphics_Mesh *mesh = l_graphics_toMesh(state, 1);
  graphics_Mesh_free(&mesh->mesh);
  luaL_unref(state, LUA_REGISTRYINDEX, mesh->textureRef);

  return 0;
}


static int l_graphics_Mesh_getCount(lua_State* state) {
  l_assertType(state, 1, l_graphics_isMesh);

  l_graphics_Mesh * mesh = l_graphics_toMesh(state, 1);

  lua_pushnumber(state, mesh->mesh.vertexCount);

  return 1;
}


static int l_graphics_Mesh_setTexture(lua_State* state) {
  l_assertType(state, 1, l_graphics_isMesh);
  graphics_Image const* texture = l_graphics_toTextureOrError(state, 2);

  l_graphics_Mesh * mesh = l_graphics_toMesh(state, 1);

  mesh->mesh.texture = texture;
  luaL_unref(state, LUA_REGISTRYINDEX, mesh->textureRef);
  lua_settop(state, 2);
  mesh->textureRef = luaL_ref(state, LUA_REGISTRYINDEX);

  return 0;
}

static int l_graphics_Mesh_getTexture(lua_State* state) {
  l_assertType(state, 1, l_graphics_isMesh);

  l_graphics_Mesh * mesh = l_graphics_toMesh(state, 1);

  lua_rawgeti(state, LUA_REGISTRYINDEX, mesh->textureRef);

  return 1;
}


static int l_graphics_Mesh_setVertexColors(lua_State* state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh * mesh = l_graphics_toMesh(state, 1);

  bool use = l_tools_toBooleanOrError(state, 2);

  graphics_Mesh_setVertexColors(&mesh->mesh, use);

  return 0;
}


static int l_graphics_Mesh_getVertexColors(lua_State *state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh * mesh = l_graphics_toMesh(state, 1);
  lua_pushboolean(state, graphics_Mesh_getVertexColors(&mesh->mesh));
  return 1;
}


static int l_graphics_Mesh_setDrawRange(lua_State *state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh * mesh = l_graphics_toMesh(state, 1);

  int top = lua_gettop(state);
  if(top == 3) {
    int idx1 = l_tools_toNumberOrError(state, 2) - 1;
    int idx2 = l_tools_toNumberOrError(state, 3) - 1;
    graphics_Mesh_setDrawRange(&mesh->mesh, idx1, idx2);
  } else if(top == 1) {
    graphics_Mesh_resetDrawRange(&mesh->mesh);
  } else {
    lua_pushstring(state, "Need none or two numbers");
    return lua_error(state);
  }

  return 0;
}


static int l_graphics_Mesh_getDrawRange(lua_State *state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh const* mesh = l_graphics_toMesh(state, 1);

  int idx1, idx2;
  if(graphics_Mesh_getDrawRange(&mesh->mesh, &idx1, &idx2)) {
    lua_pushnumber(state, idx1);
    lua_pushnumber(state, idx2);
    return 2;
  }
  return 0;
}


l_checkTypeFn(l_graphics_isMesh, moduleData.meshMT)
l_toTypeFn(l_graphics_toMesh, l_graphics_Mesh)

static luaL_Reg const meshMetatableFuncs[] = {
  {"__gc",               l_graphics_gcMesh},
  {"getCount",           l_graphics_Mesh_getCount},
  {"setTexture",         l_graphics_Mesh_setTexture},
  {"setImage",           l_graphics_Mesh_setTexture},
  {"getTexture",         l_graphics_Mesh_getTexture},
  {"getImage",           l_graphics_Mesh_getTexture},
  {"setVertexColors",    l_graphics_Mesh_setVertexColors},
  {"getVertexColors",    l_graphics_Mesh_getVertexColors},
  {"setDrawRange",       l_graphics_Mesh_setDrawRange},
  {"getDrawRange",       l_graphics_Mesh_getDrawRange},
  {NULL, NULL}
};

static luaL_Reg const meshFreeFuncs[] = {
  {"newMesh",            l_graphics_newMesh},
  {NULL, NULL}
};

void l_graphics_mesh_register(lua_State* state) {
  l_tools_registerFuncsInModule(state, "graphics", meshFreeFuncs);
  moduleData.meshMT  = l_tools_makeTypeMetatable(state, meshMetatableFuncs);
}
