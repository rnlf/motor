/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <stdbool.h>
#include "image.h"
#include "quad.h"
#include "../math/vector.h"
#include "canvas.h"

void graphics_init(int width, int height);

typedef enum {
  graphics_BlendMode_additive,
  graphics_BlendMode_alpha,
  graphics_BlendMode_subtractive,
  graphics_BlendMode_multiplicative,
  graphics_BlendMode_premultiplied,
  graphics_BlendMode_replace,
  graphics_BlendMode_screen
} graphics_BlendMode;

typedef struct {
  float red;
  float green;
  float blue;
  float alpha;
} graphics_Color;

typedef struct {
  graphics_Color foregroundColor;
  graphics_Color backgroundColor;

  bool colorMask[4];
  graphics_BlendMode blendMode;
  int scissorBox[4];
  bool scissorSet;

} graphics_DisplayState;

void graphics_setBackgroundColor(float red, float green, float blue, float alpha);
void graphics_setColor(float red, float green, float blue, float alpha);
float* graphics_getColor(void);
float* graphics_getBackgroundColor(void);
void graphics_clear(void);
void graphics_swap(void);
void graphics_drawArray(graphics_Quad const* quad, mat4x4 const* tr2d, GLuint vao, GLuint ibo, GLuint offset, GLuint count, GLenum type, GLenum indexType, float const * useColor, float ws, float hs, bool useVertexColors);
int graphics_getWidth(void);
int graphics_getHeight(void);
void graphics_setColorMask(bool r, bool g, bool b, bool a);
void graphics_getColorMask(bool *r, bool *g, bool *b, bool *a);
graphics_BlendMode graphics_getBlendMode(void);
void graphics_setBlendMode(graphics_BlendMode mode);
void graphics_clearScissor(void);
void graphics_setScissor(int x, int y, int w, int h);
bool graphics_getScissor(int *x, int *y, int *w, int *h);
void graphics_reset(void);
void graphics_defineStencil(void);
void graphics_useStencil(bool invert);
void graphics_discardStencil(void);
graphics_Filter* graphics_getDefaultFilter(void);
graphics_DisplayState const* graphics_getState(void);
void graphics_setState(graphics_DisplayState const* state);
