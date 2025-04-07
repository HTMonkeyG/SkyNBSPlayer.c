#ifndef _INC_SKY_NOTES_
#define _INC_SKY_NOTES_

#include <windows.h>
#include "nbs.h"
#include "macros.h"
#include "vector.h"

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
  /** Use 1000tps. */
  i8 highTps;
  /** Randomly shift every note along timeline. */
  i8 randomShift;
  /** Max shift time in ms. */
  i8 shiftStrength;
  /** Distribution of the shift operation. */
  i8 shiftType;
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

static const SkyNoteMasks_t CVT[25] = {
  C0, INVALID, D0, INVALID, E0, F0, INVALID, G0, INVALID, A0, INVALID, B0,
  C1, INVALID, D1, INVALID, E1, F1, INVALID, G1, INVALID, A1, INVALID, B1,
  C2
};

void sendTick(HWND hWnd, SkyMusicTick_t *tick);
void buildKeysFrom(NBSTickEffective *t, u16 *keyDown, u16 *keyUp);
int buildTicksFrom(SkyAutoPlayOptions_t *options, NBS *nbs, Vector_t *v);

#endif