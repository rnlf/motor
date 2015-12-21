#include <errno.h>
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>


static struct {
  char * identityPath;
  bool identityFirst;
  char * nameBuffer;
  size_t nameBufferLength;
} moduleData;


static void buildFilename(char const* dir, char const* filename) {
  size_t dlen = strlen(dir);
  size_t len = dlen + strlen(filename) + 2;
  if(moduleData.nameBufferLength < len) {
    moduleData.nameBuffer = realloc(moduleData.nameBuffer, len);
  }

  strcpy(moduleData.nameBuffer, dir);
  if(dir[dlen-1] != '/' && filename[0] != '/') {
    moduleData.nameBuffer[dlen] = '/';
    ++dlen;
  }
  strcpy(moduleData.nameBuffer + dlen, filename);

  if(moduleData.nameBuffer[strlen(moduleData.nameBuffer)-1] == '/') {
    moduleData.nameBuffer[strlen(moduleData.nameBuffer)-1] = 0;
  }
}


static bool statFile(char const* dir, char const* filename, struct stat *out) {

  buildFilename(dir, filename);
  int st = stat(moduleData.nameBuffer, out);
  return st == 0;
}


static bool findFileRW(char const* filename, struct stat *out) {
  if(moduleData.identityFirst) {
    if(statFile(moduleData.identityPath, filename, out)) {
      return true;
    }
    return statFile("/love", filename, out);
  }

  if(statFile("/love", filename, out)) {
    return true;
  } 
  return moduleData.identityPath != 0 && statFile(moduleData.identityPath, filename, out);
}


void filesystem_init(void) {

}


void filesystem_setIdentity(char const* name, bool first) {
  moduleData.identityPath = realloc(moduleData.identityPath, strlen(name)+7);
  strcpy(moduleData.identityPath, "/save/");
  strcpy(moduleData.identityPath + 6, name);
  moduleData.identityFirst = first;
  mkdir("/save", 0755);
  mkdir(moduleData.identityPath, 0755);
}


int filesystem_read(char const* filename, int max, char** output) {
  struct stat status;
  if(!findFileRW(filename, &status)) {
    return -1;
  }

  FILE* infile = fopen(moduleData.nameBuffer, "r");
  if(!infile) {
    return -1;
  }

  int count = status.st_size;
  if(max > -1 && max < count) {
    count = max;
  }

  *output = malloc(count + 1);
  int rcount = fread(*output, 1, count, infile);
  fclose(infile);
  (*output)[count] = 0;
  return rcount;
}


bool filesystem_write(char const* filename, char const* data, int size) {

  if(moduleData.identityPath == 0) {
    return false;
  }
  
  FILE* outfile = fopen(moduleData.nameBuffer, "w");
  if(!outfile) {
    return false;
  }

  int len = strlen(data);
  if(size != -1 && size < len) {
    len = size;
  }

  int ret = fwrite(data, 1, len, outfile);

  fclose(outfile);

  return ret == len;
}


DIR* filesystem_openDir(char const* dirname, bool identity) {
  struct stat status;

  if(identity) {
    if(!statFile(moduleData.identityPath, dirname, &status)) {
      return 0;
    }
  } else {
    if(!statFile("/love", dirname, &status)) {
      return 0;
    }
  }

  if(!S_ISDIR(status.st_mode)) {
    return 0;
  }

  return opendir(moduleData.nameBuffer);
}


bool filesystem_isFile(char const* filename) {
  struct stat status;
  bool exist = findFileRW(filename, &status);
  return exist && S_ISREG(status.st_mode);
}


bool filesystem_isDirectory(char const* filename) {
  struct stat status;
  bool exist = findFileRW(filename, &status);
  return exist && S_ISDIR(status.st_mode);
}


bool filesystem_getLastModified(char const* filename, double *out) {
  struct stat status;
  if(!findFileRW(filename, &status)) {
    return false;
  }

  *out = (double)status.st_mtim.tv_sec + ((double)status.st_mtim.tv_sec) / 1000000000.0;

  return true;
}


FILE* filesystem_fopen(char const* filename, char const* mode) {
  return fopen(filename, mode);
}
