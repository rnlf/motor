#pragma once

typedef struct {
  int width;
  int height;
} motor_WindowConfig;

typedef struct {
  char const* identity;
  motor_WindowConfig window;
} motor_Config;
