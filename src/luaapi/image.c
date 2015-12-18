#include <stdio.h>
#include <lauxlib.h>
#include "image.h"
#include "tools.h"
#include "../image/imagedata.h"

static struct {
  int imageDataMT;
} moduleData;


int l_image_newImageData(lua_State* state) {
  image_ImageData* imageData = (image_ImageData*)lua_newuserdata(state, sizeof(image_ImageData));
  int s1type = lua_type(state, 1);
  if(s1type == LUA_TSTRING) {
    image_ImageData_new_with_filename(imageData, lua_tostring(state, 1));
  } else if(s1type == LUA_TNUMBER && lua_type(state, 2) == LUA_TNUMBER) {
    image_ImageData_new_with_size(imageData, lua_tointeger(state, 1), lua_tointeger(state, 2));
  } else {
    lua_pushstring(state, "need filename or size for imagedata");
    return lua_error(state);
  }

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.imageDataMT);
  lua_setmetatable(state, -2);

  return 1;
}


static int l_image_gcImageData(lua_State* state) {
  image_ImageData* imagedata = (image_ImageData*)lua_touserdata(state, 1);
  image_ImageData_free(imagedata);
  return 0;
}


static int l_graphics_Image_mapPixel(lua_State* state) {
  if(!l_image_isImageData(state, 1)) {
    lua_pushstring(state, "first parameter must be an ImageData object");
    return lua_error(state);
  }

  if(!(lua_isfunction(state, 2) || lua_istable(state, 2))) {
    // TODO: Add check for __call meta function in tables for debug build
    lua_pushstring(state, "second parameter must be a function or callable table");
    return lua_error(state);
  }

  image_ImageData *data = l_image_toImageData(state, 1);


  int x = luaL_optnumber(state, 3, 0);
  int y = luaL_optnumber(state, 4, 0);
  int width  = luaL_optnumber(state, 5, data->w - x);
  int height = luaL_optnumber(state, 6, data->h - y);

  for(int r = y; r <= y + height; ++r) {
    for(int c = x; c <= x + width; ++c) {
      uint8_t *pixel = data->surface + (r * data->w + c) * 4;
      printf("TOP: %d\n", lua_gettop(state));
      lua_pushvalue(state, 2);
      lua_pushnumber(state, c);
      lua_pushnumber(state, r);
      lua_pushnumber(state, pixel[0]);
      lua_pushnumber(state, pixel[1]);
      lua_pushnumber(state, pixel[2]);
      lua_pushnumber(state, pixel[3]);
      lua_call(state, 6, 4);
      pixel[0] = lua_tonumber(state, -4);
      pixel[1] = lua_tonumber(state, -3);
      pixel[3] = lua_tonumber(state, -2);
      pixel[4] = lua_tonumber(state, -1);
      lua_pop(state, 4);
    }
  }

  return 0;
}


static int l_graphics_Image_getWidth(lua_State *state) {
  image_ImageData *data = l_image_toImageData(state, 1);
  lua_pushnumber(state, data->w);
  return 1;
}


static int l_graphics_Image_getHeight(lua_State *state) {
  image_ImageData *data = l_image_toImageData(state, 1);
  lua_pushnumber(state, data->h);
  return 1;
}


static int l_graphics_Image_getDimensions(lua_State *state) {
  image_ImageData *data = l_image_toImageData(state, 1);
  lua_pushnumber(state, data->w);
  lua_pushnumber(state, data->h);
  return 2;
}


static luaL_Reg const regFuncs[] = {
  {"newImageData", l_image_newImageData},
  {NULL, NULL}
};


l_checkTypeFn(l_image_isImageData, moduleData.imageDataMT)
l_toTypeFn(l_image_toImageData, image_ImageData)


static luaL_Reg const imageDataMetatableFuncs[] = {
  {"mapPixel", l_graphics_Image_mapPixel},
  {"getWidth", l_graphics_Image_getWidth},
  {"getHeight", l_graphics_Image_getHeight},
  {"getDimensions", l_graphics_Image_getDimensions},
  {"__gc", l_image_gcImageData},
  {NULL, NULL}
};


int l_image_register(lua_State* state) {
  l_tools_registerModule(state, "image", regFuncs);

  moduleData.imageDataMT = l_tools_makeTypeMetatable(state, imageDataMetatableFuncs);
  
  return 0;
}
