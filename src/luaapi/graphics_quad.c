/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "graphics_quad.h"
#include "tools.h"

static struct {
  int quadMT;
} moduleData;

int l_graphics_newQuad(lua_State* state) {
  float x = l_tools_toNumberOrError(state, 1);
  float y = l_tools_toNumberOrError(state, 2);
  float w = l_tools_toNumberOrError(state, 3);
  float h = l_tools_toNumberOrError(state, 4);
  float rw = l_tools_toNumberOrError(state, 5);
  float rh = l_tools_toNumberOrError(state, 6);

  graphics_Quad* obj = lua_newuserdata(state, sizeof(graphics_Quad));

  graphics_Quad_newWithRef(obj, x, y, w, h, rw, rh);

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.quadMT);
  lua_setmetatable(state, -2);

  return 1;
}

static int l_graphics_Quad_getViewport(lua_State* state) {
  l_assertType(state, 1, l_graphics_isQuad);

  graphics_Quad *quad = l_graphics_toQuad(state, 1);
  lua_pushnumber(state, quad->x);
  lua_pushnumber(state, quad->y);
  lua_pushnumber(state, quad->w);
  lua_pushnumber(state, quad->h);

  return 4;
}

static int l_graphics_Quad_setViewport(lua_State* state) {
  l_assertType(state, 1, l_graphics_isQuad);

  graphics_Quad *quad = l_graphics_toQuad(state, 1);
  
  float x = l_tools_toNumberOrError(state, 2);
  float y = l_tools_toNumberOrError(state, 3);
  float w = l_tools_toNumberOrError(state, 4);
  float h = l_tools_toNumberOrError(state, 5);

  quad->x = x;
  quad->y = y;
  quad->w = w;
  quad->h = h;

  return 0;
}

l_checkTypeFn(l_graphics_isQuad, moduleData.quadMT)
l_toTypeFn(l_graphics_toQuad, graphics_Quad)

static luaL_Reg const quadMetatableFuncs[] = {
  {"getViewport",        l_graphics_Quad_getViewport},
  {"setViewport",        l_graphics_Quad_setViewport},
  {NULL, NULL}
};

static luaL_Reg const quadFreeFuncs[] = {
  {"newQuad",            l_graphics_newQuad},
  {NULL, NULL}
};


void l_graphics_quad_register(lua_State* state) {
  l_tools_registerFuncsInModule(state, "graphics", quadFreeFuncs);
  moduleData.quadMT   = l_tools_makeTypeMetatable(state, quadMetatableFuncs);
}
