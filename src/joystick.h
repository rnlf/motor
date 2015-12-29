/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

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


typedef enum {
  joystick_GamepadAxis_left_x        = SDL_CONTROLLER_AXIS_LEFTX,
  joystick_GamepadAxis_left_y        = SDL_CONTROLLER_AXIS_LEFTY,
  joystick_GamepadAxis_right_x       = SDL_CONTROLLER_AXIS_RIGHTX,
  joystick_GamepadAxis_right_y       = SDL_CONTROLLER_AXIS_RIGHTY,
  joystick_GamepadAxis_left_trigger  = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
  joystick_GamepadAxis_right_trigger = SDL_CONTROLLER_AXIS_TRIGGERRIGHT
} joystick_GamepadAxis;


typedef enum {
  joystick_GamepadButton_a              = SDL_CONTROLLER_BUTTON_A,
  joystick_GamepadButton_b              = SDL_CONTROLLER_BUTTON_B,
  joystick_GamepadButton_x              = SDL_CONTROLLER_BUTTON_X,
  joystick_GamepadButton_y              = SDL_CONTROLLER_BUTTON_Y,
  joystick_GamepadButton_back           = SDL_CONTROLLER_BUTTON_BACK,
  joystick_GamepadButton_guide          = SDL_CONTROLLER_BUTTON_GUIDE,
  joystick_GamepadButton_start          = SDL_CONTROLLER_BUTTON_START,
  joystick_GamepadButton_left_stick     = SDL_CONTROLLER_BUTTON_LEFTSTICK,
  joystick_GamepadButton_right_stick    = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
  joystick_GamepadButton_left_shoulder  = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
  joystick_GamepadButton_right_shoulder = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
  joystick_GamepadButton_dpad_up        = SDL_CONTROLLER_BUTTON_DPAD_UP,
  joystick_GamepadButton_dpad_down      = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
  joystick_GamepadButton_dpag_left      = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
  joystick_GamepadButton_dpad_right     = SDL_CONTROLLER_BUTTON_DPAD_RIGHT
} joystick_GamepadButton;


typedef enum {
  joystick_GamepadMapType_none   = SDL_CONTROLLER_BINDTYPE_NONE,
  joystick_GamepadMapType_button = SDL_CONTROLLER_BINDTYPE_BUTTON,
  joystick_GamepadMapType_axis   = SDL_CONTROLLER_BINDTYPE_AXIS,
  joystick_GamepadMapType_hat    = SDL_CONTROLLER_BINDTYPE_HAT
} joystick_GamepadMapType;


typedef struct {
  joystick_GamepadMapType mapType;
  union {
    int button;
    int axis;
    struct {
      int hat;
      joystick_JoystickHat hatDir;
    } hat;
  } value;
} joystick_GamepadBind;

int joystick_getCount(void);
void joystick_init(void);
void joystick_deviceAdded(int index);
void joystick_deviceRemoved(int index);
void joystick_buttonPressed(int id, int button);
void joystick_buttonReleased(int id, int button);
void joystick_axisEvent(int id, int axis, int value);
bool joystick_Joystick_isConnected(joystick_Joystick const* joystick);
bool joystick_Joystick_isGamepad(joystick_Joystick const* joystick);
float joystick_Joystick_getAxis(joystick_Joystick const* joystick, int axis);
int joystick_Joystick_getAxisCount(joystick_Joystick const* joystick);
int joystick_Joystick_getButtonCount(joystick_Joystick const* joystick);
bool joystick_Joystick_isDown(joystick_Joystick const* joystick, int button);
joystick_GUID joystick_Joystick_getGUID(joystick_Joystick const* joystick);
int joystick_Joystick_getHatCount(joystick_Joystick const* joystick);
joystick_JoystickHat joystick_Joystick_getHat(joystick_Joystick const* joystick, int hat);
char const * joystick_Joystick_getName(joystick_Joystick const* joystick);
int joystick_Joystick_getID(joystick_Joystick const* joystick);
int joystick_Joystick_getInstanceID(joystick_Joystick const* joystick);
float joystick_Joystick_getGamepadAxis(joystick_Joystick const* joystick, joystick_GamepadAxis axis);
bool joystick_Joystick_isGamepadDown(joystick_Joystick const* joystick, joystick_GamepadButton button);
joystick_GamepadBind joystick_getButtonBind(joystick_GUID guid, joystick_GamepadButton axis);
joystick_GamepadBind joystick_getAxisBind(joystick_GUID guid, joystick_GamepadAxis axis);


typedef struct {
  void (*deviceAdded)(void *userData, joystick_Joystick *device);
  void (*deviceRemoved)(void *userData, joystick_Joystick *device);
  void (*joystickAxis)(void *userData, joystick_Joystick *device, int axis, float value);
  void (*joystickPressed)(void *userData, joystick_Joystick *device, int button);
  void (*joystickReleased)(void *userData, joystick_Joystick *device, int button);
  void *userData;
} joystick_EventCallbacks;


void joystick_setEventCallbacks(joystick_EventCallbacks const *callbacks);
