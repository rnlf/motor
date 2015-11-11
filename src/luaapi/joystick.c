#include "../joystick.h"
#include "tools.h"

static int l_joysticks_getJoysticks(lua_State* state) {
  lua_newtable(state);
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


void l_joystick_register(lua_State* state) {
  l_tools_registerModule(state, "joystick", regFuncs);
}
