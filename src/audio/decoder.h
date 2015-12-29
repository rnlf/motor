/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <AL/al.h>

typedef struct {
  bool (*testFile)(char const *filename);
  bool (*loadFile)(ALuint buffer, char const* file);
} audio_StaticSourceDecoder;

/*
  preloadSamples must return:  0 if end of stream reached
                              -1 if buffer is full
                               n where n > 0 if n samples were preloaded
*/

typedef struct {
  bool (*testFile)(char const *filename);
  bool (*openFile)(char const* filename, void **decoderData);
  int  (*getChannelCount)(void *decoderData);
  int  (*getSampleRate)(void *decoderData);
  bool (*closeFile)(void **decoderData);
  bool (*atEnd)(void const *decoderData);
  void (*rewind)(void *decoderData);
  int  (*preloadSamples)(void *decoderData, int sampleCount);
  int  (*uploadPreloadedSamples)(void *decoderData, ALuint buffer);
  void (*flush)(void *decoderData);
} audio_StreamSourceDecoder;
