/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include <stdbool.h>
#include <dirent.h>
#include <stdio.h>

void filesystem_init(void);


bool filesystem_write(char const* filename, char const* data, int size);
int filesystem_read(char const* filename, int max, char** output);
bool filesystem_isFile(char const* filename);
bool filesystem_isDirectory(char const* filename);
void filesystem_setIdentity(char const* name, bool first);
DIR* filesystem_openDir(char const* dirname, bool identity);
bool filesystem_getLastModified(char const* filename, double *out);
char const* filesystem_locateReadableFile(char const* filename);
char const* filesystem_locateWritableFile(char const* filename);
