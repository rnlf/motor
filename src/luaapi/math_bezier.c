#include <stdlib.h>
#include <lauxlib.h>
#include "math_bezier.h"
#include "../math/bezier.h"
#include "polygon.h"
#include "tools.h"


static struct {
  int bezierMT;
} moduleData;


static int l_math_newBezierCurve(lua_State* state) {
  float *vertices;
  int numCount = l_geometry_read_vertices(state, 0, &vertices, 4);


  math_BezierCurve* curve = lua_newuserdata(state, sizeof(math_BezierCurve));
  math_BezierCurve_new(curve, vertices, numCount);

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.bezierMT);
  lua_setmetatable(state, -2);

  return 1;
}


static int l_math_gcBezierCurve(lua_State* state) {
  math_BezierCurve* curve = l_math_toBezierCurve(state, 1);
  math_BezierCurve_free(curve);

  return 0;
}


static int l_math_BezierCurve_evaluate(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve const* curve = l_math_toBezierCurve(state, 1);

  float t = l_tools_toNumberOrError(state, 2);
  if(t < 0 || t > 1) {
    lua_pushstring(state, "parameter t out of range");
    return lua_error(state);
  }

  float pt[2];
  math_BezierCurve_evaluate(curve, t, pt, pt+1);

  lua_pushnumber(state, pt[0]);
  lua_pushnumber(state, pt[1]);

  return 2;
}


static int l_math_BezierCurve_getControlPoint(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve const* curve = l_math_toBezierCurve(state, 1);

  int i = l_tools_toNumberOrError(state, 2) - 1;
  if(math_BezierCurve_getControlPointCount(curve) <= i || i < 0) {
    lua_pushstring(state, "Control point index out of range");
    return lua_error(state);
  }

  float const* pt = math_BezierCurve_getControlPoint(curve, i);

  lua_pushnumber(state, pt[0]);
  lua_pushnumber(state, pt[1]);

  return 2;
}


static int l_math_BezierCurve_getControlPointCount(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve const* curve = l_math_toBezierCurve(state, 1);
  lua_pushnumber(state, math_BezierCurve_getControlPointCount(curve));
  return 1;
}


static int l_math_BezierCurve_getDegree(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve const* curve = l_math_toBezierCurve(state, 1);
  lua_pushnumber(state, math_BezierCurve_getDegree(curve));
  return 1;
}


static int l_math_BezierCurve_getDerivative(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve const* curve = l_math_toBezierCurve(state, 1);

  math_BezierCurve* derivativeCurve = lua_newuserdata(state, sizeof(math_BezierCurve));
  math_BezierCurve_getDerivative(curve, derivativeCurve);

  lua_rawgeti(state, LUA_REGISTRYINDEX, moduleData.bezierMT);
  lua_setmetatable(state, -2);
  return 1;
}


static int l_math_BezierCurve_insertControlPoint(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve *curve = l_math_toBezierCurve(state, 1);

  float x = l_tools_toNumberOrError(state, 2);
  float y = l_tools_toNumberOrError(state, 3);
  int i = luaL_optint(state, 4, -1);

  int count = math_BezierCurve_getControlPointCount(curve);
  if(i < -count || i > count) {
    lua_pushstring(state, "Control point index out of range");
    return lua_error(state);
  }

  math_BezierCurve_insertControlPoint(curve, i - 1, x, y);
  return 0;
}


static int l_math_BezierCurve_render(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve const* curve = l_math_toBezierCurve(state, 1);

  int depth = luaL_optint(state, 2, 5);

  size_t count;
  float const* coords = math_BezierCurve_render(curve, depth, &count);

  lua_createtable(state, 2*count, 0);
  for(size_t i = 0; i < 2*count; ++i) {
    lua_pushnumber(state, coords[i]);
    lua_rawseti(state, -2, i+1);
  }

  return 1;
}


static int l_math_BezierCurve_rotate(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve *curve = l_math_toBezierCurve(state, 1);

  float angle = l_tools_toNumberOrError(state, 2);
  float ox = luaL_optnumber(state, 3, 0.0f);
  float oy = luaL_optnumber(state, 4, 0.0f);

  math_BezierCurve_rotate(curve, angle, ox, oy);
  
  return 0;
}


static int l_math_BezierCurve_scale(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve *curve = l_math_toBezierCurve(state, 1);

  float scale = l_tools_toNumberOrError(state, 2);
  float ox = luaL_optnumber(state, 3, 0.0f);
  float oy = luaL_optnumber(state, 4, 0.0f);

  math_BezierCurve_scale(curve, scale, scale, ox, oy);
  
  return 0;
}


static int l_math_BezierCurve_setControlPoint(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve *curve = l_math_toBezierCurve(state, 1);

  int   i = l_tools_toNumberOrError(state, 2);
  float x = l_tools_toNumberOrError(state, 3);
  float y = l_tools_toNumberOrError(state, 4);

  math_BezierCurve_setControlPoint(curve, i, x, y);

  return 0;
}


static int l_math_BezierCurve_translate(lua_State* state) {
  l_assertType(state, 1, l_math_isBezierCurve);  
  math_BezierCurve *curve = l_math_toBezierCurve(state, 1);

  float x = l_tools_toNumberOrError(state, 2);
  float y = l_tools_toNumberOrError(state, 3);

  math_BezierCurve_translate(curve, x, y);

  return 0;
}


static luaL_Reg const bezierFreeFuncs[] = {
  {"newBezierCurve", l_math_newBezierCurve},
  {NULL, NULL}
};


static luaL_Reg const bezierMetatableFuncs[] = {
  {"__gc",                  l_math_gcBezierCurve},
  {"evaluate",              l_math_BezierCurve_evaluate},
  {"getControlPoint",       l_math_BezierCurve_getControlPoint},
  {"getControlPointCount",  l_math_BezierCurve_getControlPointCount},
  {"getDegree",             l_math_BezierCurve_getDegree},
  {"getDerivative",         l_math_BezierCurve_getDerivative},
  {"insertControlPoint",    l_math_BezierCurve_insertControlPoint},
  {"render",                l_math_BezierCurve_render},
  {"rotate",                l_math_BezierCurve_rotate},
  {"scale",                 l_math_BezierCurve_scale},
  {"setControlPoint",       l_math_BezierCurve_setControlPoint},
  {"translate",             l_math_BezierCurve_translate},
  {NULL, NULL}
};


void l_math_bezier_register(lua_State* state) {
  l_tools_registerFuncsInModule(state, "math", bezierFreeFuncs);
  moduleData.bezierMT  = l_tools_makeTypeMetatable(state, bezierMetatableFuncs);
}

l_checkTypeFn(l_math_isBezierCurve, moduleData.bezierMT)
l_toTypeFn(l_math_toBezierCurve, math_BezierCurve)
