#include "joystick.h"
#include <SDL.h>


void joystick_init(void) {
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
}


int joystick_getCount(void) {
  return SDL_NumJoysticks();
}
