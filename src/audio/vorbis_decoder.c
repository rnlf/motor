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


bool audio_vorbis_openStream(FILE* file, void **decoderData) {
  int err;
  audio_vorbis_DecoderData* data = malloc(sizeof(audio_vorbis_DecoderData));
  data->vorbis = stb_vorbis_open_file(file, 1, &err, NULL);

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
  return true;
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

// Why doesn't stb_vorbis have this?
static int vorbis_decode_file(FILE* file, int *channels, int *sample_rate, short **output)
{
   int data_len, offset, total, limit, error;
   short *data;
   stb_vorbis *v = stb_vorbis_open_file(file, 1, &error, NULL);
   if (v == NULL) return -1;
   limit = v->channels * 4096;
   *channels = v->channels;
   if (sample_rate)
      *sample_rate = v->sample_rate;
   offset = data_len = 0; 
   total = limit;
   data = (short *) malloc(total * sizeof(*data));
   if (data == NULL) {
      stb_vorbis_close(v);
      return -2;
   }
   for (;;) {
      int n = stb_vorbis_get_frame_short_interleaved(v, v->channels, data+offset, total-offset);
      if (n == 0) break;
      data_len += n;
      offset += n * v->channels;
      if (offset + limit > total) {
         short *data2;
         total *= 2;
         data2 = (short *) realloc(data, total * sizeof(*data));
         if (data2 == NULL) {
            free(data);
            stb_vorbis_close(v);
            return -2;
         }    
         data = data2;
      }    
   }
   *output = data;
   stb_vorbis_close(v);
   return data_len;
}



bool audio_vorbis_load(ALuint buffer, FILE* file) {
  short *data;
  int channels;
  int samplingrate;
  int len = vorbis_decode_file(file, &channels, &samplingrate, &data);

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
