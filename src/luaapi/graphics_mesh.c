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
  void* buffer;
  size_t bufferSize;
} moduleData;


static void ensureBufferSize(size_t size) {
  if(moduleData.bufferSize < size) {
    free(moduleData.buffer);
    moduleData.buffer = malloc(size); // count * sizeof(graphics_Vertex));
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


static size_t readVertices(lua_State* state, bool *hasVertexColor, int base) {
  if(!lua_istable(state, base)) {
    lua_pushstring(state, "Need table of vertices");
    lua_error(state); // does not return
    return 0;         // hint the compiler
  }

  size_t count = lua_objlen(state, base);
  ensureBufferSize(count * sizeof(graphics_Vertex));

  *hasVertexColor = false;
  for(size_t i = 0; i < count; ++i) {
    lua_rawgeti(state, base, i+1);
    readVertex(state, ((graphics_Vertex*)moduleData.buffer) + i, hasVertexColor);
    lua_pop(state, 1);
  }

  return count;
}


static int l_graphics_newMesh(lua_State* state) {
  bool useVertexColor;
  size_t count = readVertices(state, &useVertexColor, 1);
  graphics_Image const* texture = l_graphics_toTextureOrError(state, 2);
  graphics_MeshDrawMode mode = l_tools_toEnumOrError(state, 3, l_graphics_MeshDrawMode);

  l_graphics_Mesh* mesh = lua_newuserdata(state, sizeof(l_graphics_Mesh));
  graphics_Mesh_new(&mesh->mesh, count, (graphics_Vertex*)moduleData.buffer, texture, mode, useVertexColor);

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


static int l_graphics_Mesh_setVertexMap(lua_State *state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh *mesh = l_graphics_toMesh(state, 1);
  
  size_t top = (size_t)lua_gettop(state);
  if(top == 1) {
    graphics_Mesh_setVertexMap(&mesh->mesh, 0, 0);
    return 0;
  } 
  
  size_t count;
  if(top > 2) {
    ensureBufferSize(sizeof(uint32_t) * (top - 1));
    for(int i = 0; i < top - 1; ++i) {
      ((uint32_t*)moduleData.buffer)[i] = (uint32_t)l_tools_toNumberOrError(state, i + 2) - 1;
      printf("Index: %d\n", ((uint32_t*)moduleData.buffer)[i]);
    }
    count = top - 1;
  } else {
    size_t len = (size_t)lua_objlen(state, 2);
    ensureBufferSize(len * sizeof(uint32_t));
    for(int i = 0; i < len; ++i) {
      lua_rawgeti(state, 2, i+1);
      ((uint32_t*)moduleData.buffer)[i] = (uint32_t)l_tools_toNumberOrError(state, -1) - 1;
    }
    count = len;
  }

  graphics_Mesh_setVertexMap(&mesh->mesh, count, (uint32_t*)moduleData.buffer);

  return 0;
}


#define makePushVertexMapFunc(type)                                                     \
  static void pushVertexMap_ ## type(lua_State *state, void const* buf, size_t count) { \
    type const* b = (type const*)buf;                                                   \
    for(size_t i = 0; i < count; ++i) {                                                 \
      lua_pushnumber(state, ((int)b[i]) + 1);                                           \
      lua_rawseti(state, -2, i+1);                                                      \
    }                                                                                   \
  }
makePushVertexMapFunc(uint8_t)
makePushVertexMapFunc(uint16_t)
makePushVertexMapFunc(uint32_t)
#undef makePushVertexMapFunc

static int l_graphics_Mesh_getVertexMap(lua_State *state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh const* mesh = l_graphics_toMesh(state, 1);
  
  size_t count;
  void const* buf = graphics_Mesh_getVertexMap(&mesh->mesh, &count);
  lua_createtable(state, count, 0);
  if(count > 0xFFFF) {
    pushVertexMap_uint32_t(state, buf, count);
  } else if(count > 0xFF) {
    pushVertexMap_uint16_t(state, buf, count);
  } else {
    pushVertexMap_uint8_t(state, buf, count);
  }
  
  return 1;
}


static int l_graphics_Mesh_setVertices(lua_State* state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh *mesh = l_graphics_toMesh(state, 1);

  bool hasColor;
  size_t count = readVertices(state, &hasColor, 2);

  graphics_Mesh_setVertices(&mesh->mesh, count, (graphics_Vertex const*)moduleData.buffer);

  return 0;
}


static int l_graphics_Mesh_getVertices(lua_State* state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh const* mesh = l_graphics_toMesh(state, 1);

  size_t count;
  graphics_Vertex const *vertices = graphics_Mesh_getVertices(&mesh->mesh, &count);

  lua_createtable(state, count, 0);
  for(size_t i = 0; i < count; ++i) {
    lua_createtable(state, 8, 0);
    float const *vertex = (float const*)(vertices+i);
    for(size_t j = 0; j < 4; ++j) {
      lua_pushnumber(state, vertex[j]);
      lua_rawseti(state, -2, j + 1);
    }
    for(size_t j = 4; j < 8; ++j) {
      lua_pushnumber(state, floor(vertex[j] * 255.0f));
      lua_rawseti(state, -2, j + 1);
    }
    lua_rawseti(state, -2, i + 1);
  }

  return 1;
}


static int l_graphics_Mesh_getVertex(lua_State* state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh const* mesh = l_graphics_toMesh(state, 1);

  size_t index = l_tools_toNumberOrError(state, 2) - 1;

  graphics_Vertex const* vertex = graphics_Mesh_getVertex(&mesh->mesh, index);
  float const* vf = (float const*)vertex;

  for(size_t j = 0; j < 4; ++j) {
    lua_pushnumber(state, vf[j]);
  }
  for(size_t j = 4; j < 8; ++j) {
    lua_pushnumber(state, floor(vf[j] * 255.0f));
  }

  return 8;
}


static int l_graphics_Mesh_setVertex(lua_State* state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh * mesh = l_graphics_toMesh(state, 1);

  size_t index = l_tools_toNumberOrError(state, 2) - 1;

  graphics_Vertex v;
  float *vf = (float*)&v;
  for(int i = 0; i < 4; ++i) {
    vf[i] = l_tools_toNumberOrError(state, i+3);
  }
  for(int i = 4; i < 8; ++i) {
    vf[i] = luaL_optnumber(state, i+3, 255.0f) / 255.0f;
  }

  graphics_Mesh_setVertex(&mesh->mesh, index, &v);

  return 0;
}


static int l_graphics_Mesh_getVertexCount(lua_State *state) {
  l_assertType(state, 1, l_graphics_isMesh);
  l_graphics_Mesh const *mesh = l_graphics_toMesh(state, 1);

  lua_pushnumber(state, graphics_Mesh_getVertexCount(&mesh->mesh));
  return 1;
}


l_checkTypeFn(l_graphics_isMesh, moduleData.meshMT)
l_toTypeFn(l_graphics_toMesh, l_graphics_Mesh)

static luaL_Reg const meshMetatableFuncs[] = {
  {"__gc",               l_graphics_gcMesh},
  {"setTexture",         l_graphics_Mesh_setTexture},
  {"setImage",           l_graphics_Mesh_setTexture},
  {"getTexture",         l_graphics_Mesh_getTexture},
  {"getImage",           l_graphics_Mesh_getTexture},
  {"setVertexColors",    l_graphics_Mesh_setVertexColors},
  {"getVertexColors",    l_graphics_Mesh_getVertexColors},
  {"setDrawRange",       l_graphics_Mesh_setDrawRange},
  {"getDrawRange",       l_graphics_Mesh_getDrawRange},
  {"setVertexMap",       l_graphics_Mesh_setVertexMap},
  {"getVertexMap",       l_graphics_Mesh_getVertexMap},
  {"setVertices",        l_graphics_Mesh_setVertices},
  {"getVertices",        l_graphics_Mesh_getVertices},
  {"getVertex",          l_graphics_Mesh_getVertex},
  {"setVertex",          l_graphics_Mesh_setVertex},
  {"getVertexCount",     l_graphics_Mesh_getVertexCount},
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
