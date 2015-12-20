#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <stdbool.h>
#include <string.h>

void l_tools_registerFuncsInModule(lua_State* state, char const* module, luaL_Reg const* funcs);
void l_tools_registerModule(lua_State* state, char const* moduleName, luaL_Reg const * funcs);
int l_tools_makeTypeMetatable(lua_State* state, luaL_Reg const* funcs);


#ifndef MOTOR_SKIP_SAFETY_CHECKS
inline float l_tools_toNumberOrError(lua_State* state, int index) {
  if(lua_type(state, index) != LUA_TNUMBER) {
    lua_pushstring(state, "expected number");
    lua_error(state);
  }

  return lua_tonumber(state, index);
}

inline char const* l_tools_toStringOrError(lua_State* state, int index) {
  if(lua_type(state, index) != LUA_TSTRING) {
    lua_pushstring(state, "expected string");
    lua_error(state);
  }

  return lua_tostring(state, index);
}

inline int l_tools_toBooleanOrError(lua_State* state, int index) {
  if(lua_type(state, index) != LUA_TBOOLEAN) {
    lua_pushstring(state, "expected boolean");
    lua_error(state);
  }

  return lua_toboolean(state, index);
}
#else
inline float l_tools_toNumberOrError(lua_State* state, int index) {
  return lua_tonumber(state, index);
}

inline char const* l_tools_toStringOrError(lua_State* state, int index) {
  return lua_tostring(state, index);
}

inline int l_tools_toBooleanOrError(lua_State* state, int index) {
  return lua_toboolean(state, index);
}
#endif

typedef struct {
  char const * name;
  int value;
} l_tools_Enum;

inline int l_tools_toEnumOrError(lua_State* state, int index, l_tools_Enum const* values) {
  char const* string = l_tools_toStringOrError(state, index);

  while(values->name) {
    if(!strcmp(values->name, string)) {
      return values->value;
    }
    ++values;
  }

  lua_pushstring(state, "invalid enum value");
  return lua_error(state);
}

void l_tools_pushEnum(lua_State* state, int value, l_tools_Enum const* values);

#define l_checkTypeFn(name, typeMT) \
  bool name(lua_State* state, int index) { \
    if(lua_type(state, index) != LUA_TUSERDATA) \
      return 0;                                 \
    lua_getmetatable(state, index);             \
    lua_pushstring(state, "type");              \
    lua_rawget(state, -2);                      \
    bool matching = lua_tointeger(state, -1) == typeMT; \
    lua_pop(state, 2);                          \
    return matching;                          \
  }

#define l_toTypeFn(name, type) \
  type* name(lua_State* state, int index) {\
    return (type*)lua_touserdata(state, index);\
  }

#ifndef MOTOR_SKIP_SAFETY_CHECKS
// TODO appropriate name
# define l_assertType(state, index, func) \
    if(!func(state, index)) { \
     lua_pushstring(state, "expected X"); \
     lua_error(state); \
    }
#else
# define l_assertType(state, index, func)
#endif

int l_tools_readNumbers(lua_State* state, int offset, float **numbers, int minNums, int components);

#define l_tools_stub(name, fname)               \
  int fname(lua_State* state) {                 \
    (void)state;                                \
    static bool warned = false;                 \
    if(!warned) {                               \
      printf("WARNING: %s is a stub.\n", name); \
      warned = true;                            \
    }                                           \
    return 0;                                   \
  }


