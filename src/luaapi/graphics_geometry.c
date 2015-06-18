#include <stdlib.h>
#include "graphics_geometry.h"
#include "../graphics/geometry.h"
#include "tools.h"
#include "polygon.h"

static const l_tools_Enum l_graphics_DrawMode[] = {
  {"fill", graphics_DrawMode_fill},
  {"line", graphics_DrawMode_line},
  {NULL, 0}
};

static const l_tools_Enum l_graphics_LineJoin[] = {
  {"none",  graphics_LineJoin_none},
  {"bevel", graphics_LineJoin_bevel},
  {"miter", graphics_LineJoin_miter},
  {NULL, 0}
};

int l_geometry_circle(lua_State* state) {
  graphics_DrawMode mode = l_tools_toEnumOrError(state, 1, l_graphics_DrawMode);
  float x = l_tools_toNumberOrError(state, 2);
  float y = l_tools_toNumberOrError(state, 3);
  float r = l_tools_toNumberOrError(state, 4);
  int segments;
  if(lua_isnoneornil(state, 5)) {
    segments = r > 10 ? (int)r : 10;
  } else  {
    segments = l_tools_toNumberOrError(state, 5);
  }

  switch(mode) {
  case graphics_DrawMode_fill:
    graphics_geometry_fillCircle(x,y,r,segments);
    break;

  case graphics_DrawMode_line:
    graphics_geometry_drawCircle(x,y,r,segments);
    break;
  }

  return 0;
}

static int l_geometry_rectangle(lua_State* state) {
  graphics_DrawMode mode = l_tools_toEnumOrError(state, 1, l_graphics_DrawMode);
  float x = l_tools_toNumberOrError(state, 2);
  float y = l_tools_toNumberOrError(state, 3);
  float w = l_tools_toNumberOrError(state, 4);
  float h = l_tools_toNumberOrError(state, 5);

  switch(mode) {
  case graphics_DrawMode_fill:
    graphics_geometry_fillRectangle(x, y, w, h);
    break;

  case graphics_DrawMode_line:
    graphics_geometry_drawRectangle(x, y, w, h);
    break;
  }

  return 0;
}


static int l_geometry_line(lua_State* state) {
  float * vertices;
  int count = l_geometry_read_vertices(state, 0, &vertices);
  graphics_geometry_drawLines(count, vertices);
  return 0;
}


static int l_geometry_polygon(lua_State* state) {
  float * vertices;
  int count = l_geometry_read_vertices(state, 1, &vertices);
  graphics_DrawMode mode = l_tools_toEnumOrError(state, 1, l_graphics_DrawMode);
  switch(mode) {
  case graphics_DrawMode_fill:
    graphics_geometry_fillPolygon(count, vertices);
    break;

  case graphics_DrawMode_line:
    graphics_geometry_drawPolygon(count, vertices);
    break;
  }
  return 0;
}


static int l_geometry_setLineWidth(lua_State* state) {
  float width = l_tools_toNumberOrError(state, 1);
  graphics_geometry_setLineWidth(width);
  return 0;
}


static int l_geometry_getLineWidth(lua_State* state) {
  lua_pushnumber(state, graphics_geometry_getLineWidth());
  return 1;
}


static int l_geometry_setLineJoin(lua_State* state) {
  graphics_LineJoin join = l_tools_toEnumOrError(state, 1, l_graphics_LineJoin);
  if(join == graphics_LineJoin_bevel) {
    lua_pushstring(state, "'bevel' line join mode not supported yet");
    return lua_error(state);
  }

  graphics_geometry_setLineJoin(join);
  return 0;
}


static int l_geometry_getLineJoin(lua_State* state) {
  l_tools_pushEnum(state, graphics_geometry_getLineJoin(), l_graphics_LineJoin);
  return 1;
}


static luaL_Reg const geometryFreeFuncs[] = {
  {"circle",       l_geometry_circle},
  {"rectangle",    l_geometry_rectangle},
  {"line",         l_geometry_line},
  {"polygon",      l_geometry_polygon},
  {"setLineWidth", l_geometry_setLineWidth},
  {"getLineWidth", l_geometry_getLineWidth},
  {"setLineJoin",  l_geometry_setLineJoin},
  {"getLineJoin",  l_geometry_getLineJoin},
  {NULL, NULL}
};


void l_graphics_geometry_register(lua_State* state) {
  l_tools_registerFuncsInModule(state, "graphics", geometryFreeFuncs);
}

