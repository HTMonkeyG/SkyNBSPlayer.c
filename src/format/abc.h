#ifndef __ABC_H__
#define __ABC_H__

#include <stdio.h>
#include <string.h>

#include "../macros.h"
#include "../cJSON/cJSON.h"
#include "../vector.h"

typedef struct {
  /** In milliseconds. */
  u32 time;
  u16 note1;
  u16 note2;
} ABCTick_t;

typedef struct {
  char *name;
  char *author;
  char *oriAuthor;
  i8 isComposed;
  i8 bitsPerPage;
  i32 pitchLevel;
  f64 bpm;
  Vector_t ticks;
} SkyStudioABC;

i32 readJsonABC(const char *input, SkyStudioABC *abc);
void freeABC(SkyStudioABC *abc);

#endif