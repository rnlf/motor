/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <lualib.h>
#include "motor.h"
#include "../motor.h"

int l_motor_getVersion(lua_State* state) {
  motor_Version const * version = motor_getVersion();
  lua_pushnumber(state, version->major);
  lua_pushnumber(state, version->minor);
  lua_pushnumber(state, version->revision);
  lua_pushstring(state, version->codename);
  lua_pushstring(state, "motor");
  return 5;
}

int l_motor_register(lua_State* state) {
  lua_newtable(state);

  lua_pushstring(state, "getVersion");
  lua_pushcfunction(state, l_motor_getVersion);
  lua_rawset(state, -3);
  
  lua_setglobal(state, "love");

  luaL_openlibs(state);

  return 0;
}
