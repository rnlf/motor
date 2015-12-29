/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "mouse.h"
#include "image/imagedata.h"
#include "graphics/image.h"

// TODO:
// moduleData.getRelativeMode()
// moduleData.getSystemCursor()
// moduleData.isGrabbed()
// moduleData.newCursor()
// moduleData.setCursor()
// moduleData.setGrabbed()
// moduleData.setRelativeMode()

#ifndef EMSCRIPTEN
extern SDL_Window* graphics_getWindow(void);
#endif

static void dummyPressed(void* ud, int x, int y, int button) {}
static void dummyReleased(void* ud, int x, int y, int button) {}
static void dummyMoved(void* ud, int x, int y, int dx, int dy) {}

static struct {
  int x, y;
  int dx, dy;
  int visible;
  int buttons[256];
  mouse_Cursor const *cursor;
  int posX;
  int posY;
  mouse_EventCallbacks callbacks;
} moduleData = {
  .callbacks = {
    .pressed  = dummyPressed,
    .released = dummyReleased,
    .moved    = dummyMoved
  }
};


static void setSystemCursorVisibility() {
  SDL_ShowCursor((moduleData.visible && (moduleData.cursor == 0))
    ? SDL_ENABLE : SDL_DISABLE);
}


char const *mouse_getButtonName(int button) {
  switch (button) {
    case SDL_BUTTON_LEFT:
      return "l";

    case SDL_BUTTON_RIGHT:
      return "r";

    case SDL_BUTTON_MIDDLE:
      return "m";

    case SDL_BUTTON_X1:
      return "x1";

    case SDL_BUTTON_X2:
      return "x2";

/*
    case SDL_BUTTON_WHEELDOWN:
      return "wd";

    case SDL_BUTTON_WHEELUP:
      return "wu";
*/
  }
  return "?";
}


static int buttonEnum(const char *str) {
  int res;
  switch (*str) {
    case 'l':
      res = SDL_BUTTON_LEFT;
      break;

    case 'r':
      res = SDL_BUTTON_RIGHT;
      break;

    case 'm':
      res = SDL_BUTTON_MIDDLE;
      break;
/*
    case 'w':
      str++;
      switch (*str) {
        case 'd':
          res = SDL_BUTTON_WHEELDOWN;
          break; 

        case 'u':
          res = SDL_BUTTON_WHEELUP;
          break;
        default:
          break;
      }
      break;
*/

    default:
      return -1;
  }

  if(str[1] != '\0') {
    return -1;
  }

  return res;
}


void mouse_mousemoved(int x, int y) {
  moduleData.posX = x;
  moduleData.posY = y;

  if(moduleData.x == x && moduleData.y == y) {
    return;
  }

  moduleData.dx = x - moduleData.x;
  moduleData.dy = y - moduleData.y;
  moduleData.x = x;
  moduleData.y = y;
  moduleData.callbacks.moved(moduleData.callbacks.userData, x, y, moduleData.dx, moduleData.dy);
}


void mouse_mousepressed(int x, int y, int button) {
  moduleData.buttons[button] = 1;
  moduleData.callbacks.pressed(moduleData.callbacks.userData, x, y, button);
}



void mouse_mousereleased(int x, int y, int button) {
  moduleData.buttons[button] = 0;
  moduleData.callbacks.released(moduleData.callbacks.userData, x, y, button);
}


void mouse_getPosition(int *x, int *y) {
  *x = moduleData.x;
  *y = moduleData.y;
}


int mouse_isDown(const char *str) {
  int x = buttonEnum(str);
  if(x < 0) {
    return -1;
  }

  return moduleData.buttons[x];
}


int mouse_isVisible(void) {
  return moduleData.visible;
}


int mouse_getX(void) {
  return moduleData.x;
}


int mouse_getY(void) {
  return moduleData.y;
}


void mouse_setVisible(bool b) {
  moduleData.visible = b;
  setSystemCursorVisibility();
}




void mouse_setX(int x) {
}


void mouse_setY(int y) {
}

void mouse_setEventCallbacks(mouse_EventCallbacks const *callbacks) {
  moduleData.callbacks = *callbacks;
}

void mouse_Cursor_new(mouse_Cursor* cursor, image_ImageData const *data, int hotx, int hoty) {
  graphics_Image_new_with_ImageData(&cursor->image, data);
  cursor->hotx = hotx;
  cursor->hoty = hoty;
}

void mouse_Cursor_free(mouse_Cursor* cursor) {
  graphics_Image_free(&cursor->image);
}

void mouse_setCursor(mouse_Cursor const * cursor) {
  moduleData.cursor = cursor;
  setSystemCursorVisibility();
}


static graphics_Quad const cursorQuad = {.x=0.0f, .y=0.0f, .w=1.0f, .h=1.0f};
void mouse_cursor_draw() {
  if(moduleData.cursor && mouse_isVisible()) {
    graphics_Image_draw(&moduleData.cursor->image, &cursorQuad,
      moduleData.posX - moduleData.cursor->hotx, 
      moduleData.posY - moduleData.cursor->hoty,
      0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  }
}
