#pragma once

#include "gl.h"
#include "../math/vector.h"
#include "quad.h"
#include "image.h"

typedef struct {
  graphics_Image image;

//  GLuint fbo;
  GLuint stencilBuf;
  mat4x4 projectionMatrix;

} graphics_Canvas;

void graphics_Canvas_new(graphics_Canvas *canvas, int width, int height);
void graphics_Canvas_free(graphics_Canvas *canvas);
void graphics_Canvas_draw(graphics_Canvas const* canvas, graphics_Quad const* quad,
                         float x, float y, float r, float sx, float sy,
                         float ox, float oy, float kx, float ky);
void graphics_setCanvas(graphics_Canvas * canvas);
graphics_Canvas * graphics_getCanvas(void);
void graphics_canvas_init(int width, int height);
void graphics_Canvas_createStencilBuffer(graphics_Canvas *canvas);
