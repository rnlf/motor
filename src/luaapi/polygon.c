#include "tools.h"

int l_geometry_read_vertices(lua_State* state, int offset, float **vertices, int minVerts) {
  return l_tools_readNumbers(state, offset, vertices, minVerts, 2);
}
