/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <lauxlib.h>
#include "filesystem.h"
#include "tools.h"
#include "../filesystem/filesystem.h"

  
static int l_filesystem_read(lua_State* state) {
  // TODO implement max length
  char const* filename = l_tools_toStringOrError(state, 1);
  int len = luaL_optinteger(state, 2, -1);
  char* data = NULL;
  int rlen = filesystem_read(filename, len, &data);
  if(rlen < 0) {
    lua_pushstring(state, "could not read file");
    return lua_error(state);
  }

  lua_pushstring(state, data);
  free(data);
  lua_pushnumber(state, rlen);
  return 2;
}


static int l_filesystem_write(lua_State* state) {
  char const* filename = l_tools_toStringOrError(state, 1);
  char const* data = lua_tostring(state, 2);
  int size = luaL_optinteger(state, 3, -1);

  lua_pushboolean(state, filesystem_write(filename, data, size));
  return 1;
}


static int l_filesystem_load(lua_State* state) {
  char const* filename = l_tools_toStringOrError(state, 1);
  char* data = NULL;
  int len = filesystem_read(filename, -1, &data);
  if(len < 0) {
    lua_pushstring(state, "could not read file");
    return lua_error(state);
  }

  luaL_loadstring(state, data);
  free(data);
  return 1;
}


static int l_filesystem_isDirectory(lua_State* state) {
  char const* filename = l_tools_toStringOrError(state, 1);
  lua_pushboolean(state, filesystem_isDirectory(filename));
  
  return 1;
}


static int l_filesystem_isFile(lua_State* state) {
  char const* filename = l_tools_toStringOrError(state, 1);
  lua_pushboolean(state, filesystem_isFile(filename));
  
  return 1;
}


static int l_filesystem_setIdentity(lua_State* state) {
  char const *name = l_tools_toStringOrError(state, 1);
  bool first = false;
  if(lua_gettop(state) >= 2) {
    first = l_tools_toBooleanOrError(state, 2);
  }

  filesystem_setIdentity(name, first);

  return 0;
}


static void pushFilenames(lua_State* state, bool identity) {
  DIR* dir = filesystem_openDir(l_tools_toStringOrError(state, 1), identity);
  if(!dir) {
    return;
  }

  int table = lua_gettop(state);
  int idx = lua_objlen(state, table);
  struct dirent* entry;
  while((entry = readdir(dir))) {
    if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
      lua_pushstring(state, entry->d_name);
      ++idx;
      lua_rawseti(state, table, idx);
    }
  }

  closedir(dir);
}


static int l_filesystem_getDirectoryItems(lua_State* state) {
  lua_newtable(state);
  pushFilenames(state, true);
  pushFilenames(state, false);
  return 1;
}


static int l_filesystem_getLastModified(lua_State* state) {
  char const* filename = l_tools_toStringOrError(state, 1);

  double modtime;
  if(filesystem_getLastModified(filename, &modtime)) {
    lua_pushnumber(state, modtime);
    return 1;
  }
  
  lua_pushnil(state);
  lua_pushstring(state, "file not found");
  return 2;
}


static luaL_Reg const regFuncs[] = {
  {"load",              l_filesystem_load},
  {"read",              l_filesystem_read},
  {"write",             l_filesystem_write},
  {"isFile",            l_filesystem_isFile},
  {"isDirectory",       l_filesystem_isDirectory},
  {"setIdentity",       l_filesystem_setIdentity},
  {"getDirectoryItems", l_filesystem_getDirectoryItems},
  {"getLastModified",   l_filesystem_getLastModified},
  {NULL, NULL}
};


int l_filesystem_register(lua_State* state) {
  l_tools_registerModule(state, "filesystem", regFuncs);
  return 0;
}
