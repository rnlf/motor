/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <tgmath.h>
#include "gamma.h"

float math_linearToGamma(float c) {
  if (c > 1.0f)
    return 1.0f;
  else if (c < 0.0f)
    return 0.0f;
  else if (c < 0.0031308f)
    return c * 12.92f;
  else
    return 1.055f * pow(c, 0.41666f) - 0.055f;

}


float math_gammaToLinear(float c) {
  if (c > 1.0f)
    return 1.0f;
  else if (c < 0.0f)
    return 0.0f;
  else if (c <= 0.04045)
    return c / 12.92f;
  else
    return pow((c + 0.055f) / 1.055f, 2.4f);
}

