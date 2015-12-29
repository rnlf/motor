/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "tools.h"
#include "event.h"

static int l_event_quit(lua_State* state) {
  printf("love.event.quit is a stub\n");
  return 0;
}


static luaL_Reg const regFuncs[] = {
  {"quit", l_event_quit},
  {NULL, NULL}
};


int l_event_register(lua_State* state) {
  l_tools_registerModule(state, "event", regFuncs);

  return 0;
}
