#include <string.h>
#include <stdlib.h>
#include "bezier.h"

void math_BezierCurve_new(math_BezierCurve* curve, float const* vertices, int vertexCount) {
  size_t size = sizeof(float) * vertexCount * 2;
  curve->vertices = malloc(size);
  memcpy(curve->vertices, vertices, size);
  curve->bufferSize = vertexCount;
  curve->vertexCount = vertexCount;
}


void math_BezierCurve_free(math_BezierCurve* curve) {
  free(curve->vertices);
}
