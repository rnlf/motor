/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

typedef struct {
  int width;
  int height;
} motor_WindowConfig;

typedef struct {
  char const* identity;
  motor_WindowConfig window;
} motor_Config;
