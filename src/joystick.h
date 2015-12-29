#pragma once

#include <SDL_joystick.h>
#include <SDL_gamecontroller.h>
#include <stdbool.h>


typedef struct {
  SDL_Joystick *joystick;
  SDL_GameController *controller;
  SDL_JoystickID id;
} joystick_Joystick;

int joystick_getCount(void);
void joystick_init(void);
void joystick_deviceAdded(int index);
void joystick_deviceRemoved(int index);
void joystick_buttonEvent(int index, int button, bool state);
bool joystick_Joystick_isConnected(joystick_Joystick const* joystick);
float joystick_Joystick_getAxis(joystick_Joystick const* joystick, int axis);
int joystick_Joystick_getAxisCount(joystick_Joystick const* joystick);
int joystick_Joystick_getButtonCount(joystick_Joystick const* joystick);
bool joystick_Joystick_isDown(joystick_Joystick const* joystick, int button);



typedef struct {
  void (*deviceAdded)(void *userData, joystick_Joystick *device);
  void (*deviceRemoved)(void *userData, joystick_Joystick *device);
  void *userData;
} joystick_EventCallbacks;


void joystick_setEventCallbacks(joystick_EventCallbacks const *callbacks);
