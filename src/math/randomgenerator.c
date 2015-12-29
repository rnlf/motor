/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "randomgenerator.h"
#include "util.h"
#include <inttypes.h>


static uint64_t wangHash64(uint64_t key) {
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ (key >> 28);
  key = key + (key << 31);
  return key;
}


void math_RandomGenerator_init(math_RandomGenerator *r) {
  math_RandomGenerator_setSeed(r, 0x0139408DCBBF7A44ULL);
  r->lastRandomNormal = INFINITY;
}


void math_RandomGenerator_setSeed(math_RandomGenerator *r, uint64_t s) {
  r->seed = s;
  do {
    s = wangHash64(s);
  } while (s == 0);
  r->state = s;
}


uint64_t math_RandomGenerator_getSeed(math_RandomGenerator *r) {
  return r->seed;
}


uint64_t math_RandomGenerator_getState(math_RandomGenerator const *r) {
  return r->state;
}


void math_RandomGenerator_setState(math_RandomGenerator *r, uint64_t state) {
  r->state = state;
}


uint64_t math_RandomGenerator_rand(math_RandomGenerator *r) {
  r->state ^= (r->state << 13);
  r->state ^= (r->state >> 7);
  r->state ^= (r->state << 17);
  return r->state;
}


double math_RandomGenerator_random(math_RandomGenerator *r) {
  return math_RandomGenerator_rand(r) / ((double) (UINT64_MAX) + 1.0);
}


double math_RandomGenerator_random2(math_RandomGenerator *r, double min, double max) {
  return math_RandomGenerator_random(r) * (max - min) + min;
}


double math_RandomGenerator_randomNormal(math_RandomGenerator *r, double stddev) {

  if (r->lastRandomNormal != INFINITY) {
    double n = r->lastRandomNormal;
    r->lastRandomNormal = INFINITY;
    return n * stddev;
  }

  double n = sqrt(-2.0 * log(1. - math_RandomGenerator_random(r)));
  double phi = 2.0 * PI * (1. - math_RandomGenerator_random(r));
  
  r->lastRandomNormal = n * cos(phi);
  return n * sin(phi) * stddev;
}


double math_RandomGenerator_randomNormal2(math_RandomGenerator *r, double stddev, double mean) {
  return math_RandomGenerator_randomNormal(r, stddev) + mean;
}
