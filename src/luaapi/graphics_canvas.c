#include "../math/minmax.h"
#include <lauxlib.h>
#include "tools.h"
#include "graphics_canvas.h"
#include "../graphics/graphics.h"
#include "graphics.h"

static struct {
  int canvasMT;
  graphics_Canvas* currentCanvas;
  int canvasTableRef;
  int canvases;
} moduleData;

l_checkTypeFn(l_graphics_isCanvas, moduleData.canvasMT)
l_toTypeFn(l_graphics_toCanvas, graphics_Canvas)


int l_graphics_newCanvas(lua_State* state) {
  int width  = luaL_optint(state, 1, graphics_getWidth());
  int height = luaL_optint(state, 2, graphics_getHeight());
  
  graphics_Canvas *canvas = lua_newuserdata(state, sizeof(graphics_Canvas));
  graphics_Canvas_new(canvas, width, height);

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.canvasMT);
  lua_setmetatable(state, -2);

  return 1;
}

static int l_graphics_gcCanvas(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  graphics_Canvas *canvas = l_graphics_toCanvas(state, 1);

  graphics_Canvas_free(canvas);

  return 1;
}

static int l_graphics_setCanvas(lua_State* state) {
  graphics_Canvas *canvas[32];

  int top = lua_gettop(state);

  int canvases = 0;
  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.canvasTableRef);
  for(int i = 0; i < min(32, top); ++i) {
    if(l_graphics_isCanvas(state, i+1)) {
      canvas[i] = l_graphics_toCanvas(state, i+1);
      lua_pushvalue(state, i+1);
      ++canvases;
      lua_rawseti(state, -2, canvases);
    } else if(!lua_isnoneornil(state, i+1)) {
      lua_pushstring(state, "expected none or canvas");
      return lua_error(state);
    }
  }

  for(int i = canvases; i < moduleData.canvases; ++i) {
    lua_pushnil(state);
    lua_rawseti(state, -2, i+1);
  }
  moduleData.canvases = canvases;

  //lua_settop(state, 1);

  graphics_setCanvas(canvas, canvases);

  return 0;
}

static int l_graphics_getCanvas(lua_State* state) {
  if(moduleData.canvases == 0) {
    lua_pushnil(state);
    return 1;
  }

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.canvasTableRef);
  int tab = lua_gettop(state);
  for(int i = 0; i < moduleData.canvases; ++i) {
    lua_rawgeti(state, tab, i+1);
  }

  return moduleData.canvases;
}


static int l_graphics_Canvas_renderTo(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);
  l_assertType(state, 2, lua_isfunction);


  graphics_Canvas *canvas = l_graphics_toCanvas(state, 1);
  // TODO actual limit
  graphics_Canvas *oldCanvas[32];
  int oldCount = graphics_getCanvas(oldCanvas);
  graphics_setCanvas(&canvas, 1);
  lua_call(state, 0, 0);
  graphics_setCanvas(oldCanvas, oldCount);
  return 0;
}


static int l_graphics_Canvas_clear(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  int top = lua_gettop(state);
  int r = 0;
  int g = 0;
  int b = 0;
  int a = 0;
  if(top > 1) {
    if(lua_istable(state, 2)) {
      lua_rawgeti(state, 2, 1);
      lua_rawgeti(state, 2, 2);
      lua_rawgeti(state, 2, 3);
      lua_rawgeti(state, 2, 4);
      r = l_tools_toNumberOrError(state, -4);
      g = l_tools_toNumberOrError(state, -3);
      b = l_tools_toNumberOrError(state, -2);
      a = luaL_optnumber(state, -1, 255);
    } else {
      r = l_tools_toNumberOrError(state, 2);
      g = l_tools_toNumberOrError(state, 3);
      b = l_tools_toNumberOrError(state, 4);
      a = luaL_optnumber(state, 5, 255);
    }
  }


  graphics_Canvas *canvas = l_graphics_toCanvas(state, 1);
  // TODO actual limit
  graphics_Canvas *oldCanvas[32];
  int oldCount = graphics_getCanvas(oldCanvas);
  graphics_setCanvas(&canvas, 1);

  float oldBackground[4];
  memcpy(oldBackground, graphics_getBackgroundColor(), sizeof(oldBackground));

  graphics_setBackgroundColor(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
  graphics_clear();

  graphics_setBackgroundColor(oldBackground[0], oldBackground[1], oldBackground[2], oldBackground[3]);

  graphics_setCanvas(oldCanvas, oldCount);
  return 0;
}


static int l_graphics_Canvas_getDimensions(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  graphics_Canvas* img = l_graphics_toCanvas(state, 1);
  lua_pushinteger(state, img->image.width);
  lua_pushinteger(state, img->image.height);
  return 2;
}

static int l_graphics_Canvas_getWidth(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  graphics_Canvas* img = l_graphics_toCanvas(state, 1);
  lua_pushinteger(state, img->image.width);
  return 1;
}

static int l_graphics_Canvas_getHeight(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  graphics_Canvas* img = l_graphics_toCanvas(state, 1);
  lua_pushinteger(state, img->image.height);
  return 1;
}

static int l_graphics_Canvas_getWrap(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  graphics_Canvas* img = l_graphics_toCanvas(state, 1);

  graphics_Wrap wrap;
  graphics_Image_getWrap(&img->image, &wrap);

  l_tools_pushEnum(state, wrap.horMode, l_graphics_WrapMode);
  l_tools_pushEnum(state, wrap.verMode, l_graphics_WrapMode);

  return 2;
}

static int l_graphics_Canvas_setWrap(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  graphics_Canvas* img = l_graphics_toCanvas(state, 1);
  graphics_Wrap wrap;
  wrap.horMode = l_tools_toEnumOrError(state, 2, l_graphics_WrapMode);
  wrap.verMode = l_tools_toEnumOrError(state, 3, l_graphics_WrapMode);

  graphics_Image_setWrap(&img->image, &wrap);

  return 0;
}

static int l_graphics_Canvas_getFilter(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  graphics_Canvas* img = l_graphics_toCanvas(state, 1);

  graphics_Filter filter;

  graphics_Image_getFilter(&img->image, &filter);

  l_tools_pushEnum(state, filter.minMode, l_graphics_FilterMode);
  l_tools_pushEnum(state, filter.magMode, l_graphics_FilterMode);
  lua_pushnumber(state, filter.maxAnisotropy);

  return 3;
}

static int l_graphics_Canvas_setFilter(lua_State* state) {
  l_assertType(state, 1, l_graphics_isCanvas);

  graphics_Canvas* img = l_graphics_toCanvas(state, 1);
  graphics_Filter newFilter;
  graphics_Image_getFilter(&img->image, &newFilter);
  newFilter.minMode = l_tools_toEnumOrError(state, 2, l_graphics_FilterMode);
  newFilter.magMode = l_tools_toEnumOrError(state, 3, l_graphics_FilterMode);
  newFilter.maxAnisotropy = luaL_optnumber(state, 4, 1.0f);
  graphics_Image_setFilter(&img->image, &newFilter);

  return 0;
}

static luaL_Reg const canvasMetatableFuncs[] = {
  {"__gc",               l_graphics_gcCanvas},
  {"renderTo",           l_graphics_Canvas_renderTo},
  {"getDimensions",      l_graphics_Canvas_getDimensions},
  {"getWidth",           l_graphics_Canvas_getWidth},
  {"getHeight",          l_graphics_Canvas_getHeight},
  {"setFilter",          l_graphics_Canvas_setFilter},
  {"getFilter",          l_graphics_Canvas_getFilter},
  {"setWrap",            l_graphics_Canvas_setWrap},
  {"getWrap",            l_graphics_Canvas_getWrap},
  {"clear",              l_graphics_Canvas_clear},
  //{"getData",            l_graphics_Image_getData},
  {NULL, NULL}
};

static luaL_Reg const canvasFreeFuncs[] = {
  {"newCanvas",          l_graphics_newCanvas},
  {"setCanvas",          l_graphics_setCanvas},
  {"getCanvas",          l_graphics_getCanvas},
  {NULL, NULL}
};

void l_graphics_canvas_register(lua_State * state) {
  moduleData.canvasMT = l_tools_makeTypeMetatable(state, canvasMetatableFuncs);
  l_tools_registerFuncsInModule(state, "graphics", canvasFreeFuncs);

  lua_createtable(state, 32, 1);
  moduleData.canvasTableRef = luaL_ref(state, LUA_REGISTRYINDEX);
}
