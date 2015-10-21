#pragma once

typedef struct {
  float *vertices;
  int  bufferSize;
  int  vertexCount;
} math_BezierCurve;


void math_BezierCurve_new(math_BezierCurve* curve, float const* vertices, int vertexCount);
void math_BezierCurve_free(math_BezierCurve* curve);

