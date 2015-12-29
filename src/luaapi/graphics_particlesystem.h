/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

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
