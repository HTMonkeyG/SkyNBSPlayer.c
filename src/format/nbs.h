#ifndef __NBS_H__
#define __NBS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../vector.h"

typedef struct {
  const char *buffer;
  size_t cursor;
  size_t size;
  i32 error;
} NBSReader;

typedef struct {
  short songLengthOld;
  char version;
  char instCtr;
  short songLength;
  short layerCtr;
  char *name;
  char *author;
  char *originAuthor;
  char *desc;
  short tempo;
  char autoSave;
  char autoSaveDur;
  char timeSign;
  int minutes;
  int lc;
  int rc;
  int noteAdded;
  int noteRemoved;
  char *midiName;
  char loop;
  char maxLoopCtr;
  short loopStartTick;
} NBSHeader;

typedef struct {
  char instrument;
  char key;
  char velocity;
  unsigned char panning;
  short pitch;
} NBSNoteBlock;

typedef struct {
  int tick;
  int ticksToNext;
  int noteCtr;
  Vector_t notes;
} NBSTickEffective;

typedef struct {
  NBSHeader header;
  int tickCtr;
  Vector_t ticks;
} NBS;

#define NBS_OUTOFBOUND  0x01
#define NBS_ALLOCFAILED 0x02
#define NBS_NULLPOINTER 0x03

i32 readNBSFile(const char *buffer, size_t fileSize, NBS *nbs, i32 *err);
void freeNBSFile(NBS *nbs);

#endif
