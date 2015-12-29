/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include "graphics.h"

typedef enum {
  graphics_DrawMode_fill,
  graphics_DrawMode_line
} graphics_DrawMode;

typedef enum {
  graphics_LineJoin_none,
  graphics_LineJoin_miter,
  graphics_LineJoin_bevel
} graphics_LineJoin;

typedef enum {
  graphics_DrawStyle_rough,
  graphics_DrawStyle_smooth
} graphics_DrawStyle;

void graphics_geometry_init(void);

void graphics_geometry_fillRectangle(float x, float y, float w, float h);
void graphics_geometry_drawRectangle(float x, float y, float w, float h);

void graphics_geometry_fillCircle(float x, float y, float radius, int segments);
void graphics_geometry_drawCircle(float x, float y, float radius, int segments);

void graphics_geometry_drawLines(int vertexCount, float const* vertices);

void graphics_geometry_setLineJoin(graphics_LineJoin join);
graphics_LineJoin graphics_geometry_getLineJoin();

float graphics_geometry_getLineWidth(void);
void  graphics_geometry_setLineWidth(float width);

void graphics_geometry_drawPolygon(int count, float const* vertices);
void graphics_geometry_fillPolygon(int count, float const* vertices);

void graphics_geometry_point(float x, float y);
void graphics_geometry_setPointStyle(graphics_DrawStyle style);
void graphics_geometry_setPointSize(float size);
graphics_DrawStyle graphics_geometry_getPointStyle();
float graphics_geometry_getPointSize();
