/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../3rdparty/stb/stb_image.c"
#include "imagedata.h"
#include "../filesystem/filesystem.h"


char const* image_lastError(void) {
  return stbi_failure_reason();
}

bool image_ImageData_new_with_filename(image_ImageData *dst, char const* filename) {
  int n;

  char const *file = filesystem_locateReadableFile(filename);
  if(file == 0) {
    return false;
  }
  dst->surface = stbi_load(file, &dst->w, &dst->h, &n, 4);
  if(dst->surface == 0) {
    return false;
  }
  return true;
}

void image_ImageData_new_with_size(image_ImageData *dst, int width, int height) {
  dst->surface = malloc(width*height*4);
  memset(dst->surface, 0, width*height*4);
}

void image_ImageData_free(image_ImageData *data) {
  free(data->surface);
}

void image_init(void) {
}
