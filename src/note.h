#ifndef __NOTE_H__
#define __NOTE_H__

#include <math.h>

#include "format/nbs.h"
#include "format/format.h"
#include "macros.h"
#include "vector.h"

typedef struct {
  /** Minimun allowed inteval between ticks. */
  i32 minIntevalMs;
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

int buildTicksFrom(
  SkyAutoPlayOptions_t *options,
  GeneralSongTicks_t *file,
  Vector_t *v
);

#endif