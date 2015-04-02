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

void graphics_geometry_init();

void graphics_geometry_fillRectangle(float x, float y, float w, float h);
void graphics_geometry_drawRectangle(float x, float y, float w, float h);

void graphics_geometry_fillCircle(float x, float y, float radius, int segments);
void graphics_geometry_drawCircle(float x, float y, float radius, int segments);

void graphics_geometry_drawLines(int vertexCount, float const* vertices);

void graphics_geometry_setLineJoin(graphics_LineJoin join);
graphics_LineJoin graphics_geometry_getLineJoin();

float graphics_geometry_getLineWidth();
void  graphics_geometry_setLineWidth(float width);