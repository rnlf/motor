/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "gltools.h"


void graphics_Texture_setFilter(GLuint texID, graphics_Filter const* filter) {
  glBindTexture(GL_TEXTURE_2D, texID);

  int minFilter = GL_NEAREST;
  if(filter->mipmapMode == graphics_FilterMode_none) {
    switch(filter->minMode) {
    case graphics_FilterMode_linear:
      minFilter = GL_LINEAR;
      break;
    case graphics_FilterMode_nearest:
      minFilter = GL_NEAREST;
      break;
    default:
      break;
    }
  } else {
    if(filter->mipmapMode == graphics_FilterMode_nearest) {
      switch(filter->minMode) {
      case graphics_FilterMode_linear:
        minFilter = GL_LINEAR_MIPMAP_NEAREST;
        break;
      case graphics_FilterMode_nearest:
        minFilter = GL_NEAREST_MIPMAP_NEAREST;
        break;
      default:
        break;
      }
    } else if(filter->mipmapMode == graphics_FilterMode_linear) {
      switch(filter->minMode) {
      case graphics_FilterMode_linear:
        minFilter = GL_LINEAR_MIPMAP_LINEAR;
        break;
      case graphics_FilterMode_nearest:
        minFilter = GL_NEAREST_MIPMAP_LINEAR;
        break;
      default:
        break;
      }
    }
    glGenerateMipmap(GL_TEXTURE_2D);

  }

  int magFilter = (filter->magMode == graphics_FilterMode_linear) ? GL_LINEAR : GL_NEAREST;
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, filter->maxAnisotropy);
  
  // Not supported by WebGL, still interesting for native builds.
  // Accept GL_INVALID_ENUM on WebGL
  #ifndef EMSCRIPTEN
  if(filter->mipmapMode != graphics_FilterMode_none) {
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, filter->mipmapLodBias);
  }
  #endif
}


void graphics_Texture_getFilter(GLuint texID, graphics_Filter * filter) {
  glBindTexture(GL_TEXTURE_2D, texID);
  int fil;
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,         &fil);
  switch(fil) {
  case GL_NEAREST:
    filter->minMode = graphics_FilterMode_nearest;
    filter->mipmapMode = graphics_FilterMode_none;
    break;
  case GL_LINEAR:
    filter->minMode = graphics_FilterMode_linear;
    filter->mipmapMode = graphics_FilterMode_none;
    break;
  case GL_NEAREST_MIPMAP_NEAREST:
    filter->minMode = graphics_FilterMode_nearest;
    filter->mipmapMode = graphics_FilterMode_nearest;
    break;
  case GL_NEAREST_MIPMAP_LINEAR:
    filter->minMode = graphics_FilterMode_nearest;
    filter->mipmapMode = graphics_FilterMode_linear;
    break;
  case GL_LINEAR_MIPMAP_NEAREST:
    filter->minMode = graphics_FilterMode_linear;
    filter->mipmapMode = graphics_FilterMode_nearest;
    break;
  case GL_LINEAR_MIPMAP_LINEAR:
    filter->minMode = graphics_FilterMode_linear;
    filter->mipmapMode = graphics_FilterMode_linear;
    break;
  }

  #ifndef EMSCRIPTEN
  if(filter->mipmapMode == graphics_FilterMode_none) {
  #endif
    filter->mipmapLodBias = 0.0f;
  #ifndef EMSCRIPTEN
  } else {
    glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, &filter->mipmapLodBias);
  }
  #endif

  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,         &fil);

  filter->magMode = (fil == GL_LINEAR) ? graphics_FilterMode_linear : graphics_FilterMode_nearest;
  glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &filter->maxAnisotropy);
}
