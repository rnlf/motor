/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "source.h"

void audio_SourceCommon_init(audio_SourceCommon *source) {
  alGenSources(1, &source->source);

  alSourcef(source->source, AL_PITCH, 1);
  alSourcef(source->source, AL_GAIN, 1);
  alSource3f(source->source, AL_POSITION, 0, 0, 0);
  alSource3f(source->source, AL_VELOCITY, 0, 0, 0);
  alSourcei(source->source, AL_LOOPING, AL_FALSE);

  source->state = audio_SourceState_stopped;
}


void audio_SourceCommon_free(audio_SourceCommon *source) {
  alDeleteSources(1, &source->source);
}


void audio_SourceCommon_play(audio_SourceCommon *source) {
  alSourcePlay(source->source);
  source->state = audio_SourceState_playing;
}


void audio_SourceCommon_stop(audio_SourceCommon *source) {
  alSourceStop(source->source);
  source->state = audio_SourceState_stopped;
}

audio_SourceState audio_SourceCommon_getState(audio_SourceCommon const *source) {
  ALint state;
  alGetSourcei(source->source, AL_SOURCE_STATE, &state);
  return state;
}


void audio_SourceCommon_pause(audio_SourceCommon *source) {
  if(source->state == audio_SourceState_playing) {
    alSourcePause(source->source);
    source->state = audio_SourceState_paused;
  }
}


void audio_SourceCommon_resume(audio_SourceCommon *source) {
  if(source->state == audio_SourceState_paused) {
    alSourcePlay(source->source);
    source->state = audio_SourceState_playing;
  }
}


bool audio_SourceCommon_isPlaying(audio_SourceCommon const* source) {
  return source->state == audio_SourceState_playing;
}


bool audio_SourceCommon_isStopped(audio_SourceCommon const* source) {
  return source->state == audio_SourceState_stopped;
}


bool audio_SourceCommon_isPaused(audio_SourceCommon const* source) {
  return source->state == audio_SourceState_paused;
}


void audio_SourceCommon_setVolume(audio_SourceCommon const* source, float gain) {
  alSourcef(source->source, AL_GAIN, gain);
}


float audio_SourceCommon_getVolume(audio_SourceCommon const* source) {
  float gain;
  alGetSourcef(source->source, AL_GAIN, &gain);
  return gain;
}


float audio_SourceCommon_getPitch(audio_SourceCommon const* source) {
  float gain;
  alGetSourcef(source->source, AL_PITCH, &gain);
  return gain;
}


void audio_SourceCommon_setPitch(audio_SourceCommon const* source, float gain) {
  alSourcef(source->source, AL_PITCH, gain);
}
