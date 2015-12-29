/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

typedef struct {
  float x;
  float y;
  float z;
  float w;
} vec4;


typedef struct {
  float m[4][4];
} mat4x4;


typedef struct {
  float x;
  float y;
  float z;
} vec3;

typedef struct {
  float x;
  float y;
} vec2;

typedef struct {
  float m[3][3];
} mat3x3;

void m4x4_newIdentity(mat4x4 *out);
void m4x4_newScaling(mat4x4 *out, float x, float y, float z);
void m4x4_newTranslation(mat4x4 *out, float x, float y, float z);
void m4x4_newRotationX(mat4x4 *out, float a);
void m4x4_newRotationY(mat4x4 *out, float a);
void m4x4_newRotationZ(mat4x4 *out, float a);
void m4x4_newTransform2d(mat4x4 *out, float x, float y, float r, float sx, float sy,
                      float ox, float oy, float kx, float ky);
void m4x4_mulM4x4(mat4x4 *out, mat4x4 const* a, mat4x4 const* b);
void m4x4_mulV4(vec4 *out, mat4x4 const* m, vec4 const* v);
void m4x4_scale(mat4x4 *inout, float x, float y, float z);
void m4x4_translate(mat4x4 *inout, float x, float y, float z);
void m4x4_rotateZ(mat4x4 *inout, float a);
void m4x4_shear2d(mat4x4 *inout, float x, float y);

void m3x3_newTransform2d(mat3x3 *out, float x, float y, float r, float sx, float sy,
                      float ox, float oy, float kx, float ky, float w, float h);

// Assumes that v is a 2d point (homogeneous coord == 1).
// This allows much faster multiplication
void m3x3_mulV2(vec2 *out, mat3x3 const* m, vec2 const* v);
