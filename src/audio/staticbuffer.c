/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <stdlib.h>
#include "staticbuffer.h"

audio_StaticBuffer * audio_StaticBuffer_new() {
  audio_StaticBuffer * buf = malloc(sizeof(audio_StaticBuffer));
  alGenBuffers(1, &buf->buffer);
  buf->refCount = 1;
  return buf;
}


void audio_StaticBuffer_free(audio_StaticBuffer *buffer) {
  alDeleteBuffers(1, &buffer->buffer);
  free(buffer);
}


void audio_StaticBuffer_ref(audio_StaticBuffer *buffer) {
  ++buffer->refCount;
}


void audio_StaticBuffer_unref(audio_StaticBuffer *buffer) {
  if(--buffer->refCount == 0) {
    free(buffer);
  }
}
