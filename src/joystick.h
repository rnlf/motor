#pragma once

#include <SDL_joystick.h>

typedef struct {
  SDL_Joystick *joystick;
} joystick_Joystick;

int joystick_getCount(void);
void joystick_init(void);

