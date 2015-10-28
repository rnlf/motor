#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include "graphics.h"
#include "gl.h"
#include "../math/vector.h"
#include "matrixstack.h"
#include "font.h"
#include "batch.h"
#include "quad.h"
#include "canvas.h"
#include "shader.h"
#include "geometry.h"
#ifdef EMSCRIPTEN
# include <emscripten.h>
#endif


static struct {
//#ifndef EMSCRIPTEN
  SDL_Window* window;
  SDL_GLContext context;
//#endif
  SDL_Surface* surface;

  /*
  graphics_Color backgroundColor;
  graphics_Color foregroundColor;

  bool colorMask[4];
  graphics_BlendMode blendMode;
  int scissorBox[4];
  bool scissorSet;
*/

  graphics_DisplayState state;

  GLuint polygonVBO;
  GLuint polygonIBO;
  GLuint polygonVAO;
  graphics_Filter defaultFilter;
} moduleData = {
  .defaultFilter = {
    .maxAnisotropy = 1.0f,
    .mipmapLodBias = 1.0f,
    .minMode = graphics_FilterMode_linear,
    .magMode = graphics_FilterMode_linear,
    .mipmapMode = graphics_FilterMode_none
  }
};

//#ifndef EMSCRIPTEN
  SDL_Window* graphics_getWindow(void) {
    return moduleData.window;
  }
//#endif

void graphics_init(int width, int height) {
  SDL_Init(SDL_INIT_VIDEO);
  //#ifdef EMSCRIPTEN
  //  moduleData.surface = SDL_SetVideoMode(width, height, 0, SDL_OPENGL);
  //#else
  #ifndef EMSCRIPTEN
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  #else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  #endif
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    #if EMSCRIPTEN
      char const * title = emscripten_run_script_string("document.title");
    #else
      char const * title = "Motor2D";
    #endif

    moduleData.window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

    moduleData.context = SDL_GL_CreateContext(moduleData.window);
    SDL_GL_MakeCurrent(moduleData.window, moduleData.context);

    moduleData.surface = SDL_GetWindowSurface(moduleData.window);
    glewExperimental = GL_TRUE;

    printf("%d\n", glewInit());
  #ifndef EMSCRIPTEN
    SDL_GL_SetSwapInterval(1);
  #endif

  //#endif

//  glViewport(0,0,width,height);

  matrixstack_init();

  graphics_canvas_init(width, height);

  graphics_setColor(1.0f, 1.0f, 1.0f, 1.0f);

  graphics_geometry_init();
  graphics_font_init();
  graphics_batch_init();
  graphics_image_init();
  graphics_shader_init();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  graphics_setColorMask(true, true, true, true);
  graphics_setBlendMode(graphics_BlendMode_alpha);
  glEnable(GL_BLEND);
  graphics_clearScissor();

  glGenVertexArrays(1, &moduleData.polygonVAO);
  glBindVertexArray(moduleData.polygonVAO);
  glGenBuffers(1, &moduleData.polygonVBO);
  glGenBuffers(1, &moduleData.polygonIBO);
}

void graphics_setBackgroundColor(float red, float green, float blue, float alpha) {
  moduleData.state.backgroundColor.red   = red;
  moduleData.state.backgroundColor.green = green;
  moduleData.state.backgroundColor.blue  = blue;
  moduleData.state.backgroundColor.alpha = alpha;
  glClearColor(red, green, blue, alpha);
}

void graphics_setColor(float red, float green, float blue, float alpha) {
  moduleData.state.foregroundColor.red   = red;
  moduleData.state.foregroundColor.green = green;
  moduleData.state.foregroundColor.blue  = blue;
  moduleData.state.foregroundColor.alpha = alpha;
}

void graphics_clear(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void graphics_swap(void) {
//#ifdef EMSCRIPTEN
//  SDL_GL_SwapBuffers();
//#else
  SDL_GL_SwapWindow(moduleData.window);
//#endif
}

void graphics_drawArray(graphics_Quad const* quad, mat4x4 const* tr2d, GLuint vao, GLuint ibo, GLuint offset, GLuint count, GLenum type, GLenum indexType, float const* useColor, float ws, float hs) {

  mat4x4 tr;
  //m4x4_mul_m4x4(&tr, matrixstack_head(), tr2d);
  m4x4_mulM4x4(&tr, tr2d, matrixstack_head());

  graphics_Shader_activate(
    &graphics_getCanvas()->projectionMatrix,
    &tr,
    quad,
    useColor,
    ws,
    hs
  );

  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glDrawElements(type, count, indexType, (GLvoid const*)offset);
}

int graphics_getWidth(void) {
  return moduleData.surface->w;
}

int graphics_getHeight(void) {
  return moduleData.surface->h;
}

float* graphics_getColor(void) {
  return (float*)(&moduleData.state.foregroundColor);
}

float* graphics_getBackgroundColor(void) {
  return (float*)(&moduleData.state.backgroundColor);
}

void graphics_setColorMask(bool r, bool g, bool b, bool a) {
  moduleData.state.colorMask[0] = r;
  moduleData.state.colorMask[1] = g;
  moduleData.state.colorMask[2] = b;
  moduleData.state.colorMask[3] = a;

  glColorMask(r,g,b,a);
}

void graphics_getColorMask(bool *r, bool *g, bool *b, bool *a) {
  *r = moduleData.state.colorMask[0];
  *g = moduleData.state.colorMask[1];
  *b = moduleData.state.colorMask[2];
  *a = moduleData.state.colorMask[3];
}

graphics_BlendMode graphics_getBlendMode(void) {
  return moduleData.state.blendMode;
}

void graphics_setBlendMode(graphics_BlendMode mode) {
  moduleData.state.blendMode = mode;

  GLenum sfRGB = GL_ONE;
  GLenum dfRGB = GL_ZERO;
  GLenum sfA = GL_ONE;
  GLenum dfA = GL_ZERO;
  GLenum bFunc = GL_FUNC_ADD;

  switch(mode) {
  case graphics_BlendMode_alpha:
    sfRGB = GL_SRC_ALPHA;
    sfA = GL_ONE;
    dfRGB = dfA = GL_ONE_MINUS_SRC_ALPHA;
    break;

  case graphics_BlendMode_subtractive:
    bFunc = GL_FUNC_REVERSE_SUBTRACT;
    // fallthrough
  case graphics_BlendMode_additive:
    sfA = sfRGB = GL_SRC_ALPHA;
    dfA = dfRGB = GL_ONE;
    break;


  case graphics_BlendMode_multiplicative:
    sfA = sfRGB = GL_DST_COLOR;
    dfA = dfRGB = GL_ZERO;
    break;

  case graphics_BlendMode_premultiplied:
    sfA = sfRGB = GL_ONE;
    dfA = dfRGB = GL_ONE_MINUS_SRC_ALPHA;
    break;

  case graphics_BlendMode_screen:
    sfA = sfRGB = GL_ONE;
    dfA = dfRGB = GL_ONE_MINUS_SRC_COLOR;
    break;

  case graphics_BlendMode_replace:
  default:
    // uses default init values   
    break;
  }

  glBlendFuncSeparate(sfRGB, dfRGB, sfA, dfA);
  glBlendEquation(bFunc);
}

void graphics_clearScissor(void) {
  moduleData.state.scissorSet = false;
  glDisable(GL_SCISSOR_TEST);
}

void graphics_setScissor(int x, int y, int w, int h) {
  moduleData.state.scissorBox[0] = x;
  moduleData.state.scissorBox[1] = y;
  moduleData.state.scissorBox[2] = w;
  moduleData.state.scissorBox[3] = h;
  moduleData.state.scissorSet = true;
  glScissor(x,y,w,h);
  glEnable(GL_SCISSOR_TEST);
}

bool graphics_getScissor(int *x, int *y, int *w, int *h) {
  if(!moduleData.state.scissorSet) {
    return false;
  }

  *x = moduleData.state.scissorBox[0];
  *y = moduleData.state.scissorBox[1];
  *w = moduleData.state.scissorBox[2];
  *h = moduleData.state.scissorBox[3];

  return true;
}

void graphics_defineStencil(void) {
  graphics_Canvas_createStencilBuffer(graphics_getCanvas());
  
  // Disable color writes but don't save the mask values.
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  glClear(GL_STENCIL_BUFFER_BIT);
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_ALWAYS, 1, 1);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void graphics_useStencil(bool invert) {
  glStencilFunc(GL_EQUAL, (GLint)(!invert), 1); // invert ? 0 : 1
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  glColorMask(moduleData.state.colorMask[0],moduleData.state.colorMask[1],moduleData.state.colorMask[2],moduleData.state.colorMask[3]);
}

void graphics_discardStencil(void) {
  glColorMask(moduleData.state.colorMask[0],moduleData.state.colorMask[1],moduleData.state.colorMask[2],moduleData.state.colorMask[3]);
  glDisable(GL_STENCIL_TEST);
}

graphics_Filter* graphics_getDefaultFilter(void) {
  return &moduleData.defaultFilter;
}

void graphics_reset(void) {
  // TODO point and line drawing modes
  matrixstack_origin();
  graphics_setColor(1.0f, 1.0f, 1.0f, 1.0f);
  graphics_setBackgroundColor(0.0f, 0.0f, 0.0f, 1.0f);
  graphics_setBlendMode(graphics_BlendMode_alpha);
  graphics_setDefaultShader();
  graphics_setColorMask(true, true, true, true);
  graphics_clearScissor();
  graphics_setCanvas(NULL);
}


graphics_DisplayState const* graphics_getState(void) {
  return &moduleData.state;
}


void graphics_setState(graphics_DisplayState const* state) {
  memcpy(&moduleData.state, state, sizeof(*state));
}
