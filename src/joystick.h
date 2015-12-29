#pragma once

#include <SDL_joystick.h>
#include <SDL_gamecontroller.h>
#include <stdbool.h>


typedef struct {
  SDL_Joystick *joystick;
  SDL_GameController *controller;
  SDL_JoystickID id;
} joystick_Joystick;


typedef struct {
  char guid[33];
} joystick_GUID;


typedef enum {
  joystick_JoystickHat_centered   = SDL_HAT_CENTERED,
  joystick_JoystickHat_down       = SDL_HAT_DOWN,
  joystick_JoystickHat_left       = SDL_HAT_LEFT,
  joystick_JoystickHat_left_down  = SDL_HAT_LEFTDOWN,
  joystick_JoystickHat_left_up    = SDL_HAT_LEFTUP,
  joystick_JoystickHat_right      = SDL_HAT_RIGHT,
  joystick_JoystickHat_right_down = SDL_HAT_RIGHTDOWN,
  joystick_JoystickHat_right_up   = SDL_HAT_RIGHTUP,
  joystick_JoystickHat_up         = SDL_HAT_UP
} joystick_JoystickHat;


int joystick_getCount(void);
void joystick_init(void);
void joystick_deviceAdded(int index);
void joystick_deviceRemoved(int index);
void joystick_buttonPressed(int id, int button);
void joystick_buttonReleased(int id, int button);
void joystick_axisEvent(int id, int axis, int value);
bool joystick_Joystick_isConnected(joystick_Joystick const* joystick);
float joystick_Joystick_getAxis(joystick_Joystick const* joystick, int axis);
int joystick_Joystick_getAxisCount(joystick_Joystick const* joystick);
int joystick_Joystick_getButtonCount(joystick_Joystick const* joystick);
bool joystick_Joystick_isDown(joystick_Joystick const* joystick, int button);
joystick_GUID joystick_Joystick_getGUID(joystick_Joystick const* joystick);
int joystick_Joystick_getHatCount(joystick_Joystick const* joystick);
joystick_JoystickHat joystick_Joystick_getHat(joystick_Joystick const* joystick, int hat);


typedef struct {
  void (*deviceAdded)(void *userData, joystick_Joystick *device);
  void (*deviceRemoved)(void *userData, joystick_Joystick *device);
  void (*joystickAxis)(void *userData, joystick_Joystick *device, int axis, float value);
  void (*joystickPressed)(void *userData, joystick_Joystick *device, int button);
  void (*joystickReleased)(void *userData, joystick_Joystick *device, int button);
  void *userData;
} joystick_EventCallbacks;


void joystick_setEventCallbacks(joystick_EventCallbacks const *callbacks);
