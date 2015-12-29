/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include "gl.h"
#include "quad.h"
#include "image.h"
#include "../math/vector.h"
#include "vertex.h"
#include <stdbool.h>

typedef enum {
  graphics_BatchUsage_static = GL_STATIC_DRAW,
  graphics_BatchUsage_dynamic = GL_DYNAMIC_DRAW,
  graphics_BatchUsage_stream = GL_STREAM_DRAW
} graphics_BatchUsage;

typedef struct {
  graphics_Image const *texture;
  GLuint vbo;
  graphics_Vertex *vertexData;
  int maxCount;
  int insertPos;
  GLuint vao;
  bool dirty;
  bool bound;
  vec4 color;
  bool colorSet;
  bool colorUsed;
  graphics_BatchUsage usage;
} graphics_Batch;


void graphics_batch_init(void);
void graphics_Batch_new(graphics_Batch* batch, graphics_Image const* texture, int maxSize, graphics_BatchUsage usage);
void graphics_Batch_free(graphics_Batch* batch);
int graphics_Batch_add(graphics_Batch* batch, graphics_Quad const* q, float x, float y, float r, float sx, float sy, float ox, float oy, float kx, float ky);
void graphics_Batch_set(graphics_Batch* batch, int id, graphics_Quad const* q, float x, float y, float r, float sx, float sy, float ox, float oy, float kx, float ky);
void graphics_Batch_draw(graphics_Batch const* batch, float x, float y, float r, float sx, float sy, float ox, float oy, float kx, float ky);
void graphics_Batch_bind(graphics_Batch *batch);
void graphics_Batch_unbind(graphics_Batch *batch);
void graphics_Batch_flush(graphics_Batch *batch);
void graphics_Batch_clear(graphics_Batch *batch);
void graphics_Batch_setBufferSize(graphics_Batch* batch, int newsize);
void graphics_Batch_setBufferSizeClearing(graphics_Batch* batch, int newsize);
void graphics_Batch_setColor(graphics_Batch* batch, float r, float g, float b, float a);
void graphics_Batch_clearColor(graphics_Batch* batch);
