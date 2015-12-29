/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <lauxlib.h>
#include "../graphics/graphics.h"
#include "graphics_window.h"
#include "tools.h"


static int l_graphics_window_getWidth(lua_State* state) {
  lua_pushnumber(state, graphics_getWidth());
  return 1;
}


static int l_graphics_window_getHeight(lua_State* state) {
  lua_pushnumber(state, graphics_getHeight());
  return 1;
}


static int l_graphics_window_getDimensions(lua_State* state) {
  lua_pushnumber(state, graphics_getWidth());
  lua_pushnumber(state, graphics_getHeight());
  return 2;
}


static int l_graphics_window_getMode(lua_State* state) {
  lua_pushnumber(state, graphics_getWidth());
  lua_pushnumber(state, graphics_getHeight());

  lua_newtable(state);

  lua_pushstring(state, "fullscreen");
  lua_pushboolean(state, false);
  lua_settable(state, 3);

  lua_pushstring(state, "fullscreentype");
  lua_pushstring(state, "normal");
  lua_settable(state, 3);

  lua_pushstring(state, "vsync");
  lua_pushboolean(state, true);
  lua_settable(state, 3);

  lua_pushstring(state, "fsaa");
  lua_pushnumber(state, 0);
  lua_settable(state, 3);

  lua_pushstring(state, "resizable");
  lua_pushboolean(state, false);
  lua_settable(state, 3);

  lua_pushstring(state, "centered");
  lua_pushboolean(state, false);
  lua_settable(state, 3);

  lua_pushstring(state, "highdpi");
  lua_pushboolean(state, false);
  lua_settable(state, 3);

  lua_pushstring(state, "srgb");
  lua_pushboolean(state, false);
  lua_settable(state, 3);
  
  lua_pushstring(state, "refreshrate");
  lua_pushnumber(state, 0);
  lua_settable(state, 3);

  lua_pushstring(state, "x");
  lua_pushnumber(state, 0);
  lua_settable(state, 3);

  lua_pushstring(state, "y");
  lua_pushnumber(state, 0);
  lua_settable(state, 3);

  return 3;
}


static luaL_Reg const windowFreeFuncs[] = {
  {"getWidth",      l_graphics_window_getWidth},
  {"getHeight",     l_graphics_window_getHeight},
  {"getDimensions", l_graphics_window_getDimensions},
  {"getMode",       l_graphics_window_getMode},
  {NULL, NULL}
};

void l_graphics_window_register(lua_State* state) {
  l_tools_registerModule(state, "window", windowFreeFuncs);
}

