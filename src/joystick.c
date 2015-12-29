#include <assert.h>
#include "joystick.h"
#include <SDL.h>
#include <stdbool.h>


static void dummyAdded(void *ud, joystick_Joystick *js) {}
static void dummyRemoved(void *ud, joystick_Joystick *js) {}


static struct {
  joystick_Joystick *joysticks;
  int joystickCount;
  joystick_EventCallbacks callbacks;
} moduleData = {
  .callbacks = {
    .deviceAdded   = dummyAdded,
    .deviceRemoved = dummyRemoved
  }
};


static void resizeJoystickList(int count) {
  moduleData.joysticks = realloc(moduleData.joysticks, count * sizeof(*moduleData.joysticks));
  moduleData.joystickCount = count;
}


int joystick_getCount(void) {
  return moduleData.joystickCount;
}


static joystick_Joystick* openJoystick(int i) {
  resizeJoystickList(moduleData.joystickCount+1);
  joystick_Joystick *js = moduleData.joysticks + (moduleData.joystickCount - 1);
  js->joystick = SDL_JoystickOpen(i);
  js->id = SDL_JoystickInstanceID(js->joystick);
  if(SDL_IsGameController(i)) {
    js->controller = SDL_GameControllerOpen(i);
  } else {
    js->controller = 0;
  }
  return js;
}


void joystick_deviceAdded(int index) {
  joystick_Joystick *js = openJoystick(index);

  moduleData.callbacks.deviceAdded(moduleData.callbacks.userData, js);
}


static joystick_Joystick* findJoystickBySDLInstanceID(SDL_JoystickID id) {
  for(int i = 0; i < moduleData.joystickCount; ++i) {
    joystick_Joystick *js = moduleData.joysticks + i;
    if(id == js->id) {
      return js;
    }
  }
  return 0;
}


static void closeJoystick(int id) {
  joystick_Joystick *js = findJoystickBySDLInstanceID(id);
  
  if(js == 0) {
    return;
  }

  SDL_JoystickClose(js->joystick);

  *js = moduleData.joysticks[moduleData.joystickCount-1];
  resizeJoystickList(moduleData.joystickCount - 1);
}


void joystick_deviceRemoved(int id) {
  moduleData.callbacks.deviceRemoved(moduleData.callbacks.userData, findJoystickBySDLInstanceID(id));
  closeJoystick(id);
}


void joystick_buttonPressed(int id, int button) {
  moduleData.callbacks.joystickPressed(moduleData.callbacks.userData,
    findJoystickBySDLInstanceID(id), button);
}


void joystick_buttonReleased(int id, int button) {
  moduleData.callbacks.joystickReleased(moduleData.callbacks.userData,
    findJoystickBySDLInstanceID(id), button);
}


void joystick_axisEvent(int id, int axis, int value) {
  moduleData.callbacks.joystickAxis(moduleData.callbacks.userData,
    findJoystickBySDLInstanceID(id), axis, value / 32767.0f);
}


void joystick_init(void) {
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

  int count = SDL_NumJoysticks();

  for(int i = 0; i < count; ++i) {
    openJoystick(i);
  }
}


void joystick_setEventCallbacks(joystick_EventCallbacks const *callbacks) {
  moduleData.callbacks = *callbacks;
}


bool joystick_Joystick_isConnected(joystick_Joystick const* joystick) {
  return SDL_JoystickGetAttached(joystick->joystick);
}


bool joystick_Joystick_isGamepad(joystick_Joystick const* joystick) {
  return joystick->controller != 0;
}


float joystick_Joystick_getAxis(joystick_Joystick const* joystick, int axis) {
  int16_t val = SDL_JoystickGetAxis(joystick->joystick, axis);
  return val / 32767.0f;
}


int joystick_Joystick_getAxisCount(joystick_Joystick const* joystick) {
  return SDL_JoystickNumAxes(joystick->joystick);
}


int joystick_Joystick_getButtonCount(joystick_Joystick const* joystick) {
  return SDL_JoystickNumButtons(joystick->joystick);
}


bool joystick_Joystick_isDown(joystick_Joystick const* joystick, int button) {
  return SDL_JoystickGetButton(joystick->joystick, button);
}


joystick_GUID joystick_Joystick_getGUID(joystick_Joystick const* joystick) {
  SDL_JoystickGUID g = SDL_JoystickGetGUID(joystick->joystick);
  joystick_GUID guid;
  SDL_JoystickGetGUIDString(g, guid.guid, sizeof(guid.guid));
  return guid;
}


int joystick_Joystick_getHatCount(joystick_Joystick const* joystick) {
  return SDL_JoystickNumHats(joystick->joystick);
}


joystick_JoystickHat joystick_Joystick_getHat(joystick_Joystick const* joystick, int hat) {
  return (joystick_JoystickHat) SDL_JoystickGetHat(joystick->joystick, hat);
}


char const * joystick_Joystick_getName(joystick_Joystick const* joystick) {
  // This returns differently formatted strings on all browsers. Hooray for
  // web standardization, eh?
  return SDL_JoystickName(joystick->joystick);
}


int joystick_Joystick_getID(joystick_Joystick const* joystick) {
  return joystick->id;
}


int joystick_Joystick_getInstanceID(joystick_Joystick const* joystick) {
  return SDL_JoystickInstanceID(joystick->joystick);
}


float joystick_Joystick_getGamepadAxis(joystick_Joystick const* joystick, joystick_GamepadAxis axis) {
  return SDL_GameControllerGetAxis(joystick->controller, (SDL_GameControllerAxis) axis) / 32767.0f;
}


bool joystick_Joystick_isGamepadDown(joystick_Joystick const* joystick, joystick_GamepadButton button) {
  return SDL_GameControllerGetButton(joystick->controller, (SDL_GameControllerButton) button);
}


static joystick_GamepadBind joystick_getBind(joystick_GUID guid, char const* button) {
  SDL_JoystickGUID jguid = SDL_JoystickGetGUIDFromString(guid.guid);
  char * mapping = SDL_GameControllerMappingForGUID(jguid);

#ifdef EMSCRIPTEN
  // FIXME: emscripten port SDL2 is broken
  if(!mapping) {
    mapping = SDL_GameControllerMappingForGUID(SDL_JoystickGetGUIDFromString("emscripten"));
  }
#endif

  joystick_GamepadBind bind;

  if(!mapping) {
    bind.mapType = joystick_GamepadMapType_none;
    SDL_free(mapping);
    return bind;
  }

  char search[16] = {',', '\0'};
  
  strcat(search, button);
  strcat(search, ":");

  char const * found = strstr(mapping, search);
  if(!found) {
    bind.mapType = joystick_GamepadMapType_none;
    SDL_free(mapping);
    return bind;
  }
  found += strlen(search);

  switch(found[0]) {
  case 'b':
    bind.mapType = joystick_GamepadMapType_button;
    bind.value.button = atoi(found+1);
    break;
  case 'a':
    bind.mapType = joystick_GamepadMapType_axis;
    bind.value.axis = atoi(found+1);
    break;
  case 'h':
    bind.mapType = joystick_GamepadMapType_hat;
    bind.value.hat.hat = atoi(found+1);
    found = strchr(found, '.');
    bind.value.hat.hatDir = (joystick_JoystickHat)atoi(found);
    break;
  default:
    // Thank you SDL
    assert(false);
  };

  SDL_free(mapping);
  return bind;
}


_Static_assert(SDL_CONTROLLER_AXIS_LEFTX == 0, "Hey, someone changed SDL so that SDL_CONTROLLER_AXIS_LEFTX is not 0 anymore");
_Static_assert(SDL_CONTROLLER_AXIS_TRIGGERRIGHT == 5, "Hey, someone changed SDL so that SDL_CONTROLLER_AXIS_TRIGGERRIGHT is not 5 anymore");
static const char *buttonNames[] = {
  "leftx",
  "lefty",
  "rightx",
  "righty",
  "lefttrigger",
  "righttrigger"
};
joystick_GamepadBind joystick_getButtonBind(joystick_GUID guid, joystick_GamepadButton button) {
  return joystick_getBind(guid, buttonNames[button]);
}


_Static_assert(SDL_CONTROLLER_BUTTON_A == 0, "Hey, someone changed SDL so that SDL_CONTROLLER_BUTTON_A is not 0 anymore");
_Static_assert(SDL_CONTROLLER_BUTTON_DPAD_RIGHT == 14, "Hey, someone changed SDL so that SDL_CONTROLLER_BUTTON_DPAD_RIGHT is not 14 anymore");
static const char *axisNames[] = {
  "a",
  "b",
  "x",
  "y",
  "back",
  "guide",
  "start",
  "leftstick",
  "rightstick",
  "leftshoulder",
  "rightshoulder",
  "dpup",
  "dpdown",
  "dpleft",
  "dpright"
};
joystick_GamepadBind joystick_getAxisBind(joystick_GUID guid, joystick_GamepadAxis axis) {
  return joystick_getBind(guid, axisNames[axis]);
}
