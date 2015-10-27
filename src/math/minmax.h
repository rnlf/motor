#pragma once

inline int max(int i, int j) {
  return i > j ? i : j;
}

inline int min(int i, int j) {
  return i < j ? i : j;
}

inline int clamp(int v, int low, int high) {
  return min(max(v, low), high);
}
