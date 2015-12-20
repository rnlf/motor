#include <AL/al.h>
#include <alloca.h>
#include "../3rdparty/stb/stb_vorbis.c"
#include "decoder.h"


typedef struct {
  stb_vorbis *vorbis;
  ALshort    *readBuffer;
  int         readBufferSize;
  int         preloadedSamples;
  bool        atEnd;
} audio_vorbis_DecoderData;


bool audio_vorbis_openStream(char const * filename, void **decoderData) {
  int err;
  audio_vorbis_DecoderData* data = malloc(sizeof(audio_vorbis_DecoderData));
  data->vorbis = stb_vorbis_open_filename(filename, &err, NULL);

  stb_vorbis_info info = stb_vorbis_get_info(data->vorbis);

  data->readBufferSize    = info.channels * info.sample_rate + 4096;
  data->readBuffer        = malloc(sizeof(ALshort) * data->readBufferSize);
  data->preloadedSamples  = 0;
  data->atEnd             = false;

  *decoderData = data;

  return true;
}


bool audio_vorbis_closeStream(void **decoderData) {
  audio_vorbis_DecoderData * data = (audio_vorbis_DecoderData*)decoderData;
  stb_vorbis_close(data->vorbis);
  free(data->readBuffer);
}


int audio_vorbis_preloadStreamSamples(void* decoderData, int sampleCount) {
  audio_vorbis_DecoderData * data = (audio_vorbis_DecoderData*)decoderData;
  stb_vorbis_info info = stb_vorbis_get_info(data->vorbis);
  int channels = info.channels >= 2 ? 2 : 1;   // Force to mono or stereo

  int safeBufferSize = sampleCount * channels + 4096;
  if(safeBufferSize > data->readBufferSize) {
    data->readBufferSize = safeBufferSize;
    data->readBuffer = realloc(data->readBuffer, sizeof(ALshort) * safeBufferSize);
  }

  int space = data->readBufferSize - data->preloadedSamples - 4096;
  if(space <= 0) {
    return -1;
  }

  if(space < sampleCount) {
    sampleCount = space;
  }

  int readSamples = 0;
  while(readSamples < sampleCount) {
    float **channelData;
    int samples = stb_vorbis_get_frame_float(data->vorbis, NULL, &channelData);
    if(samples == 0) {
      data->atEnd = true;
      break;
    }
    for(int i = 0; i < samples; ++i) {
      for(int c = 0; c < channels; ++c) {
        data->readBuffer[data->preloadedSamples + readSamples + channels * i + c] = (ALshort)(channelData[c][i] * 0x7FFF);
      }
    }

    readSamples += channels * samples;
  }

  data->preloadedSamples += readSamples;

  return readSamples;
}

int audio_vorbis_uploadPreloadedStreamSamples(void *decoderData, ALuint buffer) {
  audio_vorbis_DecoderData * data = (audio_vorbis_DecoderData*)decoderData;
  stb_vorbis_info info = stb_vorbis_get_info(data->vorbis);
  int channels = info.channels >= 2 ? 2 : 1;   // Force to mono or stereo

  // Emergency loading if we ran out of time for proper preloading
  if(!data->atEnd && data->preloadedSamples < data->readBufferSize / 2) {
    audio_vorbis_preloadStreamSamples(decoderData, data->readBufferSize / 2);
  }

  if(data->preloadedSamples == 0) {
    return 0;
  }

  alBufferData(
    buffer,
    channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
    data->readBuffer,
    data->preloadedSamples * sizeof(ALshort),
    info.sample_rate
  );
  //printf("uploaded %d samples to buffer %d\n", data->preloadedSamples, buffer);

  int uploaded = data->preloadedSamples;
  data->preloadedSamples = 0;
  return uploaded;
}



bool audio_vorbis_load(ALuint buffer, char const *filename) {
  short *data;
  int channels;
  int samplingrate;
  int len = stb_vorbis_decode_filename(filename, &channels, &samplingrate, &data);

  if(len == -1) {
    return false;
  }

  alBufferData(buffer, channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, data, len * sizeof(short) * channels, samplingrate);

  free(data);

  return true;
}

void audio_vorbis_rewindStream(void *decoderData) {
  audio_vorbis_DecoderData * data = (audio_vorbis_DecoderData*)decoderData;
  data->atEnd = false;
  stb_vorbis_seek_start(data->vorbis);
}

bool audio_vorbis_atEnd(void const *decoderData) {
  audio_vorbis_DecoderData const * data = (audio_vorbis_DecoderData const *)decoderData;
  return data->atEnd;
}

int audio_vorbis_getChannelCount(void *decoderData) {
  stb_vorbis_info info = stb_vorbis_get_info((stb_vorbis*)decoderData);
  return info.channels;
}

int audio_vorbis_getSampleRate(void *decoderData) {
  stb_vorbis_info info = stb_vorbis_get_info((stb_vorbis*)decoderData);
  return info.sample_rate;
}

void audio_vorbis_flushBuffer(void *decoderData) {
  audio_vorbis_DecoderData * data = (audio_vorbis_DecoderData*)decoderData;
  data->preloadedSamples = 0;
}

audio_StreamSourceDecoder audio_vorbis_decoder = {
  .testFile          = NULL,
  .getChannelCount   = audio_vorbis_getChannelCount,
  .getSampleRate     = audio_vorbis_getSampleRate,
  .openFile          = audio_vorbis_openStream,
  .closeFile         = audio_vorbis_closeStream,
  .atEnd             = audio_vorbis_atEnd,
  .rewind            = audio_vorbis_rewindStream,
  .preloadSamples    = audio_vorbis_preloadStreamSamples,
  .uploadPreloadedSamples = audio_vorbis_uploadPreloadedStreamSamples,
  .flush             = audio_vorbis_flushBuffer
};

audio_StaticSourceDecoder audio_vorbis_static_decoder = {
  .testFile = NULL,
  .loadFile = audio_vorbis_load
};
