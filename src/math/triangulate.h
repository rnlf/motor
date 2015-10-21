#pragma once

#include <stdbool.h>

void math_triangulation_init();
int math_triangulation_triangulate(float const* verts, int count, int **indices);
bool math_isConvex(float const* verts, int count);
