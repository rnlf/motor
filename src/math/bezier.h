#pragma once

typedef struct {
  float  *vertices;
  size_t  bufferSize;
  size_t  vertexCount;
} math_BezierCurve;


void math_BezierCurve_new(math_BezierCurve* curve, float const* vertices, int vertexCount);
void math_BezierCurve_free(math_BezierCurve* curve);
void math_BezierCurve_evaluate(math_BezierCurve const* curve, float t, float *outX, float *outY);
float const* math_BezierCurve_getControlPoint(math_BezierCurve const* curve, int i);
int math_BezierCurve_getControlPointCount(math_BezierCurve const* curve);
int math_BezierCurve_getDegree(math_BezierCurve const* curve);
void math_BezierCurve_getDerivative(math_BezierCurve const* curve, math_BezierCurve *derivative);
void math_BezierCurve_insertControlPoint(math_BezierCurve *curve, int i, float x, float y);
float const* math_BezierCurve_render(math_BezierCurve const* curve, int depth, size_t *count);
void math_BezierCurve_rotate(math_BezierCurve *curve, float angle, float ox, float oy);
void math_BezierCurve_scale(math_BezierCurve *curve, float sx, float sy, float ox, float oy);
void math_BezierCurve_setControlPoint(math_BezierCurve *curve, int i, float x, float y);
void math_BezierCurve_translate(math_BezierCurve *curve, float x, float y);

