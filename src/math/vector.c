#include "vector.h"
#include <tgmath.h>


void m4x4_scale(mat4x4 *inout, float x, float y, float z) {
  for(int i = 0; i < 3; ++i) {
    inout->m[0][i] *= x;
    inout->m[1][i] *= y;
    inout->m[2][i] *= z;
  }
}

void m4x4_translate(mat4x4 *inout, float x, float y, float z) {
  inout->m[3][0] += x * inout->m[0][0] + y * inout->m[1][0] + z * inout->m[2][0];
  inout->m[3][1] += x * inout->m[0][1] + y * inout->m[1][1] + z * inout->m[2][1];
  inout->m[3][2] += x * inout->m[0][2] + y * inout->m[1][2] + z * inout->m[2][2];
}

void m4x4_shear2d(mat4x4 *inout, float x, float y) {
  inout->m[0][0] += x * inout->m[1][0];
  inout->m[0][1] += x * inout->m[1][1];
  inout->m[0][2] += x * inout->m[1][2];
  inout->m[1][0] += y * inout->m[0][0];
  inout->m[1][1] += y * inout->m[0][1];
  inout->m[1][2] += y * inout->m[0][2];
}

void m4x4_newTransform2d(mat4x4 *out, float x, float y, float r, float sx, float sy,
                      float ox, float oy, float kx, float ky) {

  float sa = sin(r);
  float ca = cos(r);
  float e = ky * sy;
  float f = kx * sx;
  float g = -ky*ox-oy;
  float j = g * sy;
  float k = -kx*oy-ox;

  out->m[0][0] = ca*sx-sa*e;
  out->m[0][1] = sa*sx+ca*e;
  out->m[0][2] = 0.0f;
  out->m[0][3] = 0.0f;

  out->m[1][0] = ca*f-sa*sy;
  out->m[1][1] = ca*sy+sa*f;
  out->m[1][2] = 0.0f;
  out->m[1][3] = 0.0f;

  out->m[2][0] = 0.0f;
  out->m[2][1] = 0.0f;
  out->m[2][2] = 1.0f;
  out->m[2][3] = 0.0f;

  out->m[3][0] = x+ca*k*sx-j*sa;
  out->m[3][1] = y+k*sa*sx+ca*j;
  out->m[3][2] = 0.0f;
  out->m[3][3] = 1.0f;
}

void m3x3_newTransform2d(mat3x3 *out, float x, float y, float r, float sx, float sy,
                      float ox, float oy, float kx, float ky, float w, float h) {

  float sa = sin(r);
  float ca = cos(r);
  float a = h * sa;
  float b = h * ca;
  float c = w * sa;
  float d = w * ca;
  float e = ky * sy;
  float f = kx * sx;
  float g = -ky*ox-oy;
  float j = g * sy;
  float k = -kx*oy-ox;

  out->m[0][0] = d*sx-c*e;
  out->m[0][1] = c*sx+d*e;
  out->m[0][2] = 0.0f;

  out->m[1][0] = b*f-a*sy;
  out->m[1][1] = b*sy+a*f;
  out->m[1][2] = 0.0f;

  out->m[2][0] = x+ca*k*sx-j*sa;
  out->m[2][1] = y+k*sa*sx+ca*j;
  out->m[2][2] = 1.0f;
}

void m4x4_rotateZ(mat4x4 *inout, float a) {
  float ca = cos(a);
  float sa = sin(a);
  float m00 = inout->m[0][0];
  float m01 = inout->m[0][1];
  float m10 = inout->m[1][0];
  float m11 = inout->m[1][1];
  float m02 = inout->m[0][2];
  float m12 = inout->m[1][2];

  inout->m[0][0] =  ca * m00 + sa * m10;
  inout->m[0][1] =  ca * m01 + sa * m11;
  inout->m[1][0] = -sa * m00 + ca * m10;
  inout->m[1][1] = -sa * m01 + ca * m11;
  inout->m[0][2] =  ca * m02 + sa * m12;
  inout->m[1][2] = -sa * m02 + ca * m12;
}

void m4x4_newIdentity(mat4x4 *out) {
  out->m[0][0] = 1.0f;
  out->m[0][1] = 0.0f;
  out->m[0][2] = 0.0f;
  out->m[0][3] = 0.0f;
  out->m[1][0] = 0.0f;
  out->m[1][1] = 1.0f;
  out->m[1][2] = 0.0f;
  out->m[1][3] = 0.0f;
  out->m[2][0] = 0.0f;
  out->m[2][1] = 0.0f;
  out->m[2][2] = 1.0f;
  out->m[2][3] = 0.0f;
  out->m[3][0] = 0.0f;
  out->m[3][1] = 0.0f;
  out->m[3][2] = 0.0f;
  out->m[3][3] = 1.0f;
}

void m4x4_newScaling(mat4x4 *out, float x, float y, float z) {
  out->m[0][0] = x;
  out->m[0][1] = 0.0f;
  out->m[0][2] = 0.0f;
  out->m[0][3] = 0.0f;
  out->m[1][0] = 0.0f;
  out->m[1][1] = y;
  out->m[1][2] = 0.0f;
  out->m[1][3] = 0.0f;
  out->m[2][0] = 0.0f;
  out->m[2][1] = 0.0f;
  out->m[2][2] = z;
  out->m[2][3] = 0.0f;
  out->m[3][0] = 0.0f;
  out->m[3][1] = 0.0f;
  out->m[3][2] = 0.0f;
  out->m[3][3] = 1.0f;
}

void m4x4_newTranslation(mat4x4 *out, float x, float y, float z) {
  out->m[0][0] = 1.0f;
  out->m[0][1] = 0.0f;
  out->m[0][2] = 0.0f;
  out->m[0][3] = 0.0f;
  out->m[1][0] = 0.0f;
  out->m[1][1] = 1.0f;
  out->m[1][2] = 0.0f;
  out->m[1][3] = 0.0f;
  out->m[2][0] = 0.0f;
  out->m[2][1] = 0.0f;
  out->m[2][2] = 1.0f;
  out->m[2][3] = 0.0f;
  out->m[3][0] = x;
  out->m[3][1] = y;
  out->m[3][2] = z;
  out->m[3][3] = 1.0f;
}

void m4x4_newRotationX(mat4x4 *out, float a) {
  // TODO
}

void m4x4_newRotationY(mat4x4 *out, float a) {
  // TODO
}

void m4x4_newRotationZ(mat4x4 *out, float a) {
  float ca = cos(a);
  float sa = sin(a);

  out->m[0][0] = ca;
  out->m[0][1] = sa;
  out->m[0][2] = 0.0f;
  out->m[0][3] = 0.0f;
  out->m[1][0] = -sa;
  out->m[1][1] = ca;
  out->m[1][2] = 0.0f;
  out->m[1][3] = 0.0f;
  out->m[2][0] = 0.0f;
  out->m[2][1] = 0.0f;
  out->m[2][2] = 1.0f;
  out->m[2][3] = 0.0f;
  out->m[3][0] = 0.0f;
  out->m[3][1] = 0.0f;
  out->m[3][2] = 0.0f;
  out->m[3][3] = 1.0f;
}

void m4x4_mulM4x4(mat4x4 *out, mat4x4 const* a, mat4x4 const* b) {
  // TODO pretty naive approach. Someday I'll have to check whether
  // a more cache friendly version is required
  for(int i = 0; i < 4; ++i) {
    for(int j = 0; j < 4; ++j) {
      out->m[i][j] = 0.0f;
      for(int k = 0; k < 4; ++k) {
        out->m[i][j] += a->m[i][k] * b->m[k][j];
      }
    }
  }
}

void m4x4_mulV4(vec4 *out, mat4x4 const* m, vec4 const* v) {
  // TODO
}

void m3x3_mulV2(vec2 *out, mat3x3 const* m, vec2 const* v) {
  out->x = m->m[0][0] * v->x + m->m[1][0] * v->y + m->m[2][0];
  out->y = m->m[0][1] * v->x + m->m[1][1] * v->y + m->m[2][1];
}
