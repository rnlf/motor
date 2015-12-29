/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <lualib.h>
#include "../motor_config.h"

int l_boot(lua_State* state, motor_Config *config);
