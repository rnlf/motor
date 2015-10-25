#include <tgmath.h>
#include <lauxlib.h>
#include "graphics_mesh.h"
#include "graphics_image.h"
#include "tools.h"


static struct {
  int meshMT;
} moduleData;


static readVertices(lua_State* state, graphics_Vertex** output) {
  if(!lua_istable(state, 2)) {

  }
}


static int l_graphics_newMesh(lua_State* state) {
  l_graphics_Mesh* mesh = lua_newuserdata(state, sizeof(l_graphics_Mesh));

  
  

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
  l_assertType(state, 2, l_graphics_isImage);

  l_graphics_Mesh * mesh = l_graphics_toMesh(state, 1);
  l_graphics_Image * image = l_graphics_toImage(state, 2);

  mesh->mesh.texture = &image->image;
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


l_checkTypeFn(l_graphics_isMesh, moduleData.meshMT)
l_toTypeFn(l_graphics_toMesh, l_graphics_Mesh)

static luaL_Reg const meshMetatableFuncs[] = {
  {"__gc",               l_graphics_gcMesh},
  {"getCount",           l_graphics_Mesh_getCount},
  {"setTexture",         l_graphics_Mesh_setTexture},
  {"setImage",           l_graphics_Mesh_setTexture},
  {"getTexture",         l_graphics_Mesh_getTexture},
  {"getImage",           l_graphics_Mesh_getTexture},
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
