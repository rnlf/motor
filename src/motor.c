/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "motor.h"

motor_Version const * motor_getVersion(void) {
  static const motor_Version version = {
    .major = 0,
    .minor = 0,
    .revision = 3,
    .codename = "Rotten Rodent"
  };

  return &version;
}
