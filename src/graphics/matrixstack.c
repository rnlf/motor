#include "../math/vector.h"
#include "matrixstack.h"
#include <string.h>

const static int stackSize = 32;

static struct {
  int head;
  mat4x4 stack[stackSize];
} moduleData;

inline mat4x4* matrixstack_head() {
  return &moduleData.stack[moduleData.head];
}

void matrixstack_init() {
  moduleData.head = 0;
  m4x4_new_identity(matrixstack_head());
}

int matrixstack_push() {
  if(moduleData.head == stackSize - 1) {
    return 1;
  }

  memcpy(matrixstack_head() + 1, matrixstack_head(), sizeof(mat4x4));
  ++moduleData.head;
  return 0;
}

int matrixstack_pop() {
  if(moduleData.head == 0) {
    return 1;
  }

  --moduleData.head;
  return 0;
}

void matrixstack_translate(float x, float y) {
  m4x4_translate(matrixstack_head(), x, y, 0.0f);
}

void matrixstack_scale(float x, float y) {
  m4x4_scale(matrixstack_head(), x, y, 0.0f);
}

void matrixstack_origin() {
  m4x4_new_identity(matrixstack_head());
}

void matrixstack_rotate(float a) {
  m4x4_rotate_z(matrixstack_head(), a);
}