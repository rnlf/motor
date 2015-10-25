#pragma once

#include "gl.h"
#include "image.h"

typedef enum {
  graphics_MeshDrawMode_fan,
  graphics_MeshDrawMode_strip,
  graphics_MeshDrawMode_triangles,
  graphics_MeshDrawMode_points
} graphics_MeshDrawMode;

typedef struct {
  graphics_Image const* texture;
  GLuint vertexBuffer;
  graphics_MeshDrawMode drawMode;
  GLuint indexBuffer;
  GLuint vertexArray;
  size_t vertexCount;
} graphics_Mesh;


void graphics_Mesh_new(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices, graphics_Image const* texture, graphics_MeshDrawMode mode);
void graphics_Mesh_free(graphics_Mesh *mesh);
void graphics_Mesh_setVertices(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices);
