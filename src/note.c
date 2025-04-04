#include "note.h"

void sendKeySet(HWND hWnd, SkyNoteKeys_t *notes, int length) {
  for (int i = 0; i < length; i++) {
    SendMessageW(hWnd, WM_KEYDOWN, KEYCODES[notes[i]], KEYS[notes[i]] << 16);
    Sleep(20);
    SendMessageW(hWnd, WM_KEYUP, KEYCODES[notes[i]], (KEYS[notes[i]] << 16) | 1 | (0B110 << 29));
  }
}

void sendTick(HWND hWnd, SkyMusicTick_t *tick) {
  for (int i = 0, j = 1; i < 15; i++, j <<= 1) {
    (tick->keyDown & j) && SendMessageW(hWnd, WM_KEYDOWN, KEYCODES[i], KEYS[i] << 16);
    (tick->keyUp & j) && SendMessageW(hWnd, WM_KEYUP, KEYCODES[i], (KEYS[i] << 16) | 1 | (0B110 << 29));
  }
}

int buildKeysFrom(NBSTickEffective *tick, SkyNoteKeys_t *keys, int length) {
  NBSNoteBlock *note;
  int j = 0;
  for (int i = 0; i < min(tick->noteCtr, length); i++) {
    note = &tick->notes[i];
    if (note->instrument == 0
      && 39 <= note->key
      && note->key <= 63) {
      if (CVT[note->key - 39] != INVALID)
        keys[j] = CVT[note->key - 39], j++;
    }
  }
  return j;
}

void buildKeysFrom_(NBSTickEffective *t, u16 *keyDown, u16 *keyUp) {
  NBSNoteBlock *note;
  *keyDown = *keyUp = 0;
  for (int i = 0; i < t->noteCtr; i++) {
    note = &t->notes[i];
    if (note->instrument == 0
      && 39 <= note->key
      && note->key <= 63) {
      if (CVT[note->key - 39] != INVALID) {
        *keyDown |= CVT[note->key - 39];
        *keyUp |= CVT[note->key - 39];
      }
    }
  }
}

/** Convert NBS to key event ticks. */
int buildTicksFrom(SkyAutoPlayOptions_t *options, NBS *nbs, Vector_t *v) {
  f32 tempo = (f32)nbs->header.tempo / 100.
    , tps = options->highTps ? 1000 : 100;
  u32 lastActiveEvent[15] = {0}
    , time = 0
    , th;
  u16 keyDown, keyUp;
  NBSTickEffective *tick = nbs->ticks;
  SkyMusicTick_t mtr, mti, lastTick;

  vec_init(v, sizeof(SkyMusicTick_t));
  th = (int)((float)tick->tick / tempo * tps);
  while (tick) {
    // Simulate tick by tick
    if (time < th)
      continue;

    // Build real tick
    buildKeysFrom_(tick, &keyDown, &keyUp);
    mtr.keyDown = keyDown;
    mtr.keyUp = 0;

    // Build inteval tick
    // 10ms inteval
    mti.tick = mtr.tick + tps / 100;

    // Next NBS tick
    tick = tick->next;
    vec_push(v, &mtr);
    vec_push(v, &mti);
  }
}
