#include <lauxlib.h>
#include "bezier.h"
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


static luaL_Reg const bezierFreeFuncs[] = {
  {"newBezierCurve", l_math_newBezierCurve},
  {NULL, NULL}
};


static luaL_Reg const bezierMetatableFuncs[] = {
  {"__gc",    l_math_gcBezierCurve},
  {NULL, NULL}
};


void l_math_bezier_register(lua_State* state) {
  l_tools_registerFuncsInModule(state, "math", bezierFreeFuncs);
  moduleData.bezierMT  = l_tools_makeTypeMetatable(state, bezierMetatableFuncs);
}

l_checkTypeFn(l_math_isBezierCurve, moduleData.bezierMT)
l_toTypeFn(l_math_toBezierCurve, math_BezierCurve)
