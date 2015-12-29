/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <AL/al.h>

typedef struct {
  ALuint buffer;
  int refCount;
} audio_StaticBuffer;


audio_StaticBuffer * audio_StaticBuffer_new();
void audio_StaticBuffer_free(audio_StaticBuffer *buffer);
void audio_StaticBuffer_ref(audio_StaticBuffer *buffer);
void audio_StaticBuffer_unref(audio_StaticBuffer *buffer);
