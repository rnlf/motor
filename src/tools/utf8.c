/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include "utf8.h"

uint32_t utf8_scan(char const ** utf8) {
  unsigned char const * u = (unsigned char const*)*utf8;
  uint32_t cp = 0;
  int more = 0;
  if(*u < 0x80) {
    cp = (*u) & 0x7F;
    more = 0;
  } else if(((*u) & 0xE0) == 0xC0) {
    cp = (*u) & 0x1F;
    more = 1;
  } else if(((*u) & 0xF0) == 0xE0) {
    cp = (*u) & 0x0F;
    more = 2;
  } else if(((*u) & 0xF8) == 0xF0) {
    cp = (*u) & 0x07;
    more = 3;
  }

  for(int i = 0; i < more; ++i) {
    cp = (cp << 6) | (u[i+1] & 0x3F);
  }

  *utf8 += 1 + more;

  return cp;
}
