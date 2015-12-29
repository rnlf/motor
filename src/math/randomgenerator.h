/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <stdint.h>

#define RANDOMGENERATOR_STATE_STR_SZ (19)


typedef struct {
  uint64_t seed;
  uint64_t state;
  double lastRandomNormal;
} math_RandomGenerator;


void math_RandomGenerator_init(math_RandomGenerator *r);
void math_RandomGenerator_setSeed(math_RandomGenerator *r, uint64_t s);
uint64_t math_RandomGenerator_getSeed(math_RandomGenerator *r);
uint64_t math_RandomGenerator_getState(math_RandomGenerator const *r);
void math_RandomGenerator_setState(math_RandomGenerator *r, uint64_t state);
uint64_t math_RandomGenerator_rand(math_RandomGenerator *r);
double math_RandomGenerator_random(math_RandomGenerator *r);
double math_RandomGenerator_random2(math_RandomGenerator *r, double min, double max);
double math_RandomGenerator_randomNormal(math_RandomGenerator *r, double stddev);
double math_RandomGenerator_randomNormal2(math_RandomGenerator *r, double stddev, double mean);

