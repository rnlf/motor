#pragma once

#include <stdbool.h>
#include "gl.h"
#include "image.h"
#include "vertex.h"

typedef enum {
  graphics_MeshDrawMode_fan       = GL_TRIANGLE_FAN,
  graphics_MeshDrawMode_strip     = GL_TRIANGLE_STRIP,
  graphics_MeshDrawMode_triangles = GL_TRIANGLES,
  graphics_MeshDrawMode_points    = GL_POINTS
} graphics_MeshDrawMode;

typedef struct {
  graphics_Image const* texture;
  GLuint vertexBuffer;
  graphics_MeshDrawMode drawMode;
  GLuint indexBuffer;
  GLuint vertexArray;
  size_t vertexCount;
  graphics_Vertex *vertices;
  void *indices;
  size_t indexBufferSize;
  bool customIndexBuffer;
  bool useVertexColor;
  bool useDrawRange;
  int drawStart;
  int drawEnd;
} graphics_Mesh;


void graphics_Mesh_new(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices, graphics_Image const* texture, graphics_MeshDrawMode mode, bool useVertexColor);
void graphics_Mesh_free(graphics_Mesh *mesh);
void graphics_Mesh_setVertices(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices);
graphics_Vertex const* graphics_Mesh_getVertices(graphics_Mesh const *mesh, size_t *count);
graphics_Vertex const* graphics_Mesh_getVertex(graphics_Mesh const *mesh, size_t index);
void graphics_Mesh_setVertexMap(graphics_Mesh *mesh, size_t count, uint32_t const* indices);
void const* graphics_Mesh_getVertexMap(graphics_Mesh const* mesh, size_t* count);
void graphics_Mesh_draw(graphics_Mesh const* mesh, float x, float y, float r, float sx, float sy, float ox, float oy, float kx, float ky);
void graphics_Mesh_setVertexColors(graphics_Mesh *mesh, bool use);
bool graphics_Mesh_getVertexColors(graphics_Mesh const *mesh);
void graphics_Mesh_resetDrawRange(graphics_Mesh *mesh);
void graphics_Mesh_setDrawRange(graphics_Mesh *mesh, int min, int max);
bool graphics_Mesh_getDrawRange(graphics_Mesh const* mesh, int *min, int *max);
graphics_Vertex const* graphics_Mesh_getVertex(graphics_Mesh const *mesh, size_t index);
void graphics_Mesh_setVertex(graphics_Mesh *mesh, size_t index, graphics_Vertex const *vertex);
size_t graphics_Mesh_getVertexCount(graphics_Mesh const *mesh);
