/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <stdbool.h>
#include "graphics/image.h"

typedef struct {
  graphics_Image image;
  int hotx;
  int hoty;
} mouse_Cursor;

void mouse_Cursor_new(mouse_Cursor* cursor, image_ImageData const* data, int hotx, int hoty);
void mouse_setCursor(mouse_Cursor const * cursor);
mouse_Cursor const*  mouse_getCursor();
void mouse_cursor_draw();
void mouse_Cursor_free(mouse_Cursor* cursor);


typedef struct {
  void (*pressed)(void * userData, int x, int y, int button);
  void (*released)(void * userData, int x, int y, int button);
  void (*moved)(void * userData, int x, int y, int dx, int dy);
  void * userData;
} mouse_EventCallbacks;

void mouse_mousemoved(int x, int y);
void mouse_mousepressed(int x, int y, int button);
void mouse_mousereleased(int x, int y, int button);
int mouse_isDown(const char *str);
int mouse_isVisible(void);
void mouse_getPosition(int *x, int *y);
int mouse_getX(void);
int mouse_getY(void);
void mouse_setVisible(bool b);
void mouse_setX(int x);
void mouse_setY(int y);
char const *mouse_getButtonName(int button);

void mouse_setEventCallbacks(mouse_EventCallbacks const *callbacks);
