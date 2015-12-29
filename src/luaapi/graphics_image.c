/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <lauxlib.h>
#include "graphics.h"
#include "graphics_image.h"
#include "tools.h"

static struct {
  int imageMT;
} moduleData;

int l_graphics_newImage(lua_State* state) {
  if(lua_type(state, 1) == LUA_TSTRING) {
    l_image_newImageData(state);
    lua_remove(state, 1);
  } 
  
  if(!l_image_isImageData(state, 1)) {
    lua_pushstring(state, "expected ImageData");
    return lua_error(state);
  }

  lua_settop(state, 1);
  image_ImageData * imageData = (image_ImageData*)lua_touserdata(state, 1);
  int ref = luaL_ref(state, LUA_REGISTRYINDEX);

  l_graphics_Image *image = (l_graphics_Image*)lua_newuserdata(state, sizeof(l_graphics_Image));

  graphics_Image_new_with_ImageData(&image->image, imageData);
  image->imageDataRef = ref;

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.imageMT);
  lua_setmetatable(state, -2);

  return 1;
}

static int l_graphics_gcImage(lua_State* state) {
  /*
  if(!l_graphics_isImage(state, 1)) {
    lua_pushstring(state, "Expected Image");
    return lua_error(state);
  }
  */

  l_graphics_Image* img = l_graphics_toImage(state, 1);

  graphics_Image_free(&img->image);
  luaL_unref(state, LUA_REGISTRYINDEX, img->imageDataRef);
  return 0;
}

static int l_graphics_Image_getDimensions(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);
  lua_pushinteger(state, img->image.width);
  lua_pushinteger(state, img->image.height);
  return 2;
}

static int l_graphics_Image_getWidth(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);
  lua_pushinteger(state, img->image.width);
  return 1;
}

static int l_graphics_Image_getHeight(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);
  lua_pushinteger(state, img->image.height);
  return 1;
}

static int l_graphics_Image_getWrap(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);

  graphics_Wrap wrap;
  graphics_Image_getWrap(&img->image, &wrap);

  l_tools_pushEnum(state, wrap.horMode, l_graphics_WrapMode);
  l_tools_pushEnum(state, wrap.verMode, l_graphics_WrapMode);

  return 2;
}

static int l_graphics_Image_setWrap(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);
  graphics_Wrap wrap;
  wrap.horMode = l_tools_toEnumOrError(state, 2, l_graphics_WrapMode);
  wrap.verMode = l_tools_toEnumOrError(state, 3, l_graphics_WrapMode);

  graphics_Image_setWrap(&img->image, &wrap);

  return 0;
}

static int l_graphics_Image_getFilter(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);

  graphics_Filter filter;

  graphics_Image_getFilter(&img->image, &filter);

  l_tools_pushEnum(state, filter.minMode, l_graphics_FilterMode);
  l_tools_pushEnum(state, filter.magMode, l_graphics_FilterMode);
  lua_pushnumber(state, filter.maxAnisotropy);

  return 3;
}

static int l_graphics_Image_setFilter(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);
  graphics_Filter newFilter;
  graphics_Image_getFilter(&img->image, &newFilter);
  newFilter.minMode = l_tools_toEnumOrError(state, 2, l_graphics_FilterMode);
  newFilter.magMode = l_tools_toEnumOrError(state, 3, l_graphics_FilterMode);
  newFilter.maxAnisotropy = luaL_optnumber(state, 4, 1.0f);
  graphics_Image_setFilter(&img->image, &newFilter);

  return 0;
}

static int l_graphics_Image_setMipmapFilter(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);

  graphics_Filter newFilter;
  graphics_Image_getFilter(&img->image, &newFilter);

  if(lua_isnoneornil(state, 2)) {
    newFilter.mipmapMode  = graphics_FilterMode_none; 
    newFilter.mipmapLodBias = 0.0f;
  } else {
    newFilter.mipmapMode  = l_tools_toEnumOrError(state, 2, l_graphics_FilterMode);
    // param 2 is supposed to be "sharpness", which is exactly opposite to LOD,
    // therefore we use the negative value
    newFilter.mipmapLodBias = -luaL_optnumber(state, 3, 0.0f);
  }
  graphics_Image_setFilter(&img->image, &newFilter);

  return 0;
}

static int l_graphics_Image_getMipmapFilter(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);

  graphics_Filter filter;

  graphics_Image_getFilter(&img->image, &filter);

  l_tools_pushEnum(state, filter.mipmapMode, l_graphics_FilterMode);
  lua_pushnumber(state, filter.mipmapLodBias);

  return 2;
}

static int l_graphics_Image_getData(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);

  l_graphics_Image* img = l_graphics_toImage(state, 1);

  lua_rawgeti(state, LUA_REGISTRYINDEX, img->imageDataRef);

  return 1;
}

static int l_graphics_Image_refresh(lua_State* state) {
  l_assertType(state, 1, l_graphics_isImage);
  l_graphics_Image* img = l_graphics_toImage(state, 1);

  lua_rawgeti(state, LUA_REGISTRYINDEX, img->imageDataRef);
  image_ImageData *data = l_image_toImageData(state, -1);

  graphics_Image_refresh(&img->image, data);

  return 0;
}

static luaL_Reg const imageMetatableFuncs[] = {
  {"__gc",               l_graphics_gcImage},
  {"getDimensions",      l_graphics_Image_getDimensions},
  {"getWidth",           l_graphics_Image_getWidth},
  {"getHeight",          l_graphics_Image_getHeight},
  {"setFilter",          l_graphics_Image_setFilter},
  {"getFilter",          l_graphics_Image_getFilter},
  {"setMipmapFilter",    l_graphics_Image_setMipmapFilter},
  {"getMipmapFilter",    l_graphics_Image_getMipmapFilter},
  {"setWrap",            l_graphics_Image_setWrap},
  {"getWrap",            l_graphics_Image_getWrap},
  {"getData",            l_graphics_Image_getData},
  {"refresh",            l_graphics_Image_refresh},
  {NULL, NULL}
};

static luaL_Reg const imageFreeFuncs[] = {
  {"newImage",           l_graphics_newImage},
  {NULL, NULL}
};

void l_graphics_image_register(lua_State* state) {
  l_tools_registerFuncsInModule(state, "graphics", imageFreeFuncs);
  moduleData.imageMT  = l_tools_makeTypeMetatable(state, imageMetatableFuncs);
}

l_checkTypeFn(l_graphics_isImage, moduleData.imageMT)
l_toTypeFn(l_graphics_toImage, l_graphics_Image)
