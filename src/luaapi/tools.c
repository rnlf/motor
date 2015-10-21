#include <stdlib.h>
#include <lauxlib.h>
#include <string.h>
#include "tools.h"


static struct {
  float * numbers;
  int numberSize;
} moduleData;



void l_tools_registerFuncsInModule(lua_State* state, char const* module, luaL_Reg const* funcs) {
  lua_getglobal(state, "love");
  lua_pushstring(state, module);
  lua_gettable(state, -2);
  //luaL_setfuncs(state, funcs, 0);
  luaL_register(state, NULL, funcs);
  lua_pop(state, 2);
}

void l_tools_registerModule(lua_State* state, char const* moduleName, luaL_Reg const * funcs) {
  lua_getglobal(state, "love");
  lua_pushstring(state, moduleName);
  //luaL_newlib(state, funcs);
  lua_newtable(state);
  luaL_register(state, NULL, funcs);
  lua_rawset(state, -3);
  lua_pop(state, 1);
}


int l_tools_makeTypeMetatable(lua_State* state, luaL_Reg const* funcs) {
  int mtref;
//  luaL_newlib(state, funcs);
  lua_newtable(state);
  luaL_register(state, NULL, funcs);
  lua_pushvalue(state, -1);
  mtref = luaL_ref(state, LUA_REGISTRYINDEX);
  lua_pushstring(state, "type");
  lua_pushinteger(state, mtref);
  lua_rawset(state, -3);
  lua_pushstring(state, "__index");
  lua_pushvalue(state, -2);
  lua_rawset(state, -3);

  lua_pop(state, 1);

  return mtref;
}


void l_tools_pushEnum(lua_State* state, int value, l_tools_Enum const* values) {
  while(values->name) {
    if(values->value == value) {
      lua_pushstring(state, values->name);
      return;
    }
    ++values;
  }

  // C code has to make sure the enum value is valid!
}


int l_tools_readNumbers(lua_State* state, int offset, float **numbers, int minNums, int components) {
  bool table = lua_istable(state, 1 + offset);

  int count;
  if(table) {
    count = lua_objlen(state, 1 + offset);
  } else {
    count = lua_gettop(state) - offset;
  }

  if(count % components) {
    lua_pushstring(state, "Need even number of values");
    return lua_error(state);
  }

  if(count < minNums) {
    lua_pushstring(state, "Need at least ");
    lua_pushnumber(state, minNums);
    lua_pushstring(state, " numbers");
    lua_concat(state, 3);
    return lua_error(state);
  }

  if(count > moduleData.numberSize) {
    moduleData.numbers = realloc(moduleData.numbers, count * sizeof(float));
    moduleData.numberSize = count;
  }

  if(table) {
    for(int i = 0; i < count; ++i) {
      lua_rawgeti(state, 1+offset, i+1);
      moduleData.numbers[i] = l_tools_toNumberOrError(state, -1);
      lua_pop(state, 1);
    }
  } else {
    for(int i = 0; i < count; ++i) {
      moduleData.numbers[i] = l_tools_toNumberOrError(state, 1 + i + offset);
    }
  }

  *numbers = moduleData.numbers;

  return count / components;
}


extern inline int l_tools_toBooleanOrError(lua_State* state, int index);
extern inline float l_tools_toNumberOrError(lua_State* state, int index);
extern inline char const* l_tools_toStringOrError(lua_State* state, int index);
extern inline int l_tools_toEnumOrError(lua_State* state, int index, l_tools_Enum const* values);
