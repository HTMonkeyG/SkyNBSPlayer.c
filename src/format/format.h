#ifndef __FORMAT_H__
#define __FORMAT_H__

#include "../macros.h"
#include "../vector.h"
#include "nbs.h"
#include "abc.h"

typedef enum {
  C0 = 0x0001,
  D0 = 0x0002,
  E0 = 0x0004,
  F0 = 0x0008,
  G0 = 0x0010,
  A0 = 0x0020,
  B0 = 0x0040,
  C1 = 0x0080,
  D1 = 0x0100,
  E1 = 0x0200,
  F1 = 0x0400,
  G1 = 0x0800,
  A1 = 0x1000,
  B1 = 0x2000,
  C2 = 0x4000,
  INVALID = -1
} SkyNoteMasks_t;

typedef struct {
  u32 tick;
  u16 keyDown;
  u16 keyUp;
} SkyMusicTick_t;

typedef struct {
  float tps;
  char *name;
  char *author;
  char *originalAuthor;
  Vector_t ticks;
} GeneralSongTicks_t;

static const SkyNoteMasks_t CVT[25] = {
  C0, INVALID, D0, INVALID, E0, F0, INVALID, G0, INVALID, A0, INVALID, B0,
  C1, INVALID, D1, INVALID, E1, F1, INVALID, G1, INVALID, A1, INVALID, B1,
  C2
};

i32 readSongFile(char *buffer, size_t fileSize, GeneralSongTicks_t *file);
i32 freeSongFile(GeneralSongTicks_t *file);

#endif