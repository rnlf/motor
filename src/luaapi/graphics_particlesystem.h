#pragma once

#include <lua.h>

#include "../graphics/particlesystem.h"

typedef struct {
  int textureRef;
  int quadTableRef;
  graphics_ParticleSystem particleSystem;
} l_graphics_ParticleSystem;


void l_graphics_particlesystem_register(lua_State* state);
bool l_graphics_isParticleSystem(lua_State* state, int index);
l_graphics_ParticleSystem* l_graphics_toParticleSystem(lua_State* state, int index);
