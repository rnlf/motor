/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

typedef struct {
  float x;
  float y;
  float w;
  float h;
} graphics_Quad;

void graphics_Quad_new(graphics_Quad *dst, float x, float y, float w, float h);
void graphics_Quad_newWithRef(graphics_Quad *dst, float x, float y, float w, float h, float rw, float rh);
