#pragma once

#include <stdbool.h>
#include <AL/al.h>

#include "source.h"
#include "decoder.h"

typedef struct {
  audio_SourceCommon common;
  audio_StreamSourceDecoder const* decoder;
  void*  decoderData;
  ALuint buffers[2];
  bool   looping;
  char  *filename;
} audio_StreamSource;

bool audio_loadStream(audio_StreamSource *source, char const * filename);
void audio_StreamSource_free(audio_StreamSource *source);
void audio_StreamSource_play(audio_StreamSource *source);
void audio_StreamSource_setLooping(audio_StreamSource *source, bool loop);
bool audio_StreamSource_isLooping(audio_StreamSource const* source);
void audio_updateStreams(void);
void audio_streamsource_init(void);
void audio_StreamSource_stop(audio_StreamSource *source);
void audio_StreamSource_rewind(audio_StreamSource *source);
void audio_StreamSource_pause(audio_StreamSource *source);
void audio_StreamSource_resume(audio_StreamSource *source);
void audio_StreamSource_clone(audio_StreamSource const* oldSrc, audio_StreamSource * newSrc);
