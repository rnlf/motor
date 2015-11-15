#pragma once

#include <SDL_joystick.h>
#include <stdbool.h>


typedef struct {
  SDL_Joystick *joystick;
  SDL_JoystickID id;
} joystick_Joystick;

int joystick_getCount(void);
void joystick_init(void);
void joystick_deviceAdded(int index);
void joystick_deviceRemoved(int index);
void joystick_buttonEvent(int index, int button, bool state);


typedef struct {
  void (*deviceAdded)(void *userData, joystick_Joystick *device);
  void (*deviceRemoved)(void *userData, joystick_Joystick *device);
  void *userData;
} joystick_EventCallbacks;


void joystick_setEventCallbacks(joystick_EventCallbacks const *callbacks);
