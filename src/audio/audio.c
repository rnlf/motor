/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "audio.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>
#include "streamsource.h"


static struct {
  ALCdevice  *device;
  ALCcontext *context;
} moduleData;


void audio_init(void) {
  moduleData.device = alcOpenDevice(0);
  moduleData.context = alcCreateContext(moduleData.device, 0);
  if(!alcMakeContextCurrent(moduleData.context)) {
    printf("Failed to initialite audio context\n");
  }

  audio_streamsource_init();
}
