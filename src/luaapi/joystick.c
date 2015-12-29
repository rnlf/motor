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


static int l_joystick_Joystick_isGamepad(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);

  lua_pushboolean(state, joystick_Joystick_isGamepad(joystick->joystick));
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


static int l_joystick_Joystick_getButtonCount(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);

  lua_pushinteger(state, joystick_Joystick_getButtonCount(joystick->joystick));
  return 1;
}


static int l_joystick_Joystick_isDown(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);
  int count = lua_gettop(state) - 1;
  for(int i = 0; i < count; ++i) {
    int button = l_tools_toNumberOrError(state, i+2);
    lua_pushboolean(state, joystick_Joystick_isDown(joystick->joystick, button - 1));
  }
  return count;
}


static int l_joystick_Joystick_getGUID(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);
  
  joystick_GUID guid = joystick_Joystick_getGUID(joystick->joystick);
  lua_pushstring(state, guid.guid);

  return 1;
}


static int l_joystick_Joystick_getHatCount(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);
  lua_pushinteger(state, joystick_Joystick_getHatCount(joystick->joystick));

  return 1;
}


static const l_tools_Enum l_joystick_JoystickHat[] = {
  {"c",    joystick_JoystickHat_centered},
  {"d",    joystick_JoystickHat_down},
  {"l",    joystick_JoystickHat_left},
  {"ld",   joystick_JoystickHat_left_down},
  {"lu",   joystick_JoystickHat_left_up},
  {"r",    joystick_JoystickHat_right},
  {"rd",   joystick_JoystickHat_right_down},
  {"ru",   joystick_JoystickHat_right_up},
  {"u",    joystick_JoystickHat_up},
  {NULL, 0}
};


static int l_joystick_Joystick_getHat(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);

  int hat = l_tools_toNumberOrError(state, 2) - 1;

  joystick_JoystickHat dir = joystick_Joystick_getHat(joystick->joystick, hat);

  l_tools_pushEnum(state, dir, l_joystick_JoystickHat);

  return 1;
}


static int l_joystick_Joystick_getName(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);
  
  lua_pushstring(state, joystick_Joystick_getName(joystick->joystick));

  return 1;
}


static int l_joystick_Joystick_getID(lua_State *state) {
  l_joystick_Joystick *joystick = l_joystick_toJoystick(state, 1);
  
  lua_pushinteger(state, joystick_Joystick_getID(joystick->joystick));
  lua_pushinteger(state, joystick_Joystick_getInstanceID(joystick->joystick));

  return 2;
}


static luaL_Reg const joystickMetatableFuncs[] = {
  {"isConnected",    l_joystick_Joystick_isConnected},
  {"isGamepad",      l_joystick_Joystick_isGamepad},
  {"getAxis",        l_joystick_Joystick_getAxis},
  {"getAxes",        l_joystick_Joystick_getAxes},
  {"getAxisCount",   l_joystick_Joystick_getAxisCount},
  {"getButtonCount", l_joystick_Joystick_getButtonCount},
  {"isDown",         l_joystick_Joystick_isDown},
  {"getGUID",        l_joystick_Joystick_getGUID},
  {"getHatCount",    l_joystick_Joystick_getHatCount},
  {"getHat",         l_joystick_Joystick_getHat},
  {"getName",        l_joystick_Joystick_getName},
  {"getID",          l_joystick_Joystick_getID},
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


// Push the Lua object corresponding to the given device onto the stack
static void l_joystick_pushDevice(lua_State *state, joystick_Joystick *device) {
  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.joystickTableRef);

  int len = lua_objlen(state, -1);
  int i;
  for(i = 0; i < len; ++i) {
    lua_rawgeti(state, -1, i+1);
    l_joystick_Joystick *js = l_joystick_toJoystick(state, -1);

    if(js->joystick == device) {
      lua_replace(state, -2);
      return;
    }

    lua_pop(state, 1);
  }
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


static void l_joystick_joystickAxis(void *ud, joystick_Joystick *device, int axis, float value) {
  lua_State * state = (lua_State*)ud;
  int top = lua_gettop(state);
  lua_getglobal(state, "love");
  lua_pushstring(state, "joystickaxis");
  lua_rawget(state, -2);
  
  l_joystick_pushDevice(state, device);

  lua_pushinteger(state, axis + 1);
  lua_pushnumber(state, value);

  lua_call(state, 3, 0);
  lua_settop(state, top);
}


static void l_joystick_joystickPressed(void *ud, joystick_Joystick *device, int button) {
  lua_State * state = (lua_State*)ud;
  int top = lua_gettop(state);
  lua_getglobal(state, "love");
  lua_pushstring(state, "joystickpressed");
  lua_rawget(state, -2);
  
  l_joystick_pushDevice(state, device);

  lua_pushinteger(state, button + 1);

  lua_call(state, 2, 0);
  lua_settop(state, top);
}


static void l_joystick_joystickReleased(void *ud, joystick_Joystick *device, int button) {
  lua_State * state = (lua_State*)ud;
  int top = lua_gettop(state);
  lua_getglobal(state, "love");
  lua_pushstring(state, "joystickreleased");
  lua_rawget(state, -2);
  
  l_joystick_pushDevice(state, device);

  lua_pushinteger(state, button + 1);

  lua_call(state, 2, 0);
  lua_settop(state, top);
}


void l_joystick_register(lua_State* state) {
  l_tools_registerModule(state, "joystick", regFuncs);
  moduleData.joystickMT = l_tools_makeTypeMetatable(state, joystickMetatableFuncs);

  joystick_EventCallbacks callbacks = {
    .deviceAdded      = l_joystick_deviceAdded,
    .deviceRemoved    = l_joystick_deviceRemoved,
    .joystickAxis     = l_joystick_joystickAxis,
    .joystickPressed  = l_joystick_joystickPressed,
    .joystickReleased = l_joystick_joystickReleased,
    .userData         = state
  };

  joystick_setEventCallbacks(&callbacks);

  lua_createtable(state, 0, 0);
  moduleData.joystickTableRef = luaL_ref(state, LUA_REGISTRYINDEX);
}
