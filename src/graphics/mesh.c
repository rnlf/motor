#include <stdlib.h>
#include <string.h>
#include "mesh.h"
#include "graphics.h"

void graphics_Mesh_new(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices, graphics_Image const* texture, graphics_MeshDrawMode mode) {
  mesh->texture = texture;
  mesh->drawMode = mode;

  mesh->vertices = 0;
  mesh->indices = 0;

  mesh->customIndexBuffer = false;

  glGenVertexArrays(1, &mesh->vertexArray);
  glBindVertexArray(mesh->vertexArray);
  glGenBuffers(1, &mesh->indexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
  glGenBuffers(1, &mesh->vertexBuffer);
  graphics_Mesh_setVertices(mesh, vertexCount, vertices);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(graphics_Vertex), 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(graphics_Vertex), (GLvoid const*)(2*sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(graphics_Vertex), (GLvoid const*)(4*sizeof(float)));
}


void graphics_Mesh_free(graphics_Mesh *mesh) {
  glDeleteBuffers(1,      &mesh->vertexBuffer);
  glDeleteBuffers(1,      &mesh->indexBuffer);
  glDeleteVertexArrays(1, &mesh->vertexArray);
  free(mesh->indices);
  free(mesh->vertices);
}


void graphics_Mesh_setVertices(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices) {
  printf("vbo: %d, sz: %d\n", mesh->vertexBuffer, vertexCount * sizeof(graphics_Vertex));
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(graphics_Vertex), vertices, GL_DYNAMIC_DRAW);
  mesh->vertices = realloc(mesh->vertices, sizeof(graphics_Vertex) * vertexCount);
  memcpy(mesh->vertices, vertices, vertexCount);
  mesh->vertexCount = vertexCount;

  if(!mesh->customIndexBuffer) {
    graphics_Mesh_setVertexMap(mesh, 0, 0);
  }
}


#define makeFillDefaultIndexBufferFunc(type)                             \
  static void fillDefaultIndexBuffer_ ## type(void *out, size_t count) { \
    printf("Creating default index buffer " #type ": %d\n", count); \
    type *t = (type*)out;                                                \
    for(size_t i = 0; i < count; ++i) {                                  \
      t[i] = (type)i;                                                    \
    }                                                                    \
  }
makeFillDefaultIndexBufferFunc(uint8_t)
makeFillDefaultIndexBufferFunc(uint16_t)
makeFillDefaultIndexBufferFunc(uint32_t)
#undef makeFillDefaultIndexBufferFunc


static size_t indexSize(graphics_Mesh const* mesh) {
  if(mesh->vertexCount > 0xFFFF) {
    return sizeof(uint32_t);
  } else if(mesh->vertexCount > 0xFF) {
    return sizeof(uint16_t);
  }
  return sizeof(uint8_t);
}


static void createDefaultIndexBuffer(graphics_Mesh *mesh) {
  mesh->customIndexBuffer = false;

  size_t idxSize = indexSize(mesh);
  mesh->indexBufferSize = idxSize * mesh->vertexCount;
  printf("creating default index buffer, sz=%d\n", mesh->indexBufferSize);
  mesh->indices = realloc(mesh->indices, mesh->indexBufferSize);

  switch(idxSize) {
  case 1:
    fillDefaultIndexBuffer_uint8_t(mesh->indices, mesh->vertexCount);
    break;
  case 2:
    fillDefaultIndexBuffer_uint16_t(mesh->indices, mesh->vertexCount);
    break;
  case 4:
    fillDefaultIndexBuffer_uint32_t(mesh->indices, mesh->vertexCount);
    break;
  }

  printf("Index Buffer: ");
  for(size_t i = 0; i < mesh->indexBufferSize; ++i) {
    printf("%02x", ((char*)mesh->indices)[i]);
  }
  printf("\n");

  mesh->customIndexBuffer = false;
}


#define makeFillCustomIndexBufferFunc(type)                                                      \
  static void fillCustomIndexBuffer_ ## type(void *out, size_t count, uint32_t const* indices) { \
    type *t = (type*)out;                                                                        \
    for(size_t i = 0; i < count; ++i) {                                                          \
      t[i] = (type)i;                                                                          \
    }                                                                                            \
  }
makeFillCustomIndexBufferFunc(uint8_t)
makeFillCustomIndexBufferFunc(uint16_t)
makeFillCustomIndexBufferFunc(uint32_t)
#undef makeFillCustomIndexBufferFunc

static void createCustomIndexBuffer(graphics_Mesh *mesh, size_t count, uint32_t const* indices) {
  mesh->customIndexBuffer = true;

  size_t idxSize = indexSize(mesh);
  mesh->indexBufferSize = idxSize * count;
  mesh->indices = realloc(mesh->indices, mesh->indexBufferSize);

  switch(idxSize) {
  case 1:
    fillCustomIndexBuffer_uint8_t(mesh, count, indices);
    break;
  case 2:
    fillCustomIndexBuffer_uint16_t(mesh, count, indices);
    break;
  case 4:
    fillCustomIndexBuffer_uint32_t(mesh, count, indices);
    break;
  }
}


void graphics_Mesh_setVertexMap(graphics_Mesh *mesh, size_t count, uint32_t const* indices) {
  if(indices == 0) {
    createDefaultIndexBuffer(mesh);
  } else {
    createCustomIndexBuffer(mesh, count, indices);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBufferSize, mesh->indices, GL_DYNAMIC_DRAW);
}

static graphics_Quad const fullQuad = {0.0f, 0.0f, 1.0f, 1.0f};
static GLenum const glTypes[] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, 0, GL_UNSIGNED_INT};
void graphics_Mesh_draw(graphics_Mesh const* mesh, float x, float y, float r, float sx, float sy, float ox, float oy, float kx, float ky) {

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mesh->texture->texID);
  mat4x4 tr2d;
  m4x4_newTransform2d(&tr2d, x, y, r, sx, sy, ox, oy, kx, ky);

  float const color[] = {1.0f, 1.0f, 1.0f, 1.0f};
  size_t idxSize = indexSize(mesh);
  graphics_drawArray(
    &fullQuad,
    &tr2d,
    mesh->vertexArray,
    mesh->indexBuffer,
    mesh->indexBufferSize / idxSize,
    mesh->drawMode,
    glTypes[idxSize-1],
    0, // color,
    1.0f,
    1.0f
  );

}
