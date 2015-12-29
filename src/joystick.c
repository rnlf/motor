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


void joystick_buttonEvent(int index, int button, bool state) {
  printf("Button: %d, %d, %d\n", index, button, state);
}


void joystick_init(void) {
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);

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
