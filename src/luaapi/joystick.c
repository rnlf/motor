#include "joystick.h"
#include "tools.h"
#include <stdio.h>

static struct {
  int joystickMT;
  int joystickTableRef;
} moduleData;


l_checkTypeFn(l_joystick_isJoystick, moduleData.joystickMT)
l_toTypeFn(l_joystick_toJoystick, l_joystick_Joystick)


static int l_joysticks_getJoysticks(lua_State* state) {
  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.joystickTableRef);
  int len = lua_objlen(state, 1);
  lua_createtable(state, len, 0);
  for(int i = 0; i < len; ++i) {
    lua_rawgeti(state, 1, i+1);
    lua_rawseti(state, 2, i+1);
  }
  return 1;
}


static int l_joysticks_getJoystickCount(lua_State* state) {
  lua_pushnumber(state, joystick_getCount());
  return 1;
}


static luaL_Reg const regFuncs[] = {
  {"getJoysticks",     l_joysticks_getJoysticks},
  {"getJoystickCount", l_joysticks_getJoystickCount},
  {NULL, NULL}
};


static int l_joystick_Joystick_isConnected(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);

  lua_pushboolean(state, joystick_Joystick_isConnected(joystick->joystick));
  return 1;
}


static int l_joystick_Joystick_getAxis(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);
  int axis = l_tools_toNumberOrError(state, 2);

  float axisValue = joystick_Joystick_getAxis(joystick->joystick, axis - 1);
  lua_pushnumber(state, axisValue);

  return 1;
}


static int l_joystick_Joystick_getAxes(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);

  int count = joystick_Joystick_getAxisCount(joystick->joystick);

  for(int i = 0; i < count; ++i) {
    float axisValue = joystick_Joystick_getAxis(joystick->joystick, i);
    lua_pushnumber(state, axisValue);
  }

  return count;
}


static int l_joystick_Joystick_getAxisCount(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);

  lua_pushinteger(state, joystick_Joystick_getAxisCount(joystick->joystick));
  return 1;
}


static luaL_Reg const joystickMetatableFuncs[] = {
  {"isConnected",  l_joystick_Joystick_isConnected},
  {"getAxis",      l_joystick_Joystick_getAxis},
  {"getAxes",      l_joystick_Joystick_getAxes},
  {"getAxisCount", l_joystick_Joystick_getAxisCount},
  {NULL, NULL}
};


static void l_joystick_deviceAdded(void *ud, joystick_Joystick *device) {
  lua_State *state = (lua_State*)ud;

  lua_settop(state, 0);
  // st: jsTab
  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.joystickTableRef);
  
  int len = lua_objlen(state, 1);

  // st: jsTab js
  l_joystick_Joystick *js = lua_newuserdata(state, sizeof(l_joystick_Joystick));

  js->joystick = device;

  // st: jsTab js mt
  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.joystickMT);

  // st: jsTab js
  lua_setmetatable(state, 2);

  // st: jsTab
  lua_rawseti(state, 1, len + 1);

  printf("L: %d\n", lua_objlen(state, 1));
}


static void l_joystick_deviceRemoved(void *ud, joystick_Joystick *device) {
  lua_State *state = (lua_State*)ud;

  // st: jsTab
  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.joystickTableRef);
  
  int len = lua_objlen(state, -1);
  int i;
  for(i = 0; i < len; ++i) {
    lua_rawgeti(state, -1, i+1);
    l_joystick_Joystick *js = l_joystick_toJoystick(state, -1);

    if(js->joystick == device) {
      lua_pop(state, 1);
      break;
    }

    lua_pop(state, 1);
  }

  for(int j = i; j < len-1; ++j) {
    lua_rawgeti(state, 1, j+2);
    lua_rawseti(state, 1, j+1);
  }
  
  lua_pushnil(state);
  lua_rawseti(state, 1, len);

}


void l_joystick_register(lua_State* state) {
  l_tools_registerModule(state, "joystick", regFuncs);
  moduleData.joystickMT = l_tools_makeTypeMetatable(state, joystickMetatableFuncs);

  joystick_EventCallbacks callbacks = {
    .deviceAdded   = l_joystick_deviceAdded,
    .deviceRemoved = l_joystick_deviceRemoved,
    .userData      = state
  };

  joystick_setEventCallbacks(&callbacks);

  lua_createtable(state, 0, 0);
  moduleData.joystickTableRef = luaL_ref(state, LUA_REGISTRYINDEX);
}
