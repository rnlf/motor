/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <tgmath.h>
#include "image.h"
#include "quad.h"
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
  unsigned code;
  int bearingX;
  int bearingY;
  int advance;
  int textureIdx;
  graphics_Quad textureCoords;
} graphics_Glyph;

typedef struct {
  graphics_Glyph const* glyphs;
  int numGlyphs;

} graphics_GlyphSet;

typedef struct {
  GLuint *textures;

  graphics_GlyphSet glyphs[256];

  int numTextures;
  int currentX;
  int currentY;
  int currentRowHeight;
  int textureWidth;
  int textureHeight;

} graphics_GlyphMap;

typedef struct {
  FT_Face face;
  graphics_GlyphMap glyphs;
  int height;
  int descent;
  int ascent;
  float lineHeight;
} graphics_Font;


int graphics_Font_getWrap(graphics_Font * font, char const* line, int width, char **wrapped);

void graphics_font_init(void);

int graphics_Font_new(graphics_Font *dst, char const* filename, int ptsize);

void graphics_Font_free(graphics_Font *obj);

int graphics_Font_getHeight(graphics_Font const* font);

int graphics_Font_getAscent(graphics_Font const* font);

int graphics_Font_getDescent(graphics_Font const* font);

int graphics_Font_getBaseline(graphics_Font const* font);

int graphics_Font_getWidth(graphics_Font * font, char const* line);

typedef enum {
  graphics_TextAlign_center,
  graphics_TextAlign_left,
  graphics_TextAlign_right,
  graphics_TextAlign_justify
} graphics_TextAlign;

void graphics_Font_render(graphics_Font* font, char const* text, int x, int y, float r, float sx, float sy, float ox, float oy, float kx, float ky);
void graphics_Font_printf(graphics_Font* font, char const* text, int px, int py, int limit, graphics_TextAlign align, float r, float sx, float sy, float ox, float oy, float kx, float ky);

void graphics_Font_setFilter(graphics_Font *font, graphics_Filter const* filter);
void graphics_Font_getFilter(graphics_Font *font, graphics_Filter *filter);
