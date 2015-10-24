#include <tgmath.h>
#include <string.h>
#include <stdlib.h>
#include "bezier.h"


void math_BezierCurve_new(math_BezierCurve* curve, float const* vertices, int vertexCount) {
  size_t size = sizeof(float) * (vertexCount > 0 ? vertexCount : 1) * 2;
  curve->vertices = malloc(size);
  if(vertices && vertexCount > 0) {
    memcpy(curve->vertices, vertices, size);
  }
  curve->bufferSize = vertexCount > 0 ? vertexCount : 1;
  curve->vertexCount = vertexCount;
}


void math_BezierCurve_free(math_BezierCurve* curve) {
  free(curve->vertices);
}


void math_BezierCurve_evaluate(math_BezierCurve const* curve, float t, float *outX, float *outY) {
  float temp[curve->vertexCount*2];

  memcpy(temp, curve->vertices, curve->vertexCount * sizeof(float) * 2);
  for(int step = 1; step < curve->vertexCount; ++step) {
    for(int i = 0; i < curve->vertexCount - step; ++i) {
      temp[2*i  ] = temp[2*i]   * (1.0f - t) + temp[2*(i+1)  ] * t;
      temp[2*i+1] = temp[2*i+1] * (1.0f - t) + temp[2*(i+1)+1] * t;
    }
  }

  *outX = temp[0];
  *outY = temp[1];
}


float const* math_BezierCurve_getControlPoint(math_BezierCurve const* curve, int i) {
  return curve->vertices + 2*i;
}


int math_BezierCurve_getControlPointCount(math_BezierCurve const* curve) {
  return curve->vertexCount;
}


int math_BezierCurve_getDegree(math_BezierCurve const* curve) {
  return curve->vertexCount - 1;
}


void math_BezierCurve_getDerivative(math_BezierCurve const* curve, math_BezierCurve *derivative) {
  int sc = curve->vertexCount-1;
  math_BezierCurve_new(derivative, 0, sc);

  float degree = sc;

  for(int i = 0; i < sc; ++i) {
    derivative->vertices[2*i  ] = (curve->vertices[2*(i+1)  ] - curve->vertices[2*i]  ) * degree;
    derivative->vertices[2*i+1] = (curve->vertices[2*(i+1)+1] - curve->vertices[2*i+1]) * degree;
  }
}


void math_BezierCurve_insertControlPoint(math_BezierCurve *curve, int i, float x, float y) {
  size_t tailSize = (curve->vertexCount - i) * 2 * sizeof(float);
  if(curve->bufferSize == curve->vertexCount) {
    // Like realloc, but leave insert position free and move all above that one position up
    float* newBuf = malloc(sizeof(float) * 4 * curve->bufferSize);
    memcpy(newBuf, curve->vertices, sizeof(float) * 2 * i);
    memcpy(newBuf + 2*(i+1), curve->vertices+2*i, tailSize);
    free(curve->vertices);
    curve->vertices = newBuf;
    curve->bufferSize *= 2;
  } else {
    // Move last count - i vertices up one position
    memmove(
      curve->vertices+2*(i+1),
      curve->vertices+2*i,
      tailSize
    );
  }

  curve->vertices[2*i]   = x;
  curve->vertices[2*i+1] = y;
  ++curve->vertexCount;
}


static size_t subdivide(float **pcoords, size_t size, int depth) {
  // straight C port from the LOVE version

  float* coords = *pcoords;
  
  if(depth <= 0) {
    return size;
  }

  float *left  = malloc(sizeof(float) * size * 2);
  float *right = malloc(sizeof(float) * size * 2);

  for(size_t step = 1; step < size; ++step) {
    left [(step-1)*2  ] = coords[0];
    left [(step-1)*2+1] = coords[1];
    right[(step-1)*2  ] = coords[2*(size-step)  ];
    right[(step-1)*2+1] = coords[2*(size-step)+1];

    for(size_t i = 0; i < size - step; ++i) {
      coords[2*i  ] = (coords[2*i  ] + coords[2*(i+1)  ]) * 0.5f;
      coords[2*i+1] = (coords[2*i+1] + coords[2*(i+1)+1]) * 0.5f;
    }
  }

  left [2*(size-1)  ] = coords[0];
  left [2*(size-1)+1] = coords[1];
  right[2*(size-1)  ] = coords[0];
  right[2*(size-1)+1] = coords[1];

  size_t countLeft  = subdivide(&left,  size, depth-1);
  size_t countRight = subdivide(&right, size, depth-1);
  size_t count      = countLeft + countRight - 1;

  coords = realloc(coords, sizeof(float) * (2*count-1) * 2);

  memcpy(coords, left, sizeof(float) * countLeft * 2);
  for(size_t i = 1; i < countRight; ++i) {
    coords[2*(countLeft+i-1)  ] = right[2*(countLeft - i -1)  ];
    coords[2*(countLeft+i-1)+1] = right[2*(countLeft - i -1)+1];
  }

  free(left);
  free(right);

  *pcoords = coords;

  return count;
}


float const* math_BezierCurve_render(math_BezierCurve const* curve, int depth, size_t *count) {
  float *coords = malloc(sizeof(float) * 2 * curve->vertexCount);
  memcpy(coords, curve->vertices, sizeof(float) * 2 * curve->vertexCount);

  *count = subdivide(&coords, curve->vertexCount, depth);
  return coords;
}


void math_BezierCurve_rotate(math_BezierCurve *curve, float angle, float ox, float oy) {
  for(int i = 0; i < curve->vertexCount; ++i) {
    float x = curve->vertices[2*i  ] - ox;
    float y = curve->vertices[2*i+1] - oy;

    float ca = cos(angle);
    float sa = sin(angle);

    curve->vertices[2*i  ] = ca * x - sa * y + ox;
    curve->vertices[2*i+1] = sa * x + ca * y + oy;
  }
}


void math_BezierCurve_scale(math_BezierCurve *curve, float sx, float sy, float ox, float oy) {
  for(int i = 0; i < curve->vertexCount; ++i) {
    float dx = curve->vertices[2*i  ] - ox;
    float dy = curve->vertices[2*i+1] - oy;
    curve->vertices[2*i  ] = ox + sx * dx;
    curve->vertices[2*i+1] = oy + sy * dy;
  }
}


void math_BezierCurve_setControlPoint(math_BezierCurve *curve, int i, float x, float y) {
  curve->vertices[2*i] = x;
  curve->vertices[2*i+1] = y;
}


void math_BezierCurve_translate(math_BezierCurve *curve, float x, float y) {
  for(int i = 0; i < curve->vertexCount; ++i) {
    curve->vertices[2*i  ] += x;
    curve->vertices[2*i+1] += y;
  }
}


