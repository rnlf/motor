/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "tools.h"
#include "audio.h"
#include "../math/minmax.h"

// TODO use two separate metatables for streaming and static sources?

static struct {
  int staticMT;
  int streamMT;
} moduleData;


l_checkTypeFn(l_audio_isStaticSource, moduleData.staticMT)
l_checkTypeFn(l_audio_isStreamSource, moduleData.streamMT)
l_toTypeFn(l_audio_toStaticSource, audio_StaticSource)
l_toTypeFn(l_audio_toStreamSource, audio_StreamSource)


static const l_tools_Enum l_audio_SourceType[] = {
  {"static", audio_SourceType_static},
  {"stream", audio_SourceType_stream},
  {NULL, 0}
};

static int l_audio_newSource(lua_State *state) {
  char const* filename = l_tools_toStringOrError(state, 1);

  audio_SourceType type = audio_SourceType_stream;
  if(!lua_isnoneornil(state, 2)) {
    type = l_tools_toEnumOrError(state, 2, l_audio_SourceType);
  }

  bool success = true;
  // TODO handle load errors
  switch(type) {
  case audio_SourceType_static:
    {
      audio_StaticSource *src = lua_newuserdata(state, sizeof(audio_StaticSource));
      success = audio_loadStatic(src, filename);
      lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.staticMT);
      break;
    }

  case audio_SourceType_stream:
    {
      audio_StreamSource *src = lua_newuserdata(state, sizeof(audio_StreamSource));
      audio_loadStream(src, filename);
      lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.streamMT);
      break;
    }
  }
  
  if(success) {
    // Do not set MT if loading failed: GC must not be called
    lua_setmetatable(state, -2);
    return 1;
  } else {
    lua_pushstring(state, "Failed to open audio source ");
    lua_pushstring(state, filename);
    lua_concat(state, 2);
    return lua_error(state);
  }
}


#define t_l_audio_source_generic(type, fun) \
  static int l_audio_ ## type ## Source_ ## fun(lua_State *state) { \
    l_assertType(state, 1, l_audio_is ## type ## Source);  \
    audio_ ## type ## Source *src = (audio_ ## type ## Source*) lua_touserdata(state, 1); \
    audio_ ## type ## Source_ ## fun(src); \
    return 0; \
  }


t_l_audio_source_generic(Stream, free)
t_l_audio_source_generic(Static, free)
t_l_audio_source_generic(Stream, play)
t_l_audio_source_generic(Static, play)
t_l_audio_source_generic(Static, stop)
t_l_audio_source_generic(Stream, stop)
t_l_audio_source_generic(Static, rewind)
t_l_audio_source_generic(Stream, rewind)
t_l_audio_source_generic(Static, pause)
t_l_audio_source_generic(Stream, pause)
t_l_audio_source_generic(Static, resume)
t_l_audio_source_generic(Stream, resume)
#undef t_l_audio_source_generic


#define t_l_audio_source_setLooping(type) \
  static int l_audio_ ## type ## Source_setLooping(lua_State *state) { \
    l_assertType(state, 1, l_audio_is ## type ## Source);  \
    audio_ ## type ## Source *src = lua_touserdata(state, 1); \
    bool loop = l_tools_toBooleanOrError(state, 2); \
    audio_ ## type ## Source_setLooping(src, loop); \
    return 0; \
  }

t_l_audio_source_setLooping(Static)
t_l_audio_source_setLooping(Stream)
#undef t_l_audio_source_setLooping


#define t_l_audio_source_isLooping(type) \
  static int l_audio_ ## type ## Source_isLooping(lua_State *state) { \
    l_assertType(state, 1, l_audio_is ## type ## Source);  \
    audio_ ## type ## Source *src = lua_touserdata(state, 1); \
    lua_pushboolean(state, audio_ ## type ## Source_isLooping(src)); \
    return 0; \
  }

t_l_audio_source_isLooping(Static)
t_l_audio_source_isLooping(Stream)
#undef t_l_audio_source_isLooping


static bool isSource(lua_State * state, int index) {
  return l_audio_isStaticSource(state, index) || l_audio_isStreamSource(state,index);
}


// Yes I know it is not very beautiful to assume knowledge of the structure
// of audio_StreamSource and audio_StaticSource. I am using this hack to
// avoid duplication of the code for each of these functions (and for the
// actual backends).
// But since I'm aiming to minimize the download size of the compiled
// engine, I think it is legitimate to do so.
#define t_l_audio_SourceCommon_getBool(fun) \
  static int l_audio_SourceCommon_ ## fun(lua_State* state) { \
    l_assertType(state, 1, isSource); \
    audio_SourceCommon const* src = (audio_SourceCommon const*)lua_touserdata(state, 1); \
    lua_pushboolean(state, audio_SourceCommon_ ## fun(src)); \
    return 1; \
  }
t_l_audio_SourceCommon_getBool(isPlaying)
t_l_audio_SourceCommon_getBool(isStopped)
t_l_audio_SourceCommon_getBool(isPaused)
#undef t_l_audio_SourceCommon_getBool

static int l_audio_SourceCommon_isStatic(lua_State *state) {
  if(l_audio_isStaticSource(state, 1)) {
    lua_pushboolean(state, true);
  } else if(l_audio_isStreamSource(state, 1)) {
    lua_pushboolean(state, false);
  } else {
    lua_pushstring(state, "Expected Source");
    return lua_error(state);
  }
  return 1;
}


static int l_audio_SourceCommon_setVolume(lua_State *state) {
  l_assertType(state, 1, isSource);
  float gain = l_tools_toNumberOrError(state, 2);
  audio_SourceCommon *source = (audio_SourceCommon*)lua_touserdata(state, 1);
  audio_SourceCommon_setVolume(source, gain);
  return 0;
}


static int l_audio_SourceCommon_getVolume(lua_State *state) {
  l_assertType(state, 1, isSource);
  audio_SourceCommon *source = (audio_SourceCommon*)lua_touserdata(state, 1);
  lua_pushnumber(state, audio_SourceCommon_getVolume(source));
  return 1;
}


static int l_audio_SourceCommon_setPitch(lua_State *state) {
  l_assertType(state, 1, isSource);
  float pitch = l_tools_toNumberOrError(state, 2);
  audio_SourceCommon *source = (audio_SourceCommon*)lua_touserdata(state, 1);
  audio_SourceCommon_setPitch(source, pitch);
  return 0;
}


static int l_audio_SourceCommon_getPitch(lua_State *state) {
  l_assertType(state, 1, isSource);
  audio_SourceCommon *source = (audio_SourceCommon*)lua_touserdata(state, 1);
  lua_pushnumber(state, audio_SourceCommon_getPitch(source));
  return 1;
}


static int l_audio_StreamSource_clone(lua_State *state) {
  l_assertType(state, 1, l_audio_isStreamSource);
  audio_StreamSource *oldSrc = l_audio_toStreamSource(state, 1);
  audio_StreamSource *newSrc = lua_newuserdata(state, sizeof(audio_StreamSource));
  audio_StreamSource_clone(oldSrc, newSrc);
  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.streamMT);
  lua_setmetatable(state, -2);
  return 1;
}


static int l_audio_StaticSource_clone(lua_State *state) {
  l_assertType(state, 1, l_audio_isStaticSource);
  audio_StaticSource *oldSrc = l_audio_toStaticSource(state, 1);
  audio_StaticSource *newSrc = lua_newuserdata(state, sizeof(audio_StaticSource));
  audio_StaticSource_clone(oldSrc, newSrc);
  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.staticMT);
  lua_setmetatable(state, -2);
  return 1;
}


static int l_audio_setVolume(lua_State *state) {
  double gain = l_tools_toNumberOrError(state, 1);
  gain = min(1.0, max(0.0, gain));
  audio_setVolume(gain);
  return 0;
}


l_tools_stub("Source:setPosition",             l_audio_SourceCommon_setPosition)
l_tools_stub("Source:setRelative",             l_audio_SourceCommon_setRelative)
l_tools_stub("Source:setAttenuationDistances", l_audio_SourceCommon_setAttenuationDistances)


#define t_sourceMetatableFuncs(type) \
  static luaL_Reg const type ## SourceMetatableFuncs[] = { \
    {"__gc",       l_audio_ ## type ## Source_free}, \
    {"play",       l_audio_ ## type ## Source_play}, \
    {"stop",       l_audio_ ## type ## Source_stop}, \
    {"rewind",     l_audio_ ## type ## Source_rewind}, \
    {"pause",      l_audio_ ## type ## Source_pause}, \
    {"resume",     l_audio_ ## type ## Source_resume}, \
    {"setLooping", l_audio_ ## type ## Source_setLooping}, \
    {"isPlaying",  l_audio_SourceCommon_isPlaying}, \
    {"isStopped",  l_audio_SourceCommon_isStopped}, \
    {"isPaused",   l_audio_SourceCommon_isPaused}, \
    {"isStatic",   l_audio_SourceCommon_isStatic}, \
    {"isLooping",  l_audio_ ## type ## Source_isLooping}, \
    {"setVolume",  l_audio_SourceCommon_setVolume}, \
    {"getVolume",  l_audio_SourceCommon_getVolume}, \
    {"clone",      l_audio_ ## type ## Source_clone}, \
    {"setPitch",   l_audio_SourceCommon_setPitch}, \
    {"getPitch",   l_audio_SourceCommon_getPitch}, \
    {"setPosition", l_audio_SourceCommon_setPosition}, \
    {"setRelative", l_audio_SourceCommon_setRelative}, \
    {"setAttenuationDistances", l_audio_SourceCommon_setAttenuationDistances}, \
    {NULL, NULL} \
  };
t_sourceMetatableFuncs(Stream)
t_sourceMetatableFuncs(Static)
#undef t_sourceMetatableFuncs




static luaL_Reg const regFuncs[] = {
  {"newSource", l_audio_newSource},
  {"setVolume", l_audio_setVolume},
  {NULL, NULL}
};

static char const bootScript[] =
  "for i, k in ipairs({\"play\", \"pause\", \"stop\", \"resume\", \"rewind\"}) do\n"
  "  love.audio[k] = function(src, ...)\n"
  "    src[k](src, ...)\n"
  "  end\n"
  "end\n";

static int registerFreeFunctionAdaptors(lua_State *state) {
  return luaL_dostring(state, bootScript);
}

int l_audio_register(lua_State *state) {
  l_tools_registerModule(state, "audio", regFuncs);
  moduleData.staticMT = l_tools_makeTypeMetatable(state, StaticSourceMetatableFuncs);
  moduleData.streamMT = l_tools_makeTypeMetatable(state, StreamSourceMetatableFuncs);

  if(registerFreeFunctionAdaptors(state)) {
    return 0;
  }

  return 1;
}
