#include <tgmath.h>
#include <lauxlib.h>
#include "graphics_batch.h"
#include "graphics_quad.h"
#include "graphics_texture.h"
#include "tools.h"


static struct {
  int batchMT;
} moduleData;

static const graphics_Quad defaultQuad = {
  .x = 0.0,
  .y = 0.0,
  .w = 1.0,
  .h = 1.0
};

int l_graphics_newSpriteBatch(lua_State* state) {
  l_graphics_Image const* image = l_graphics_toTextureOrError(state, 1);

  int count = luaL_optnumber(state, 2, 128);

  l_graphics_Batch* batch = lua_newuserdata(state, sizeof(l_graphics_Batch));
  graphics_Batch_new(&batch->batch, &image->image, count, graphics_BatchUsage_static);

  lua_pushvalue(state, 1);
  batch->textureRef = luaL_ref(state, LUA_REGISTRYINDEX);

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.batchMT);
  lua_setmetatable(state, -2);
  return 1;
}

static int l_graphics_gcSpriteBatch(lua_State* state) {
  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);
  graphics_Batch_free(&batch->batch);
  luaL_unref(state, LUA_REGISTRYINDEX, batch->textureRef);


  return 0;
}

static int l_graphics_SpriteBatch_bind(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  graphics_Batch_bind(&batch->batch);

  return 0;
}

static int l_graphics_SpriteBatch_unbind(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  graphics_Batch_unbind(&batch->batch);

  return 0;
}

static int l_graphics_SpriteBatch_flush(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  graphics_Batch_flush(&batch->batch);

  return 0;
}

static int l_graphics_SpriteBatch_add(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  graphics_Quad const * quad = &defaultQuad;
  int baseidx = 2;

  if(l_graphics_isQuad(state, 2)) {
    quad = l_graphics_toQuad(state, 2);
    baseidx = 3;
  }
  
  float x  = luaL_optnumber(state, baseidx,     0.0f);
  float y  = luaL_optnumber(state, baseidx + 1, 0.0f);
  float r  = luaL_optnumber(state, baseidx + 2, 0.0f);
  float sx = luaL_optnumber(state, baseidx + 3, 1.0f);
  float sy = luaL_optnumber(state, baseidx + 4, sx);
  float ox = luaL_optnumber(state, baseidx + 5, 0.0f);
  float oy = luaL_optnumber(state, baseidx + 6, 0.0f);
  float kx = luaL_optnumber(state, baseidx + 7, 0.0f);
  float ky = luaL_optnumber(state, baseidx + 8, 0.0f);

  int i = graphics_Batch_add(&batch->batch, quad, x, y, r, sx, sy, ox, oy, kx, ky);
  lua_pushinteger(state, i);


  return 1;
}

static int l_graphics_SpriteBatch_set(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  int id = l_tools_toNumberOrError(state, 2);

  graphics_Quad const * quad = &defaultQuad;
  int baseidx = 3;

  if(l_graphics_isQuad(state, 3)) {
    quad = l_graphics_toQuad(state, 3);
    baseidx = 4;
  }
  
  float x  = luaL_optnumber(state, baseidx,     0.0f);
  float y  = luaL_optnumber(state, baseidx + 1, 0.0f);
  float r  = luaL_optnumber(state, baseidx + 2, 0.0f);
  float sx = luaL_optnumber(state, baseidx + 3, 1.0f);
  float sy = luaL_optnumber(state, baseidx + 4, sx);
  float ox = luaL_optnumber(state, baseidx + 5, 0.0f);
  float oy = luaL_optnumber(state, baseidx + 6, 0.0f);
  float kx = luaL_optnumber(state, baseidx + 7, 0.0f);
  float ky = luaL_optnumber(state, baseidx + 8, 0.0f);

  graphics_Batch_set(&batch->batch, id, quad, x, y, r, sx, sy, ox, oy, kx, ky);

  return 0;
}

static int l_graphics_SpriteBatch_clear(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);
  graphics_Batch_clear(&batch->batch);

  return 0;
}

int l_graphics_SpriteBatch_getBufferSize(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);
  
  lua_pushnumber(state, batch->batch.maxCount);

  return 1;
}

int l_graphics_SpriteBatch_setBufferSize(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  int newsize = l_tools_toNumberOrError(state, 2);
  
  graphics_Batch_setBufferSize(&batch->batch, newsize);

  return 0;
}

int l_graphics_SpriteBatch_getCount(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  lua_pushnumber(state, batch->batch.insertPos);

  return 1;
}

int l_graphics_SpriteBatch_setTexture(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);
  l_assertType(state, 2, l_graphics_isImage);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);
  l_graphics_Image * image = l_graphics_toImage(state, 2);

  batch->batch.texture = &image->image;
  luaL_unref(state, LUA_REGISTRYINDEX, batch->textureRef);
  lua_settop(state, 2);
  batch->textureRef = luaL_ref(state, LUA_REGISTRYINDEX);

  return 0;
}

int l_graphics_SpriteBatch_getTexture(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  lua_rawgeti(state, LUA_REGISTRYINDEX, batch->textureRef);

  return 1;
}

int l_graphics_SpriteBatch_setColor(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  if(!lua_isnumber(state, 2)) {
    graphics_Batch_clearColor(&batch->batch);
  } else {
    float r = l_tools_toNumberOrError(state, 2);
    float g = l_tools_toNumberOrError(state, 3);
    float b = l_tools_toNumberOrError(state, 4);
    float a = luaL_optnumber(state, 5, 255);
    graphics_Batch_setColor(&batch->batch, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  }

  
  return 0;
}

int l_graphics_SpriteBatch_getColor(lua_State* state) {
  l_assertType(state, 1, l_graphics_isBatch);

  l_graphics_Batch * batch = l_graphics_toBatch(state, 1);

  if(!batch->batch.colorSet) {
    return 0;
  }

  lua_pushnumber(state, floor(batch->batch.color.x * 255));
  lua_pushnumber(state, floor(batch->batch.color.y * 255));
  lua_pushnumber(state, floor(batch->batch.color.z * 255));
  lua_pushnumber(state, floor(batch->batch.color.w * 255));

  return 4;
}

l_checkTypeFn(l_graphics_isBatch, moduleData.batchMT)
l_toTypeFn(l_graphics_toBatch, l_graphics_Batch)

static luaL_Reg const batchMetatableFuncs[] = {
  {"__gc",               l_graphics_gcSpriteBatch},
  {"add",                l_graphics_SpriteBatch_add},
  {"set",                l_graphics_SpriteBatch_set},
  {"bind",               l_graphics_SpriteBatch_bind},
  {"unbind",             l_graphics_SpriteBatch_unbind},
  {"flush",              l_graphics_SpriteBatch_flush},
  {"clear",              l_graphics_SpriteBatch_clear},
  {"getBufferSize",      l_graphics_SpriteBatch_getBufferSize},
  {"setBufferSize",      l_graphics_SpriteBatch_setBufferSize},
  {"getCount",           l_graphics_SpriteBatch_getCount},
  {"setTexture",         l_graphics_SpriteBatch_setTexture},
  {"setImage",           l_graphics_SpriteBatch_setTexture},
  {"getTexture",         l_graphics_SpriteBatch_getTexture},
  {"getImage",           l_graphics_SpriteBatch_getTexture},
  {"setColor",           l_graphics_SpriteBatch_setColor},
  {"getColor",           l_graphics_SpriteBatch_getColor},
  {NULL, NULL}
};

static luaL_Reg const batchFreeFuncs[] = {
  {"newSpriteBatch",     l_graphics_newSpriteBatch},
  {NULL, NULL}
};

void l_graphics_batch_register(lua_State* state) {
  l_tools_registerFuncsInModule(state, "graphics", batchFreeFuncs);
  moduleData.batchMT  = l_tools_makeTypeMetatable(state, batchMetatableFuncs);
}
