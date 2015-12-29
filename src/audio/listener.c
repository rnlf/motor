/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "listener.h"
#include <AL/al.h>

void audio_setVolume(double gain) {
  alListenerf(AL_GAIN, gain);
}
