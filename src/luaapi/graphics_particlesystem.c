#include <stdlib.h>
#include <tgmath.h>
#include <lauxlib.h>
#include "tools.h"
#include "../graphics/particlesystem.h"
#include "graphics_quad.h"
#include "graphics_texture.h"
#include "graphics_particlesystem.h"


static const l_tools_Enum l_graphics_AreaSpreadDistribution[] = {
  {"uniform", graphics_AreaSpreadDistribution_uniform},
  {"normal",  graphics_AreaSpreadDistribution_normal},
  {"none",    graphics_AreaSpreadDistribution_none},
  {NULL, 0}
};


static const l_tools_Enum l_graphics_ParticleInsertMode[] = {
  {"top", graphics_ParticleInsertMode_top},
  {"bottom", graphics_ParticleInsertMode_bottom},
  {"random", graphics_ParticleInsertMode_random},
  {NULL, 0}
};

static struct {
  int particleSystemMT;
  void* buffer;
  size_t bufferSize;
} moduleData;


static void ensureBufferSize(size_t t) {
  if(moduleData.bufferSize < t) {
    free(moduleData.buffer);
    moduleData.buffer = malloc(t);
    moduleData.bufferSize = t;
  }
}


static int l_graphics_newParticleSystem(lua_State *state) {
  graphics_Image *texture = l_graphics_toTextureOrError(state, 1);
  int size = l_tools_toNumberOrError(state, 2);

  l_graphics_ParticleSystem *ps = lua_newuserdata(state, sizeof(l_graphics_ParticleSystem));
  graphics_ParticleSystem_new(&ps->particleSystem, texture, size);

  lua_pushvalue(state, 1);
  ps->textureRef = lua_ref(state, LUA_REGISTRYINDEX);
  ps->quadTableRef = LUA_NOREF;

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.particleSystemMT);
  lua_setmetatable(state, -2);

  return 1;
}


static int l_graphics_gcParticleSystem(lua_State *state) {
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);
  graphics_ParticleSystem_free(&ps->particleSystem);
  luaL_unref(state, LUA_REGISTRYINDEX, ps->textureRef);

  return 0;
}


static int l_graphics_ParticleSystem_setAreaSpread(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);
  graphics_AreaSpreadDistribution dist = l_tools_toEnumOrError(state, 2, l_graphics_AreaSpreadDistribution);
  float dx = l_tools_toNumberOrError(state, 3);
  float dy = l_tools_toNumberOrError(state, 4);

  graphics_ParticleSystem_setAreaSpread(&ps->particleSystem, dist, dx, dy);

  return 0;
}


static int l_graphics_ParticleSystem_getAreaSpread(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);
  
  graphics_AreaSpreadDistribution mode;
  float dx, dy;
  graphics_ParticleSystem_getAreaSpread(&ps->particleSystem, &mode, &dx, &dy);

  l_tools_pushEnum(state, mode, l_graphics_AreaSpreadDistribution);
  return 1;
}


static int l_graphics_ParticleSystem_setColors(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);

  int top = lua_gettop(state);
  if(top < 2) {
    lua_pushstring(state, "at least one color required");
    return lua_error(state);
  }

  size_t count;
  if(lua_istable(state, 2)) {
    count = top - 1;
    ensureBufferSize(sizeof(graphics_Color) * count);
    graphics_Color *colors = moduleData.buffer;
    _Static_assert(sizeof(graphics_Color) == 4*sizeof(float), "who changed graphics_Color??");

    for(int i = 0; i < count; ++i) {
      float *col = (float*)(colors+i);
      for(int j = 0; j < 3; ++j) {
        lua_rawgeti(state, i + 2, j + 1);
        col[j] = l_tools_toNumberOrError(state, -1) / 255.0f;
      }
      lua_rawgeti(state, i + 2, 4);
      col[3] = luaL_optnumber(state, -1, 255.0) / 255.0f;
      lua_pop(state, 4);
    }
  } else {
    if((top - 1) % 4 != 0) {
      lua_pushstring(state, "need tables or multiple of 4 color components");
      return lua_error(state);
    }

    count = (top - 1) / 4;
    ensureBufferSize(sizeof(graphics_Color) * count);
    float *colors = moduleData.buffer;
    for(int i = 0; i < count * 4; ++i) {
      colors[i] = l_tools_toNumberOrError(state, 2+i) / 255.0f;
    }
  }

  graphics_ParticleSystem_setColors(&ps->particleSystem, count, moduleData.buffer);

  return 0;
}


static int l_graphics_ParticleSystem_getColors(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);

  size_t count;
  graphics_Color const *colors = graphics_ParticleSystem_getColors(&ps->particleSystem, &count);

  for(int i = 0; i < count; ++i) {
    float const *color = (float const*)(colors + i);
    for(int j = 0; j < 4; ++j) {
      lua_pushnumber(state, floor(color[j] * 255.0f));
    }
  }

  return count * 4;
}


static int l_graphics_ParticleSystem_setTexture(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);

  graphics_Image *texture = l_graphics_toTextureOrError(state, 2);
  luaL_unref(state, LUA_REGISTRYINDEX, ps->textureRef);
  lua_settop(state, 2);

  ps->textureRef = luaL_ref(state, LUA_REGISTRYINDEX);

  graphics_ParticleSystem_setTexture(&ps->particleSystem, texture);

  return 0;
}


static int l_graphics_ParticleSystem_getTexture(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);

  lua_rawgeti(state, LUA_REGISTRYINDEX, ps->textureRef);

  return 1;
}


static int l_graphics_ParticleSystem_setSizes(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);

  int top = lua_gettop(state);
  if(top == 1) {
    lua_pushstring(state, "need at least one size");
    return lua_error(state);
  }

  ensureBufferSize(sizeof(float) * (top -1 ));
  float *sizes = (float*)moduleData.buffer;

  for(int i = 0; i < top-1; ++i) {
    sizes[i] = l_tools_toNumberOrError(state, 2+i);
  }

  graphics_ParticleSystem_setSizes(&ps->particleSystem, top-1, sizes);

  return 0;
}


static int l_graphics_ParticleSystem_getSizes(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);

  size_t count;
  float const *sizes = graphics_ParticleSystem_getSizes(&ps->particleSystem, &count);

  for(int i = 0; i < count; ++i) {
    lua_pushnumber(state, sizes[i]);
  }

  return count;
}


static int l_graphics_ParticleSystem_setQuads(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);

  
  int top = lua_gettop(state);
  if(top == 1) {
    lua_pushstring(state, "need at least one quad");
    return lua_error(state);
  }

  ensureBufferSize(sizeof(graphics_Quad*) * (top -1 ));
  graphics_Quad const **quads = (graphics_Quad const**)moduleData.buffer;

  lua_createtable(state, top - 1, 0);
  for(int i = 0; i < top-1; ++i) {
    l_assertType(state, 2+i, l_graphics_isQuad);
    quads[i] = l_graphics_toQuad(state, 2+i);
    lua_pushvalue(state, 2+i);
    lua_rawseti(state, -2, i+1);
  }

  luaL_ref(state, LUA_REGISTRYINDEX);

  graphics_ParticleSystem_setQuads(&ps->particleSystem, top-1, quads);

  return 0;
}


static int l_graphics_ParticleSystem_setRelativeRotation(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);

  bool enable = l_tools_toBooleanOrError(state, 2);

  graphics_ParticleSystem_setRelativeRotation(&ps->particleSystem, enable);

  return 0;
}


static int l_graphics_ParticleSystem_setInsertMode(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);
  graphics_ParticleInsertMode mode = l_tools_toEnumOrError(state, 2, l_graphics_ParticleInsertMode);

  graphics_ParticleSystem_setInsertMode(&ps->particleSystem, mode);

  return 0;
}


static int l_graphics_ParticleSystem_getInsertMode(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);
  
  graphics_ParticleInsertMode mode = graphics_ParticleSystem_getInsertMode(&ps->particleSystem);

  l_tools_pushEnum(state, mode, l_graphics_ParticleInsertMode);

  return 1;
}


static int l_graphics_ParticleSystem_clone(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);

  lua_settop(state, 1);
  
  l_graphics_ParticleSystem *psNew = lua_newuserdata(state, sizeof(l_graphics_ParticleSystem));
  graphics_ParticleSystem_clone(&ps->particleSystem, &psNew->particleSystem);

  lua_rawgeti(state, LUA_REGISTRYINDEX, ps->textureRef);
  psNew->textureRef = luaL_ref(state, LUA_REGISTRYINDEX);

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.particleSystemMT);
  lua_setmetatable(state, -2);

  lua_rawgeti(state, LUA_REGISTRYINDEX, ps->quadTableRef);
  int quadCount = lua_objlen(state, -1);
  lua_createtable(state, quadCount, 0);
  for(int i = 0; i < quadCount; ++i) {
    lua_rawgeti(state, -2, i+1);
    lua_rawseti(state, -2, i+1);
  }
  psNew->quadTableRef = luaL_ref(state, LUA_REGISTRYINDEX);

  return 1;
}


static int l_graphics_ParticleSystem_setLinearAcceleration(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);

  float xmin = l_tools_toNumberOrError(state, 2);
  float ymin = l_tools_toNumberOrError(state, 3);
  float xmax = luaL_optnumber(state, 4, xmin);
  float ymax = luaL_optnumber(state, 5, ymin);

  graphics_ParticleSystem_setLinearAcceleration(&ps->particleSystem, xmin, ymin, xmax, ymax);

  return 0;
}


static int l_graphics_ParticleSystem_getLinearAcceleration(lua_State *state) {
  l_assertType(state, 1, l_graphics_isParticleSystem);
  l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);

  float xmin, ymin, xmax, ymax;
  graphics_ParticleSystem_getLinearAcceleration(&ps->particleSystem, &xmin, &ymin, &xmax, &ymax);
  lua_pushnumber(state, xmin);
  lua_pushnumber(state, ymin);
  lua_pushnumber(state, xmax);
  lua_pushnumber(state, ymax);

  return 4;
}


#define makeSingleBoolGetter(name) \
  static int l_graphics_ParticleSystem_ ## name(lua_State *state) { \
    l_assertType(state, 1, l_graphics_isParticleSystem);          \
    l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);  \
    lua_pushboolean(state, graphics_ParticleSystem_ ## name(&ps->particleSystem)); \
    return 1;   \
  }

#define makeSingleNumParamSetter(name, type) \
  static int l_graphics_ParticleSystem_ ## name(lua_State *state) { \
    l_assertType(state, 1, l_graphics_isParticleSystem);          \
    l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);  \
    type v = l_tools_toNumberOrError(state, 2);   \
    graphics_ParticleSystem_ ## name(&ps->particleSystem, v); \
    return 0;   \
  }
    

#define makeSingleNumParamGetter(name, type) \
  static int l_graphics_ParticleSystem_ ## name(lua_State *state) { \
    l_assertType(state, 1, l_graphics_isParticleSystem);          \
    l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);  \
    lua_pushnumber(state, graphics_ParticleSystem_ ## name(&ps->particleSystem)); \
    return 1;   \
  }


#define makeTwoNumParamSetter(name, type, opt) \
  static int l_graphics_ParticleSystem_ ## name(lua_State *state) { \
    l_assertType(state, 1, l_graphics_isParticleSystem);          \
    l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);  \
    type v1 = l_tools_toNumberOrError(state, 2);   \
    type v2; \
    if(opt) { \
      v2 = l_tools_toNumberOrError(state, 3);   \
    } else { \
      v2 = luaL_optnumber(state, 3, v1); \
    } \
    graphics_ParticleSystem_ ## name(&ps->particleSystem, v1, v2); \
    return 0;   \
  }
    

#define makeTwoNumParamGetter(name, type) \
  static int l_graphics_ParticleSystem_ ## name(lua_State *state) { \
    l_assertType(state, 1, l_graphics_isParticleSystem);          \
    l_graphics_ParticleSystem const *ps = l_graphics_toParticleSystem(state, 1);  \
    type v1, v2; \
    graphics_ParticleSystem_ ## name(&ps->particleSystem, &v1, &v2); \
    lua_pushnumber(state, v1); \
    lua_pushnumber(state, v2); \
    return 2;   \
  }

#define makeNoParamFunc(name) \
  static int l_graphics_ParticleSystem_ ## name(lua_State *state) { \
    l_assertType(state, 1, l_graphics_isParticleSystem);          \
    l_graphics_ParticleSystem *ps = l_graphics_toParticleSystem(state, 1);  \
    graphics_ParticleSystem_ ## name(&ps->particleSystem); \
    return 0; \
  }
    


makeNoParamFunc(pause)
makeNoParamFunc(reset)
makeNoParamFunc(start)
makeNoParamFunc(stop)

makeSingleBoolGetter(isActive)
makeSingleBoolGetter(isPaused)
makeSingleBoolGetter(isStopped)
makeSingleBoolGetter(hasRelativeRotation)

makeSingleNumParamSetter(update,                    float)
makeSingleNumParamSetter(emit,                      size_t)
makeSingleNumParamSetter(setBufferSize,             float)
makeSingleNumParamSetter(setDirection,              float)
makeSingleNumParamSetter(setEmissionRate,           float)
makeSingleNumParamSetter(setEmitterLifetime,        float)
makeSingleNumParamSetter(setSizeVariation,          float)
makeSingleNumParamSetter(setSpinVariation,          float)
makeSingleNumParamSetter(setSpread,                 float)

makeSingleNumParamGetter(getBufferSize,             float)
makeSingleNumParamGetter(getDirection,              float)
makeSingleNumParamGetter(getEmissionRate,           float)
makeSingleNumParamGetter(getEmitterLifetime,        float)
makeSingleNumParamGetter(getSizeVariation,          float)
makeSingleNumParamGetter(getSpinVariation,          float)
makeSingleNumParamGetter(getSpread,                 float)
makeSingleNumParamGetter(getCount,                  size_t)

makeTwoNumParamSetter(moveTo,                       float, false)
makeTwoNumParamSetter(setOffset,                    float, false)
makeTwoNumParamSetter(setParticleLifetime,          float, true)
makeTwoNumParamSetter(setPosition,                  float, true)
makeTwoNumParamSetter(setRadialAcceleration,        float, true)
makeTwoNumParamSetter(setRotation,                  float, true)
makeTwoNumParamSetter(setSpeed,                     float, true)
makeTwoNumParamSetter(setSpin,                      float, true)
makeTwoNumParamSetter(setTangentialAcceleration,    float, true)
makeTwoNumParamSetter(setLinearDamping,             float, true)

makeTwoNumParamGetter(getOffset,                    float)
makeTwoNumParamGetter(getParticleLifetime,          float)
makeTwoNumParamGetter(getPosition,                  float)
makeTwoNumParamGetter(getRadialAcceleration,        float)
makeTwoNumParamGetter(getRotation,                  float)
makeTwoNumParamGetter(getSpeed,                     float)
makeTwoNumParamGetter(getSpin,                      float)
makeTwoNumParamGetter(getTangentialAcceleration,    float)
makeTwoNumParamGetter(getLinearDamping,             float)


l_checkTypeFn(l_graphics_isParticleSystem, moduleData.particleSystemMT)
l_toTypeFn(l_graphics_toParticleSystem, l_graphics_ParticleSystem)

static luaL_Reg const particleSystemMetatableFuncs[] = {
  // Handwritten
  {"__gc",          			      l_graphics_gcParticleSystem},
  {"setAreaSpread", 			      l_graphics_ParticleSystem_setAreaSpread},
  {"setColors",                 l_graphics_ParticleSystem_setColors},
  {"setTexture",                l_graphics_ParticleSystem_setTexture},
  {"setSizes",                  l_graphics_ParticleSystem_setSizes},
  {"setQuads",                  l_graphics_ParticleSystem_setQuads},
  {"setRelativeRotation",       l_graphics_ParticleSystem_setRelativeRotation},
  {"setInsertMode",             l_graphics_ParticleSystem_setInsertMode},
  {"setLinearAcceleration",     l_graphics_ParticleSystem_setLinearAcceleration},

  // Single Num Param
  {"update",                    l_graphics_ParticleSystem_update},
  {"emit",                      l_graphics_ParticleSystem_emit},
  {"setBufferSize", 			      l_graphics_ParticleSystem_setBufferSize},
  {"setDirection",              l_graphics_ParticleSystem_setDirection},
  {"setEmissionRate",           l_graphics_ParticleSystem_setEmissionRate},
  {"setEmitterLifetime",        l_graphics_ParticleSystem_setEmitterLifetime},
  {"setSizeVariation",          l_graphics_ParticleSystem_setSizeVariation},
  {"setSpinVariation",          l_graphics_ParticleSystem_setSpinVariation},
  {"setSpread",                 l_graphics_ParticleSystem_setSpread},

  // Two Num Param
  {"moveTo",                    l_graphics_ParticleSystem_moveTo},
  {"setOffset",                 l_graphics_ParticleSystem_setOffset},
  {"setParticleLifetime",       l_graphics_ParticleSystem_setParticleLifetime},
  {"setPosition",               l_graphics_ParticleSystem_setPosition},
  {"setRadialAcceleration",     l_graphics_ParticleSystem_setRadialAcceleration},
  {"setRotation",               l_graphics_ParticleSystem_setRotation},
  {"setSpeed",                  l_graphics_ParticleSystem_setSpeed},
  {"setSpin",                   l_graphics_ParticleSystem_setSpin},
  {"setTangentialAcceleration", l_graphics_ParticleSystem_setTangentialAcceleration},
  {"setLinearDamping",          l_graphics_ParticleSystem_setLinearDamping},

  // Handwritten
  {"getAreaSpread", 			      l_graphics_ParticleSystem_getAreaSpread},
  {"getColors",     			      l_graphics_ParticleSystem_getColors},
  {"getTexture",                l_graphics_ParticleSystem_getTexture},
  {"getSizes",                  l_graphics_ParticleSystem_getSizes},
  {"getInsertMode",             l_graphics_ParticleSystem_getInsertMode},
  {"getLinearAcceleration",     l_graphics_ParticleSystem_getLinearAcceleration},

  // Single Num Param
  {"getBufferSize", 			      l_graphics_ParticleSystem_getBufferSize},
  {"getDirection",              l_graphics_ParticleSystem_getDirection},
  {"getEmissionRate",           l_graphics_ParticleSystem_getEmissionRate},
  {"getEmitterLifetime",        l_graphics_ParticleSystem_getEmitterLifetime},
  {"getSizeVariation",          l_graphics_ParticleSystem_getSizeVariation},
  {"getSpinVariation",          l_graphics_ParticleSystem_getSpinVariation},
  {"getSpread",                 l_graphics_ParticleSystem_getSpread},
  {"getCount",                  l_graphics_ParticleSystem_getCount},

  // Two Num Param
  {"getOffset",                 l_graphics_ParticleSystem_getOffset},
  {"getParticleLifetime",       l_graphics_ParticleSystem_getParticleLifetime},
  {"getPosition",               l_graphics_ParticleSystem_getPosition},
  {"getRadialAcceleration",     l_graphics_ParticleSystem_getRadialAcceleration},
  {"getRotation",               l_graphics_ParticleSystem_getRotation},
  {"getSpeed",                  l_graphics_ParticleSystem_getSpeed},
  {"getSpin",                   l_graphics_ParticleSystem_getSpin},
  {"getTangentialAcceleration", l_graphics_ParticleSystem_getTangentialAcceleration},
  {"getLinearDamping",          l_graphics_ParticleSystem_getLinearDamping},

  // Booleans
  {"isActive",                  l_graphics_ParticleSystem_isActive},
  {"isPaused",                  l_graphics_ParticleSystem_isPaused},
  {"isStopped",                 l_graphics_ParticleSystem_isStopped},
  {"hasRelativeRotation",       l_graphics_ParticleSystem_hasRelativeRotation},

  // No-Param-Funcs
  {"pause",                     l_graphics_ParticleSystem_pause},
  {"reset",                     l_graphics_ParticleSystem_reset},
  {"start",                     l_graphics_ParticleSystem_start},
  {"stop",                      l_graphics_ParticleSystem_stop},
  {"clone",                     l_graphics_ParticleSystem_clone},

  {NULL, NULL}
};

static luaL_Reg const particleSystemFreeFuncs[] = {
  {"newParticleSystem",         l_graphics_newParticleSystem},
  {NULL, NULL}
};


void l_graphics_particlesystem_register(lua_State* state) {
  l_tools_registerFuncsInModule(state, "graphics", particleSystemFreeFuncs);
  moduleData.particleSystemMT = l_tools_makeTypeMetatable(state, particleSystemMetatableFuncs);
}
