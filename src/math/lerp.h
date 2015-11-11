#pragma once

inline float lerp(float a, float b, float t) {
  return a + (b-a) * t;
}
