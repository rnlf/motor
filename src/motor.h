/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

typedef struct {
  int major;
  int minor;
  int revision;
  char const * codename;
} motor_Version;

motor_Version const * motor_getVersion(void);
