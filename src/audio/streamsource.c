#include "../filesystem/filesystem.h"
#include <string.h>
#include <stdlib.h>
#include "streamsource.h"
#include "decoder.h"

static struct {
  audio_StreamSource ** playingStreams;
  int playingStreamSize;
  int playingStreamCount;
} moduleData;


extern audio_StreamSourceDecoder audio_vorbis_decoder;

static audio_StreamSourceDecoder const* streamDecoders[] = {
  &audio_vorbis_decoder
};

static void audio_StreamSource_stop1(audio_StreamSource *source);


static void initialPreload(audio_StreamSource *source) {
  for(int i = 0; i < preloadBufferCount; ++i) {
    source->decoder->preloadSamples(source->decoderData, 44100);
    source->decoder->uploadPreloadedSamples(source->decoderData, source->buffers[i]);
  }
}


void audio_StreamSource_free(audio_StreamSource *source) {
  // Stop without rewind
  audio_StreamSource_stop1(source);

  free(source->filename);
  alDeleteBuffers(preloadBufferCount, source->buffers);
  audio_SourceCommon_free(&source->common);
  source->decoder->closeFile(source->decoderData);
}


bool audio_loadStream(audio_StreamSource *source, char const * filename) {
  // TODO select approprate decoder (there is only one right now though!)
  source->decoder = streamDecoders[0];
  
  FILE* infile = filesystem_fopen(filename, "rb");

  bool good = source->decoder->openFile(infile, &source->decoderData);
  if(!good) {
    return false;
  }

  audio_SourceCommon_init(&source->common);

  alGenBuffers(preloadBufferCount, source->buffers);

  initialPreload(source);

  source->looping = false;

  // Make copy of filename (allows simple free on the filename when closing a stream)
  source->filename = malloc(sizeof(char) * (strlen(filename) + 1));
  strcpy(source->filename, filename);

  return good;
}


static void prepareToPlay(audio_StreamSource *source) {
  if(source->common.state == audio_SourceState_playing) {
    return;
  } else if(source->common.state == audio_SourceState_paused) {
    audio_SourceCommon_play(&source->common);
    return;
  }

  alSourceQueueBuffers(source->common.source, preloadBufferCount, source->buffers);

  if(moduleData.playingStreamCount == moduleData.playingStreamSize) {
    moduleData.playingStreamSize = 2 * moduleData.playingStreamSize;
    moduleData.playingStreams = realloc(moduleData.playingStreams, moduleData.playingStreamSize*sizeof(audio_StreamSource*));
  }

  moduleData.playingStreams[moduleData.playingStreamCount] = source;
  ++moduleData.playingStreamCount;
}


void audio_StreamSource_play(audio_StreamSource *source) {
  switch(source->common.state) {
  case audio_SourceState_stopped:
    prepareToPlay(source);
    // Fall through
  case audio_SourceState_paused:
    audio_SourceCommon_play(&source->common);
    break;
  default:
    break;
  }
}


void audio_updateStreams(void) {
  for(int i = 0; i < moduleData.playingStreamCount;) {
    audio_StreamSource const* source = moduleData.playingStreams[i];

    int loaded = source->decoder->preloadSamples(source->decoderData, 8000);
    if(loaded == 0) {
      if(source->looping) {
        source->decoder->rewind(source->decoderData);
      } else {
        
      }
    }

    ALuint src = source->common.source;
    ALint count;
    ALint queued;
    ALint state;
    alGetSourcei(src, AL_BUFFERS_PROCESSED, &count);
    alGetSourcei(src, AL_BUFFERS_QUEUED, &queued);
    alGetSourcei(src, AL_SOURCE_STATE, &state);
  //  printf("%d buffers free, %d queued, state=%d\n", count, queued, state);

    for(int j = 0; j < count; ++j) {
      ALuint buf;
      alSourceUnqueueBuffers(src, 1, &buf);
      // This may cause preloading two full frames
      
      int uploaded = source->decoder->uploadPreloadedSamples(source->decoderData, buf);
      if(uploaded) {
        alSourceQueueBuffers(src, 1, &buf);
      }
    }

    alGetSourcei(src, AL_BUFFERS_QUEUED, &queued);
    if(state == AL_STOPPED && queued == 0) {
      --moduleData.playingStreamCount;
      moduleData.playingStreams[i] = moduleData.playingStreams[moduleData.playingStreamCount];
    } else if(state == AL_STOPPED && queued > 0) {
      // Should only happen when we seriously ran out of time,
      // but there are documented cases where it happened during load time.
      alSourcePlay(src);
    } else {
      ++i;
    }
  }
}


void audio_StreamSource_setLooping(audio_StreamSource *source, bool loop) {
  source->looping = loop;
}


bool audio_StreamSource_isLooping(audio_StreamSource const* source) {
  return source->looping;
}


void audio_streamsource_init(void) {
  moduleData.playingStreamCount = 0;
  moduleData.playingStreamSize  = 16;
  moduleData.playingStreams     = malloc(sizeof(audio_StreamSource*) * 16);
}


static void audio_StreamSource_stop1(audio_StreamSource *source) {
  // Remove from list of active streams
  for(int i = 0; i < moduleData.playingStreamCount; ++i) {
    if(moduleData.playingStreams[i] == source) {
      --moduleData.playingStreamCount;
      moduleData.playingStreams[i] = moduleData.playingStreams[moduleData.playingStreamCount];
      break;
    }
  }

  audio_SourceCommon_stop(&source->common);

  ALint count;
  alGetSourcei(source->common.source, AL_BUFFERS_PROCESSED, &count);
  for(int j = 0; j < count; ++j) {
    ALuint buf;
    alSourceUnqueueBuffers(source->common.source, 1, &buf);
  }
}


void audio_StreamSource_stop(audio_StreamSource *source) {
  if(source->common.state == audio_SourceState_stopped) {
    return;
  }

  audio_StreamSource_stop1(source);

  source->decoder->rewind(source->decoderData);
  source->decoder->flush(source->decoderData);
  initialPreload(source);
}


void audio_StreamSource_rewind(audio_StreamSource *source) {
  // TODO Skip to end of current buffers if playing or paused
  audio_SourceState state = source->common.state;

  audio_StreamSource_stop(source);

  if(state == audio_SourceState_playing) {
    audio_StreamSource_play(source);
  } else {
    prepareToPlay(source);
    source->common.state = audio_SourceState_paused;
  }
}


void audio_StreamSource_pause(audio_StreamSource *source) {
  audio_SourceCommon_pause(&source->common);
}


void audio_StreamSource_resume(audio_StreamSource *source) {
  audio_SourceCommon_resume(&source->common);
}


void audio_StreamSource_clone(audio_StreamSource const* oldSrc, audio_StreamSource * newSrc) {
  audio_loadStream(newSrc, oldSrc->filename);
}
