#include <tgmath.h>
#include <stdlib.h>
#include "../math/util.h"
#include "geometry.h"
#include "graphics.h"
#include "shader.h"
#include "matrixstack.h"

static struct {
  GLuint dataVBO;
  GLuint dataIBO;
  GLuint dataVAO;
  int currentDataSize;
  int currentIndexSize;
  float *data;
  uint16_t *index;
  graphics_Shader plainColorShader;
  float lineWidth;
  graphics_LineJoin join;
  graphics_DrawStyle pointStyle;
  float pointSize;
} moduleData;


void graphics_geometry_init(void) {
  glGenVertexArrays(1, &moduleData.dataVAO);
  glBindVertexArray(moduleData.dataVAO);
  glGenBuffers(1, &moduleData.dataIBO);
  glGenBuffers(1, &moduleData.dataVBO);
  glBindBuffer(GL_ARRAY_BUFFER, moduleData.dataVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moduleData.dataIBO);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(2*sizeof(float)));

  graphics_Shader_new(&moduleData.plainColorShader, NULL,
    "vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_cords ) {\n"
    "  return color;\n"
    "}\n");

  moduleData.lineWidth = 1.0f;
  moduleData.join = graphics_LineJoin_none;
}


static void growBuffers(int vertices, int indices) {
  int datasize = vertices * 6 * sizeof(float);
  if(moduleData.currentDataSize < datasize) {
    free(moduleData.data);
    moduleData.data = (float*)malloc(datasize);
    moduleData.currentDataSize = datasize;
  }

  int indexsize = indices * sizeof(uint16_t);
  if(moduleData.currentIndexSize < indexsize) {
    free(moduleData.index);
    moduleData.index = (uint16_t*)malloc(indexsize);
    moduleData.currentIndexSize = indexsize;
  }
}


static void drawBuffer(int vertices, int indices, GLenum type) {
  glBindBuffer(GL_ARRAY_BUFFER, moduleData.dataVBO);
  glBufferData(GL_ARRAY_BUFFER, vertices*6*sizeof(float), moduleData.data, GL_STREAM_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moduleData.dataIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices*sizeof(uint16_t), moduleData.index, GL_STREAM_DRAW);

  graphics_Shader *shader = graphics_getShader();
  graphics_setShader(&moduleData.plainColorShader);
  mat4x4 tr;
  m4x4_newIdentity(&tr);
  graphics_Quad quad = {0,0,1,1};

  graphics_drawArray(&quad, &tr, moduleData.dataVAO, moduleData.dataIBO, 0, indices, type, GL_UNSIGNED_SHORT, graphics_getColor(), 1, 1);

  graphics_setShader(shader);
}

void graphics_geometry_drawCircle(float x, float y, float radius, int segments) {
  growBuffers(segments*2, segments*2+2);

  float step = 2*PI / segments;
  float ang = 0;
  moduleData.index[2*segments  ] = 0;
  moduleData.index[2*segments+1] = 1;
  float lwh = moduleData.lineWidth / 2.0f;
  for(int i = 0; i < segments; ++i, ang += step) {
    float s = sin(ang);
    float c = cos(ang);
    float * base = moduleData.data + 12*i;

    // outer vertex
    base[0] = s * (radius+lwh) + x;
    base[1] = c * (radius+lwh) + y;
    base[2]  = 1.0f;
    base[3]  = 1.0f;
    base[4]  = 1.0f;
    base[5]  = 1.0f;

    // inner vertex
    base[6] = s * (radius-lwh) + x;
    base[7] = c * (radius-lwh) + y;
    base[8]  = 1.0f;
    base[9]  = 1.0f;
    base[10] = 1.0f;
    base[11] = 1.0f;

    moduleData.index[2*i] = 2*i;
    moduleData.index[2*i+1] = 2*i+1;
  }

  drawBuffer(segments*2, segments*2+2, GL_TRIANGLE_STRIP);
}


void graphics_geometry_fillCircle(float x, float y, float radius, int segments) {
  growBuffers(segments+1, segments+2);

  float step = 2*PI / segments;
  float ang = 0;
  moduleData.data[0] = x;
  moduleData.data[1] = y;
  moduleData.data[2] = 1.0f;
  moduleData.data[3] = 1.0f;
  moduleData.data[4] = 1.0f;
  moduleData.data[5] = 1.0f;
  moduleData.index[0] = 0;
  moduleData.index[segments+1] = 1;
  for(int i = 0; i < segments; ++i, ang -= step) {
    float * base = moduleData.data + 6*(i+1);
    base[0] = sin(ang) * radius + x;
    base[1] = cos(ang) * radius + y;
    base[2] = 1.0f;
    base[3] = 1.0f;
    base[4] = 1.0f;
    base[5] = 1.0f;
    moduleData.index[i+1] = i+1;
  }

  drawBuffer(segments+1, segments+2, GL_TRIANGLE_FAN);
}

void graphics_geometry_fillRectangle(float x, float y, float w, float h) {
  graphics_Shader *shader = graphics_getShader();

  graphics_Image img = {
    0, w, h
  };
  graphics_Quad q = {0,0,1,1};
  graphics_setShader(&moduleData.plainColorShader);
  graphics_Image_draw(&img, &q, x, y, 0, 1, 1, 0, 0, 0, 0);

  graphics_setShader(shader);
}


void graphics_geometry_drawRectangle(float x, float y, float w, float h) {
  growBuffers(8, 10);

  float *buf = moduleData.data;
  float lwh = moduleData.lineWidth / 2.0f;

  // top left
  buf[0] = x - lwh;
  buf[1] = y - lwh;

  buf[6] = x + lwh;
  buf[7] = y + lwh;

  float x2 = x + w;
  float y2 = y + h;

  // top right
  buf[12] = x2 + lwh;
  buf[13] = y  - lwh;

  buf[18] = x2 - lwh;
  buf[19] = y  + lwh;

  // bottom right
  buf[24] = x2 + lwh;
  buf[25] = y2 + lwh;

  buf[30] = x2 - lwh;
  buf[31] = y2 - lwh;

  // bottom left
  buf[36] = x - lwh;
  buf[37] = y2 + lwh;

  buf[42] = x + lwh;
  buf[43] = y2 - lwh;

  for(int i = 0; i < 8; ++i) {
    for(int j = 2; j < 6; ++j) {
      buf[6*i+j] = 1.0f;
    }
    moduleData.index[i] = i;
  }
  moduleData.index[8] = 0;
  moduleData.index[9] = 1;

  drawBuffer(8, 10, GL_TRIANGLE_STRIP);
}


static void mkNormal(float px1, float py1, float px2, float py2, float *nx, float *ny) {
  float dx = px2 - px1;
  float dy = py2 - py1;
  float l = 0.5f * moduleData.lineWidth / sqrt(dx*dx+dy*dy);
  *nx = dy * l;
  *ny = -dx * l;
}

static inline void makeNoneVertex(float *output, float px1, float py1, float px2, float py2) {
    float nx, ny;
    mkNormal(px1, py1, px2, py2, &nx, &ny);

    output[0]  = px1 - nx;
    output[1]  = py1 - ny;
    output[6]  = px1 + nx;
    output[7]  = py1 + ny;
    output[12] = px2 - nx;
    output[13] = py2 - ny;
    output[18] = px2 + nx;
    output[19] = py2 + ny;
    

    for(int j = 0; j < 4; ++j) {
      for(int k = 0; k < 4; ++k) {
        output[6*j + 2 + k] = 1.0f;
      }
    }
}


static void drawLineJoinNone(int vertexCount, float const* vertices, bool polygon) {
  int verts = 4*(vertexCount-(polygon?0:1));
  int indices = 6*(vertexCount-(polygon?0:1));
  growBuffers(verts, indices);

  for(int i = 0; i < vertexCount - 1; ++i) {
    float px1 = vertices[2*i];
    float py1 = vertices[2*i+1];
    float px2 = vertices[2*i+2];
    float py2 = vertices[2*i+3];

    float * base = moduleData.data + 24 * i;
    makeNoneVertex(base, px1, py1, px2, py2);
  }

  if(polygon) {
    makeNoneVertex(moduleData.data + (vertexCount-1) * 24, vertices[2*vertexCount-2], vertices[2*vertexCount-1], vertices[0], vertices[1]);
  }

  for(int i = 0; i < vertexCount-(polygon?0:1); ++i) {
    uint16_t * base = moduleData.index + i * 6;

    base[0] = 4*i;
    base[1] = 4*i+1;
    base[2] = 4*i+2;
    base[3] = 4*i+1;
    base[4] = 4*i+2;
    base[5] = 4*i+3;
  }

  drawBuffer(verts, indices, GL_TRIANGLES);
}


typedef struct {
  float nx, ny;
  float px, py;
} MiterLinesState;
 

static void makeMiterVertex(MiterLinesState *state, float* output, float px2, float py2) {
  // Compute offset from centerline for the current vertex.
  // It works like this: Take angle bisector of the normal vectors
  // for the adjacent line segments and scale it to the required length.
  // The bisector is the sum of the unit length normal vectors and renormalized.
  // The normal vector, line segment and the bisector form a right triangle,
  // of which we know the length of one cathetus, and the angle between this leg
  // and the hypothenuse.
  // The known cathetus is half the line width, the length of the hypothenuse is
  // the length which we have to scale the bisector to.
  // The cosine of the angle between the two can be obtained by forming the
  // dot product between the unit normal vector and the normalized bisector.
  float nx2, ny2;
  mkNormal(state->px, state->py, px2, py2, &nx2, &ny2);

  float sx = state->nx + nx2;
  float sy = state->ny + ny2;
  float s = 1.0f / sqrt(sx*sx+sy*sy);
  // sx,sy is unit length bisector
  sx *= s;
  sy *= s;

  // c is length of hypothenuse in our triangle
  float c = moduleData.lineWidth * moduleData.lineWidth / (4*(state->nx*sx + state->ny*sy));

  // sx,sy is bisector scaled to the required length, i.e. the offset from the
  // vertex for both sides
  sx *= c;
  sy *= c;

  // Add the vertices
  output[0] = state->px - sx;
  output[1] = state->py - sy;
  output[6] = state->px + sx;
  output[7] = state->py + sy;

  // Needed for next iteration
  state->px = px2;
  state->py = py2;
  state->nx = nx2;
  state->ny = ny2;
}

static void drawLineJoinMiter(int vertexCount, float const* vertices, bool polygon) {
  growBuffers(vertexCount * 2, (vertexCount + (polygon ? 1 : 0)) * 2);

  MiterLinesState state;

  // First vertex
  // Note: reversed order to avoid having to swap them before entering the loop

  if(!polygon) {
    state.px = vertices[2];
    state.py = vertices[3];
    float px2 = vertices[0];
    float py2 = vertices[1];
    mkNormal(px2, py2, state.px, state.py, &state.nx, &state.ny);
    // First vertex
    moduleData.data[0] = px2 - state.nx;
    moduleData.data[1] = py2 - state.ny;
    moduleData.data[6] = px2 + state.nx;
    moduleData.data[7] = py2 + state.ny;
  } else {
    float px2 = vertices[0];
    float py2 = vertices[1];
    state.px = vertices[2*vertexCount-2];
    state.py = vertices[2*vertexCount-1];
    mkNormal(px2, py2, state.px, state.py, &state.nx, &state.ny);
    makeMiterVertex(&state, moduleData.data, px2, py2);
  }


  // Inner vertices
  for(int i = polygon ? 0 : 1; i < vertexCount - 1; ++i) {
    float px2 = vertices[(i+1) * 2];
    float py2 = vertices[(i+1) * 2 + 1];
    float *base = moduleData.data + i * 12;

    makeMiterVertex(&state, base, px2, py2);
  }

  // Last vertex
  if(!polygon) {
    moduleData.data[(vertexCount - 1) * 12    ] = state.px - state.nx;
    moduleData.data[(vertexCount - 1) * 12 + 1] = state.py - state.ny;
    moduleData.data[(vertexCount - 1) * 12 + 6] = state.px + state.nx;
    moduleData.data[(vertexCount - 1) * 12 + 7] = state.py + state.ny;
  } else {
    float px2 = vertices[0];
    float py2 = vertices[1];
    float *base = moduleData.data + (vertexCount - 1) * 12;

    makeMiterVertex(&state, base, px2, py2);

    // Add indices for last segment (from last back to first vertex)
    moduleData.index[vertexCount * 2 ] = 0;
    moduleData.index[vertexCount * 2 + 1] = 1;
  }

  for(int i = 0; i < vertexCount * 2; ++i) {
    for(int j = 0; j < 4; ++j) {
      moduleData.data[i*6+j+2] = 1.0f;
    }
    moduleData.index[i] = i;
  }


  drawBuffer(vertexCount * 2, (vertexCount + (polygon ? 1 : 0)) * 2, GL_TRIANGLE_STRIP);
}

void graphics_geometry_drawLines(int vertexCount, float const* vertices) {
  switch(moduleData.join) {
  case graphics_LineJoin_none:
    drawLineJoinNone(vertexCount, vertices, false);
    break;

  case graphics_LineJoin_miter:
    drawLineJoinMiter(vertexCount, vertices, false);
    break;

  case graphics_LineJoin_bevel:
    //makeLineJoinBevel(vertexCount, vertices);
    break;
  }

}


void graphics_geometry_drawPolygon(int count, float const* vertices) {
  switch(moduleData.join) {
  case graphics_LineJoin_none:
    drawLineJoinNone(count, vertices, true);
    break;

  case graphics_LineJoin_miter:
    drawLineJoinMiter(count, vertices, true);
    break;

  case graphics_LineJoin_bevel:
    //makeLineJoinBevel(vertexCount, vertices, true);
    break;
  }
}


void graphics_geometry_fillPolygon(int count, float const* vertices) {
  growBuffers(count, count);

  for(int i = 0; i < count; ++i) {
    moduleData.data[i*6] = vertices[2*i];
    moduleData.data[i*6+1] = vertices[2*i+1];
    moduleData.data[i*6+2] = 1.0f;
    moduleData.data[i*6+3] = 1.0f;
    moduleData.data[i*6+4] = 1.0f;
    moduleData.data[i*6+5] = 1.0f;

    moduleData.index[i] = i;
  }

  drawBuffer(count, count, GL_TRIANGLE_FAN);
}


void graphics_geometry_point(float x, float y) {
  switch(moduleData.pointStyle) {
  case graphics_DrawStyle_rough:
    graphics_geometry_fillRectangle(x - moduleData.pointSize / 2.0f, y - moduleData.pointSize / 2.0f, moduleData.pointSize, moduleData.pointSize);
    break;
      
  case graphics_DrawStyle_smooth:
    graphics_geometry_fillCircle(x, y, moduleData.pointSize / 2.0f, moduleData.pointSize > 20.0f ? (int)(moduleData.pointSize / 2.0f): 20);
    break;
  }
}


float graphics_geometry_getLineWidth(void) {
  return moduleData.lineWidth;
}


void graphics_geometry_setLineWidth(float width) {
  moduleData.lineWidth = width;
}


void graphics_geometry_setLineJoin(graphics_LineJoin join) {
  moduleData.join = join;
}


graphics_LineJoin graphics_geometry_getLineJoin() {
  return moduleData.join;
}

void graphics_geometry_setPointStyle(graphics_DrawStyle style) {
  moduleData.pointStyle = style;
}

void graphics_geometry_setPointSize(float size) {
  moduleData.pointSize = size;
}

graphics_DrawStyle graphics_geometry_getPointStyle() {
  return moduleData.pointStyle;
}

float graphics_geometry_getPointSize() {
  return moduleData.pointSize;
}
