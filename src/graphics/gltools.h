/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include "gl.h"

typedef enum {
  graphics_FilterMode_none = 0,
  graphics_FilterMode_nearest = 1,
  graphics_FilterMode_linear = 2
} graphics_FilterMode;

typedef struct {
  float maxAnisotropy;
  float mipmapLodBias;
  graphics_FilterMode minMode;
  graphics_FilterMode magMode;
  graphics_FilterMode mipmapMode;
} graphics_Filter;

void graphics_Texture_getFilter(GLuint texID, graphics_Filter * filter);
void graphics_Texture_setFilter(GLuint texID, graphics_Filter const * filter);
