#ifndef __NOTE_H__
#define __NOTE_H__

#include "format/nbs.h"
#include "format/format.h"
#include "macros.h"
#include "vector.h"

typedef struct {
  /** Game average frame rate. */
  i32 fps;
  /** Use 1000tps. */
  i8 highTps;
  /** Randomly shift every note along timeline. */
  i8 randomShift;
  /** Max shift time in ms. */
  i8 shiftStrength;
  /** Distribution of the shift operation. */
  i8 shiftType;
  /** Restrict all notes to 2-Octave limitaion of Sky. */
  i8 allowClamp;
} SkyAutoPlayOptions_t;

static const u16 KEYS[15] = {
  21, 22, 23, 24, 25,
  35, 36, 37, 38, 39,
  49, 50, 51, 52, 53
};

static const u8 KEYCODES[15] = {
  'Y', 'U', 'I', 'O', 'P',
  'H', 'J', 'K', 'L', ';',
  'N', 'M', ',', '.', '/'
};

int buildTicksFrom(
  SkyAutoPlayOptions_t *options,
  GeneralSongTicks_t *file,
  Vector_t *v
);

#endif