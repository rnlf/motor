#include "mesh.h"

void graphics_Mesh_new(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices, graphics_Image const* texture, graphics_MeshDrawMode mode) {
  mesh->texture = texture;
  mesh->drawMode = mode;

  glGenVertexArrays(1, &mesh->vertexArray);
  glBindVertexArray(mesh->vertexArray);
  glGenBuffers(1, &mesh->vertexBuffer);
  graphics_Mesh_setVertices(mesh, vertexCount, vertices);

  mesh->indexBuffer = 0;
}


void graphics_Mesh_free(graphics_Mesh *mesh) {
  glDeleteBuffers(1,      &mesh->vertexBuffer);
  glDeleteBuffers(1,      &mesh->indexBuffer);
  glDeleteVertexArrays(1, &mesh->vertexArray);
}


void graphics_Mesh_setVertices(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices) {
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(graphics_Vertex), vertices, GL_DYNAMIC_DRAW);
  mesh->vertexCount = vertexCount;
}
