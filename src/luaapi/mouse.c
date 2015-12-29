/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "mouse.h"
#include "../mouse.h"
#include "tools.h"
#include "image.h"

static struct {
  int cursorMT;
  int cursorRef;
} moduleData;

static int l_mouse_newCursor(lua_State *state) {
  int hotx = luaL_optinteger(state, 2, 0);
  int hoty = luaL_optinteger(state, 3, 0);
  
  mouse_Cursor *cursor = (mouse_Cursor*)lua_newuserdata(state, sizeof(mouse_Cursor));

  if(lua_isstring(state, 1)) {
    char const* filename = lua_tostring(state, 1);
    image_ImageData data;
    image_ImageData_new_with_filename(&data, filename);

    mouse_Cursor_new(cursor, &data, hotx, hoty);
    image_ImageData_free(&data);
  } else if(l_image_isImageData(state, 1)) {
    image_ImageData const *data = l_image_toImageData(state, 1);
    mouse_Cursor_new(cursor, data, hotx, hoty);
  }

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.cursorMT);
  lua_setmetatable(state, -2);

  return 1;
}


static int l_mouse_setCursor(lua_State *state) {
  if(lua_isuserdata(state, 1)) {
    mouse_Cursor *cursor = l_mouse_toCursor(state, 1);
    mouse_setCursor(cursor);
    luaL_unref(state, LUA_REGISTRYINDEX, moduleData.cursorRef);
    moduleData.cursorRef = luaL_ref(state, LUA_REGISTRYINDEX);
  } else {
    luaL_unref(state, LUA_REGISTRYINDEX, moduleData.cursorRef);
    moduleData.cursorRef = LUA_NOREF;
    mouse_setCursor(0);
  }
  return 0;
}


static int l_mouse_gcCursor(lua_State *state) {
  mouse_Cursor * cursor = l_mouse_toCursor(state, 1);
  mouse_Cursor_free(cursor);
  return 0;
}

l_checkTypeFn(l_mouse_isCursor, moduleData.cursorMT)
l_toTypeFn(l_mouse_toCursor, mouse_Cursor)


static int l_mouse_isVisible(lua_State *state) {
  lua_pushboolean(state, mouse_isVisible());
  return 1;
}


static int l_mouse_isDown(lua_State *state) {
  const char *name = luaL_checkstring(state, 1);
  int res = mouse_isDown(name);
  if(res < 0) {
    luaL_error(state, "bad button name '%s'", name);
  }
  lua_pushboolean(state, res);
  return 1;
}


static int l_mouse_getPosition(lua_State *state) {
  int x, y;
  mouse_getPosition(&x, &y);
  lua_pushinteger(state, x);
  lua_pushinteger(state, y);
  return 2;
}


static int l_mouse_getX(lua_State *state) {
  lua_pushinteger(state, mouse_getX());
  return 1;
} 


static int l_mouse_getY(lua_State *state) {
  lua_pushinteger(state, mouse_getY());
  return 1;
} 


static int l_mouse_setPosition(lua_State *state) {
  lua_pushstring(state, "love.mouse.setPosition not supported in Motor2D. There is no HTML5 API to do that. Sorry.");
  return lua_error(state);
} 


static int l_mouse_setVisible(lua_State *state) {
  mouse_setVisible(lua_toboolean(state, 1));
  return 0;
}


static int l_mouse_setX(lua_State *state) {
  lua_pushstring(state, "love.mouse.setX not supported in Motor2D. There is no HTML5 API to do that. Sorry.");
  return lua_error(state);
} 


static int l_mouse_setY(lua_State *state) {
  lua_pushstring(state, "love.mouse.setY not supported in Motor2D. There is no HTML5 API to do that. Sorry.");
  return lua_error(state);
} 


static void l_mouse_pressed(void *ud, int x, int y, int button) {
  lua_State * state = (lua_State*)ud;
  int top = lua_gettop(state);
  lua_getglobal(state, "love");
  lua_pushstring(state, "mousepressed");
  lua_rawget(state, -2);
  lua_pushnumber(state, x);
  lua_pushnumber(state, y);
  lua_pushstring(state, mouse_getButtonName(button));
  lua_call(state, 3, 0);
  lua_settop(state, top);
}


static void l_mouse_released(void *ud, int x, int y, int button) {
  lua_State * state = (lua_State*)ud;
  int top = lua_gettop(state);
  lua_getglobal(state, "love");
  lua_pushstring(state, "mousereleased");
  lua_rawget(state, -2);
  lua_pushnumber(state, x);
  lua_pushnumber(state, y);
  lua_pushstring(state, mouse_getButtonName(button));
  lua_call(state, 3, 0);
  lua_settop(state, top);
}


static void l_mouse_moved(void *ud, int x, int y, int dx, int dy) {
  lua_State * state = (lua_State*)ud;

  int top = lua_gettop(state);
  //lua_rawgeti(loopData->luaState, LUA_REGISTRYINDEX, loopData->errhand);
  lua_getglobal(state, "love");
  lua_pushstring(state, "mousemoved");
  lua_rawget(state, -2);
  lua_pushnumber(state, x);
  lua_pushnumber(state, y);
  lua_pushnumber(state, dx);
  lua_pushnumber(state, dy);
  lua_call(state, 4, 0);
  //pcall(state, 4);
  lua_settop(state, top);
}


static luaL_Reg const regFuncs[] = {
  {"isDown",         l_mouse_isDown        },
  {"isVisible",      l_mouse_isVisible     },
  {"getPosition",    l_mouse_getPosition   },
  {"getX",           l_mouse_getX          },
  {"getY",           l_mouse_getY          },
  {"setPosition",    l_mouse_setPosition   },
  {"setVisible",     l_mouse_setVisible    },
  {"setX",           l_mouse_setX          },
  {"setY",           l_mouse_setY          },
  {"newCursor",      l_mouse_newCursor     },
  {"setCursor",      l_mouse_setCursor     },
  {NULL, NULL }
};

static luaL_Reg const cursorMetatableFuncs[] = {
  {"__gc",           l_mouse_gcCursor},
  {NULL, NULL}
};



void l_mouse_register(lua_State* state) {
  l_tools_registerModule(state, "mouse", regFuncs);
  moduleData.cursorMT  = l_tools_makeTypeMetatable(state, cursorMetatableFuncs);
  moduleData.cursorRef = LUA_NOREF;

  mouse_EventCallbacks callbacks = {
    .pressed  = l_mouse_pressed,
    .released = l_mouse_released,
    .moved    = l_mouse_moved,
    .userData = state
  };

  mouse_setEventCallbacks(&callbacks);

}
