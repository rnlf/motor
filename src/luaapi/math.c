#include "polygon.h"
#include "tools.h"
#include "math.h"
#include "../math/randomgenerator.h"
#include "../math/triangulate.h"
#include "../math/minmax.h"
#include "../math/gamma.h"
#include "../3rdparty/noise1234/simplexnoise1234.h"
#include <math.h>
#include <inttypes.h>

static struct {
  math_RandomGenerator defaultRandomGenerator;
  int randomGeneratorMT;
  int polygonDatSize;
  float *polygonData;

} moduleData;

l_checkTypeFn(l_math_isRandomGenerator, moduleData.randomGeneratorMT)
l_toTypeFn(l_math_toRandomGenerator, math_RandomGenerator)

static int l_math_random(lua_State* state, int offset, math_RandomGenerator *randomGenerator) {
  float lo = 0.0f, hi = 1.0f;
  int t = lua_gettop(state) - offset;
  switch(t) {
  case 1:
    hi = l_tools_toNumberOrError(state, 1+offset);
    break;

  case 2:
    lo = l_tools_toNumberOrError(state, 1+offset);
    hi = l_tools_toNumberOrError(state, 2+offset);
    break;
  }

  float number = math_RandomGenerator_random2(randomGenerator, lo, hi);
  if(t > 0) {
    number = floor(number);
  }

  lua_pushnumber(state, number);
  return 1;
}


static int l_math_random_free(lua_State* state) {
  return l_math_random(state, 0, &moduleData.defaultRandomGenerator);
}


static int l_math_RandomGenerator_random(lua_State* state) {
  l_assertType(state, 1, l_math_isRandomGenerator);
  math_RandomGenerator *generator = l_math_toRandomGenerator(state, 1);
  return l_math_random(state, 1, generator);
}


static int l_math_randomNormal(lua_State* state, int offset, math_RandomGenerator *randomGenerator) {

  float stddev = luaL_optnumber(state, 1 + offset, 1.0f);
  float mean = luaL_optnumber(state, 2 + offset, 0.0f);

  float number = math_RandomGenerator_randomNormal2(randomGenerator, stddev, mean);

  lua_pushnumber(state, number);
  return 1;
}

static int l_math_randomNormal_free(lua_State* state) {
  return l_math_randomNormal(state, 0, &moduleData.defaultRandomGenerator);
}


static int l_math_RandomGenerator_randomNormal(lua_State* state) {
  l_assertType(state, 1, l_math_isRandomGenerator);
  math_RandomGenerator *generator = l_math_toRandomGenerator(state, 1);
  return l_math_randomNormal(state, 1, generator);
}




static int l_math_setRandomSeed_free(lua_State* state) {
  uint64_t seed = 0;
  switch(lua_gettop(state)) {
  case 2:
    seed = (uint64_t)floor(l_tools_toNumberOrError(state, 2)) << 32;
  case 1:
    seed |= (uint64_t)floor(l_tools_toNumberOrError(state, 1));
    math_RandomGenerator_setSeed(&moduleData.defaultRandomGenerator, seed);
    break;
  default:
    lua_pushstring(state, "Expected low and optionally high word for seed");
    return lua_error(state);
  }
  return 0;
}

static int l_math_RandomGenerator_setRandomSeed(lua_State* state) {
  l_assertType(state, 1, l_math_isRandomGenerator);
  math_RandomGenerator *generator = l_math_toRandomGenerator(state, 1);
  uint64_t seed = 0;
  switch(lua_gettop(state)) {
  case 3:
    seed = (uint64_t)floor(l_tools_toNumberOrError(state, 3)) << 32;
  case 2:
    seed |= (uint64_t)floor(l_tools_toNumberOrError(state, 2));
    math_RandomGenerator_setSeed(generator, seed);
    break;
  default:
    lua_pushstring(state, "Expected low and optionally high word for seed");
    return lua_error(state);
  }
  return 0;
}


static int l_math_newRandomGenerator(lua_State* state) {
  math_RandomGenerator *generator = lua_newuserdata(state, sizeof(math_RandomGenerator));
  math_RandomGenerator_init(generator);

  uint64_t seed = 0;
  switch(lua_gettop(state)) {
  case 3:
    seed = (uint64_t)floor(l_tools_toNumberOrError(state, 2)) << 32;
  case 2:
    seed |= (uint64_t)floor(l_tools_toNumberOrError(state, 1));
    math_RandomGenerator_setSeed(generator, seed);
    break;
  default:
    break;
  }

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.randomGeneratorMT);
  lua_setmetatable(state, -2);
 
  return 1;
}

static int l_math_getRandomSeed(lua_State* state) {
  math_RandomGenerator* generator = &moduleData.defaultRandomGenerator;
  if(l_math_isRandomGenerator(state, 1)) {
    generator = l_math_toRandomGenerator(state, 1);
  }

  uint64_t seed = math_RandomGenerator_getSeed(generator);
  lua_pushnumber(state, seed & 0xFFFFFFFF);
  lua_pushnumber(state, (seed >> 32) & 0xFFFFFFFF);
  return 2;
}


static int l_math_RandomGenerator_getState(lua_State* state) {
  l_assertType(state, 1, l_math_isRandomGenerator);
  math_RandomGenerator const *generator = l_math_toRandomGenerator(state, 1);

  uint64_t rgState = math_RandomGenerator_getState(generator);

  char buf[19];
  sprintf(buf, "0x%" PRIx64, rgState);
  lua_pushstring(state, buf);
  return 1;
}


static int l_math_RandomGenerator_setState(lua_State* state) {
  l_assertType(state, 1, l_math_isRandomGenerator);
  math_RandomGenerator *generator = l_math_toRandomGenerator(state, 1);

  char const * str = l_tools_toStringOrError(state, 2);
  uint64_t rgState;
  sscanf(str, "0x%" PRIx64, &rgState);

  math_RandomGenerator_setState(generator, rgState);

  return 0;
}


static int l_math_triangulate(lua_State* state) {
  float *vertices;
  int count = l_geometry_read_vertices(state, 0, &vertices, 6);
  int *indices;
  int triangles = math_triangulation_triangulate(vertices, count, &indices);

  lua_createtable(state, triangles, 0);
  for(int i = 0; i < triangles; ++i) {
    lua_createtable(state, 3, 0);
    for(int j = 0; j < 3; ++j) {
      int vIdx = indices[3*i+j];
      lua_pushnumber(state, vertices[2*vIdx]);
      lua_rawseti(state, -2, 2*j+1);
      lua_pushnumber(state, vertices[2*vIdx+1]);
      lua_rawseti(state, -2, 2*j+2);
    }
    lua_rawseti(state, -2, i+1);
  }
  return 1;
}


static int l_math_isConvex(lua_State* state) {
  float *vertices;
  int count = l_geometry_read_vertices(state, 0, &vertices, 6);

  lua_pushboolean(state, math_isConvex(vertices, count));
  return 1;
}


static int l_math_noise(lua_State* state) {
  float a[4];
  float r;

  int t = min(lua_gettop(state), 4);

  for(int i = 0; i < t; ++i) {
    a[i] = l_tools_toNumberOrError(state, i+1);
  }

  switch(t) {
  case 1:
    r = simplexnoise1234_noise1(a[0]);
    break;

  case 2:
    r = simplexnoise1234_noise2(a[0], a[1]);
    break;

  case 3:
    r = simplexnoise1234_noise3(a[0], a[1], a[2]);
    break;

  case 4:
    r = simplexnoise1234_noise4(a[0], a[1], a[2], a[3]);
    break;

  default:
    lua_pushstring(state, "need at least one dimension");
    return lua_error(state);
  }
  
  lua_pushnumber(state, r);
  return 1;
}


static int l_math_linearGammaConvert(lua_State* state, float (*convert)(float)) {
  float *components;
  int numbers = l_tools_readNumbers(state, 0, &components, 1, 1);
  int i;
  for(i = 0; i < numbers && i < 4; ++i) {
    lua_pushnumber(state, convert(components[i]));
  }

  return i;
}


static int l_math_linearToGamma(lua_State* state) {
  return l_math_linearGammaConvert(state, math_linearToGamma);
}


static int l_math_gammaToLinear(lua_State* state) {
  return l_math_linearGammaConvert(state, math_gammaToLinear);
}



static luaL_Reg const mathFreeFuncs[] = {
  {"random",             l_math_random_free},
  {"randomNormal",       l_math_randomNormal_free},
  {"newRandomGenerator", l_math_newRandomGenerator},
  {"setRandomSeed",      l_math_setRandomSeed_free},
  {"getRandomSeed",      l_math_getRandomSeed},
  {"triangulate",        l_math_triangulate},
  {"noise",              l_math_noise},
  {"isConvex",           l_math_isConvex},
  {"linearToGamma",      l_math_linearToGamma},
  {"gammaToLinear",      l_math_gammaToLinear},
  {NULL, NULL}
};


static luaL_Reg const randomGeneratorMetatableFuncs[] = {
  {"random",       l_math_RandomGenerator_random},
  {"randomNormal", l_math_RandomGenerator_randomNormal},
  {"setSeed",      l_math_RandomGenerator_setRandomSeed},
  {"getSeed",      l_math_getRandomSeed},
  {"getState",     l_math_RandomGenerator_getState},
  {"setState",     l_math_RandomGenerator_setState},
  {NULL, NULL}
};

void l_math_register(lua_State* state) {
  math_RandomGenerator_init(&moduleData.defaultRandomGenerator);

  moduleData.randomGeneratorMT = l_tools_makeTypeMetatable(state, randomGeneratorMetatableFuncs);
  l_tools_registerModule(state, "math", mathFreeFuncs);

}
