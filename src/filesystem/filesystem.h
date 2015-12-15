#pragma once

#include <stdbool.h>
#include <dirent.h>

void filesystem_init(void);


bool filesystem_write(char const* filename, char const* data, int size);
int filesystem_read(char const* filename, int max, char** output);
bool filesystem_isFile(char const* filename);
void filesystem_setIdentity(char const* name, bool first);
DIR* filesystem_openDir(char const* dirname, bool identity);
