#include "canvas.h"
#include "image.h"

static struct {
  graphics_Canvas ** canvas;
  int canvasCount;
  graphics_Canvas defaultCanvas;
  GLuint fbo;
} moduleData;


static void assertCanvasCount(int count) {
  if(count > moduleData.canvasCount) {
    moduleData.canvases = realloc(moduleData.canvases, sizeof(graphics_Canvas*) * count);
    moduleData.canvasCount = count;
  }
}

void graphics_Canvas_new(graphics_Canvas *canvas, int width, int height) {
  GLuint oldTex, oldFBO;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&oldTex);
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&oldFBO);
  glGenTextures(1, &canvas->image.texID);
  glBindTexture(GL_TEXTURE_2D, canvas->image.texID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

//  glGenFramebuffers(1, &canvas->fbo);
//  glBindFramebuffer(GL_FRAMEBUFFER, canvas->fbo);
//  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, canvas->image.texID, 0);
//  glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
  glBindTexture(GL_TEXTURE_2D, oldTex);

  m4x4_newTranslation(&canvas->projectionMatrix, -1.0f, -1.0f, 0.0f);
  m4x4_scale(&canvas->projectionMatrix, 2.0f / width, 2.0f / height, 0.0f);
  canvas->image.width = width;
  canvas->image.height = height;
  canvas->stencilBuf = 0;
}

void graphics_Canvas_free(graphics_Canvas *canvas) {
  graphics_Image_free(&canvas->image);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &canvas->fbo);
}


void graphics_Canvas_createStencilBuffer(graphics_Canvas *canvas) {
  if(canvas->stencilBuf > 0) {
    return;
  }

  GLuint oldFBO;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&oldFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, canvas->fbo);
  glGenRenderbuffers(1, &canvas->stencilBuf);
  glBindRenderbuffer(GL_RENDERBUFFER, canvas->stencilBuf);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, canvas->image.width, canvas->image.height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, canvas->stencilBuf);

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

}

void graphics_Canvas_draw(graphics_Canvas const* canvas, graphics_Quad const* quad,
                         float x, float y, float r, float sx, float sy,
                         float ox, float oy, float kx, float ky) {

  graphics_Image_draw(&canvas->image, quad, x, y, r, sx, sy, ox, oy, kx, ky);
}

void graphics_setCanvas(graphics_Canvas ** canvas, int count) {
  if(!canvas) {
    canvas = &moduleData.defaultCanvas;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, moduleData.fbo);
  }
  moduleData.canvas[0] = canvas;

  //glViewport(0,0,canvas->image.width, canvas->image.height);
}


void graphics_canvas_init(int width, int height) {
  glGenFramebuffers(1, &moduleData.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, moduleData.fbo);
  assertCanvasCount(1);
  m4x4_newTranslation(&moduleData.defaultCanvas.projectionMatrix, -1.0f, 1.0f, 0.0f);
  m4x4_scale(&moduleData.defaultCanvas.projectionMatrix, 2.0f / width, -2.0f / height, 0.0f);
  //moduleData.defaultCanvas.fbo = 0;
  moduleData.defaultCanvas.image.width = width;
  moduleData.defaultCanvas.image.height = height;
  moduleData.canvas = &moduleData.defaultCanvas;
  moduleData.defaultCanvas.stencilBuf = 1;
}

graphics_Canvas * graphics_getCanvas(void) {
  return moduleData.canvas;
}
