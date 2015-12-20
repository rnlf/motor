#include "listener.h"
#include <AL/al.h>

void audio_setVolume(double gain) {
  alListenerf(AL_GAIN, gain);
}
