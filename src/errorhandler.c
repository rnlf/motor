/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#ifdef EMSCRIPTEN
# include <emscripten.h>
#endif
#include <lua.h>
#include <stdio.h>
#include "graphics/graphics.h"
#include "graphics/font.h"

int errorhandler(lua_State *state) {
  // TODO
  //const char* msg = lua_tostring(state, 1);
  lua_Debug debug;
  int level = 0;
  int count = 0;
  lua_pushstring(state, "\n\nStack trace:\n");
  while(lua_getstack(state, level, &debug)) {
    lua_getinfo(state, "Sl", &debug);
    if(strcmp(debug.what, "C")) {
      lua_pushstring(state, debug.short_src);
      lua_pushstring(state, ":");
      lua_pushnumber(state, debug.currentline);
      lua_pushstring(state, "\n");
      ++count;
    }
    ++level;
  }
  lua_concat(state, 4*count+2);
  return 1;
}


// Expects errorhandler at index 1
void pcall(lua_State *state, int nargs) {
  if(lua_pcall(state, nargs, 0, 1)) {
    char const *msg = lua_tostring(state, -1);
    printf("Lua error: %s\n", msg);
    graphics_Font font;
    graphics_setBackgroundColor(0.64f, 0.27f, 0.26f, 1.0f);
    graphics_clear();
    graphics_setColor(1.0f, 1.0f, 1.0f, 0.8f);
    graphics_Font_new(&font, 0, 12);
    graphics_Font_render(&font, msg, 10, 40, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f); 
    graphics_swap();

    #ifdef EMSCRIPTEN
      emscripten_force_exit(1);
    #else
      exit(1);
    #endif
  }
}
