/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#ifdef EMSCRIPTEN
# include <emscripten.h>
#endif
#include <stdio.h>
#include <dirent.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <unistd.h>

#include "luaapi/audio.h"
#include "luaapi/graphics.h"
#include "luaapi/joystick.h"
#include "luaapi/graphics_font.h"
#include "luaapi/image.h"
#include "luaapi/motor.h"
#include "luaapi/boot.h"
#include "luaapi/keyboard.h"
#include "luaapi/mouse.h"
#include "luaapi/filesystem.h"
#include "luaapi/timer.h"
#include "luaapi/math.h"
#include "luaapi/event.h"

#include "graphics/graphics.h"
#include "graphics/matrixstack.h"

#include "audio/audio.h"

#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "image/imagedata.h"
#include "timer/timer.h"
#include "math/math.h"
#include "errorhandler.h"
#include "filesystem/filesystem.h"


typedef struct {
//  double lastTime;
  lua_State *luaState;
  int errhand;
} MainLoopData;


void main_loop(void *data) {
  MainLoopData* loopData = (MainLoopData*)data;
  // TODO use registry to get love.update and love.draw?

  chdir("love");

  timer_step();
  graphics_clear();
  matrixstack_origin();
  lua_rawgeti(loopData->luaState, LUA_REGISTRYINDEX, loopData->errhand);
  lua_getglobal(loopData->luaState, "love");
  lua_pushstring(loopData->luaState, "update");

  lua_rawget(loopData->luaState, -2);
  lua_pushnumber(loopData->luaState, timer_getDelta());
  pcall(loopData->luaState, 1);

  lua_pushstring(loopData->luaState, "draw");
  lua_rawget(loopData->luaState, -2);

  pcall(loopData->luaState, 0);

  graphics_DisplayState curState = *graphics_getState();
  matrixstack_push();

  mouse_cursor_draw();

  matrixstack_pop();
  graphics_setState(&curState);

  graphics_swap();

  lua_pop(loopData->luaState, 1);

  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
    case SDL_KEYDOWN:
      keyboard_keypressed(event.key.keysym.sym);
      break;
    case SDL_KEYUP:
      keyboard_keyreleased(event.key.keysym.sym);
      break;
    case SDL_TEXTINPUT:
      keyboard_textInput(event.text.text);
      break;
    case SDL_MOUSEMOTION:
      mouse_mousemoved(event.motion.x, event.motion.y);
      break;
    case SDL_MOUSEBUTTONDOWN:
      mouse_mousepressed(event.button.x, event.button.y, event.button.button);
      break;
    case SDL_MOUSEBUTTONUP:
      mouse_mousereleased(event.button.x, event.button.y, event.button.button);
      break;
    case SDL_JOYDEVICEADDED:
      joystick_deviceAdded(event.jdevice.which);
      break;
    case SDL_JOYDEVICEREMOVED:
      joystick_deviceRemoved(event.jdevice.which);
      break;
    case SDL_JOYBUTTONUP:
      joystick_buttonReleased(event.jbutton.which, event.jbutton.button);
      break;
    case SDL_JOYBUTTONDOWN:
      joystick_buttonPressed(event.jbutton.which, event.jbutton.button);
      break;
    case SDL_JOYAXISMOTION:
      joystick_axisEvent(event.jaxis.which, event.jaxis.axis, event.jaxis.value);
      break;


#ifndef EMSCRIPTEN
    case SDL_QUIT:
      exit(0);
#endif
    }
  }

  audio_updateStreams();
  //lua_gc(loopData->luaState, LUA_GCCOLLECT, 0);
}

MainLoopData mainLoopData;

int main() {
  lua_State *lua = luaL_newstate();
  luaL_openlibs(lua);


  motor_Config config;
  l_motor_register(lua);
  l_audio_register(lua);
  l_graphics_register(lua);
  l_image_register(lua);
  l_keyboard_register(lua);
  l_mouse_register(lua);
  l_filesystem_register(lua);
  l_timer_register(lua);
  l_math_register(lua);
  l_event_register(lua);
  l_joystick_register(lua);

  chdir("/love");
  l_boot(lua, &config);

  if(config.identity) {
    filesystem_setIdentity(config.identity, false);
  }

  image_init();
  joystick_init();
  keyboard_init();
  graphics_init(config.window.width, config.window.height);
  audio_init();
  math_init();


  if(luaL_dofile(lua, "main.lua")) {
    printf("Error: %s\n", lua_tostring(lua, -1));
  }

  lua_pushcfunction(lua, errorhandler);
  lua_getglobal(lua, "love");
  lua_pushstring(lua, "load");
  lua_rawget(lua, -2);
  /*if(lua_pcall(lua, 0, 0, 1)) {
    printf("Error in love.load: %s\n", lua_tostring(lua, -1));
  }
  */
  pcall(lua, 0);
  lua_pop(lua, 1);

  lua_pushcfunction(lua, errorhandler);

  mainLoopData.luaState = lua;
  mainLoopData.errhand = luaL_ref(lua, LUA_REGISTRYINDEX);

  timer_init();
#ifdef EMSCRIPTEN
  emscripten_set_main_loop_arg(main_loop, &mainLoopData, 0, 1);
#else
  for(;;) {
    main_loop(&mainLoopData);
  }
#endif
}
