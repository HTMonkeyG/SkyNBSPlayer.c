#include "format.h"

static void buildKeysFrom(NBSTickEffective *t, u16 *keyDown, u16 *keyUp) {
  NBSNoteBlock *note;
  *keyDown = *keyUp = 0;
  for (int i = 0; i < t->noteCtr; i++) {
    vec_at(&t->notes, i, (void **)&note);
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

static i32 convertFromNBS(NBS *nbs, GeneralSongTicks_t *file) {
  size_t s;
  NBSTickEffective *tick;
  SkyMusicTick_t currentTick;

  if (!nbs || !file)
    return 0;

  file->tps = (f32)nbs->header.tempo / 100.;
  file->author = malloc(strlen(nbs->header.author) + 1);
  file->originalAuthor = malloc(strlen(nbs->header.originAuthor) + 1);
  file->name = malloc(strlen(nbs->header.name) + 1);
  strcpy(file->author, nbs->header.author);
  strcpy(file->originalAuthor, nbs->header.originAuthor);
  strcpy(file->name, nbs->header.name);
  vec_size(&nbs->ticks, &s);

  for (size_t i = 0; i < s; i++) {
    vec_at(&nbs->ticks, i, (void **)&tick);
    currentTick.tick = tick->tick;
    buildKeysFrom(tick, &currentTick.keyDown, &currentTick.keyUp);
    // Do nothing when no valid note.
    if (currentTick.keyDown || currentTick.keyUp)
      vec_push(&file->ticks, &currentTick);
  }

  return 1;
}

static i32 convertFromABC(SkyStudioABC *abc, GeneralSongTicks_t *file) {
  size_t s;
  ABCTick_t *tick;
  SkyMusicTick_t currentTick;

  if (!abc || !file)
    return 0;

  file->tps = 1000.;
  file->author = malloc(strlen(abc->author) + 1);
  file->originalAuthor = malloc(strlen(abc->oriAuthor) + 1);
  file->name = malloc(strlen(abc->name) + 1);
  strcpy(file->author, abc->author);
  strcpy(file->originalAuthor, abc->oriAuthor);
  strcpy(file->name, abc->name);
  vec_size(&abc->ticks, &s);

  for (size_t i = 0; i < s; i++) {
    vec_at(&abc->ticks, i, (void **)&tick);
    currentTick.tick = tick->time;
    currentTick.keyDown = tick->note1 | tick->note2;
    currentTick.keyUp = tick->note1 | tick->note2;
    vec_push(&file->ticks, &currentTick);
  }

  return 1;
}

i32 readSongFile(char *buffer, size_t fileSize, GeneralSongTicks_t *file) {
  i32 err, result;
  NBS nbs;
  SkyStudioABC abc;

  if (!buffer || !file)
    return 0;
  
  vec_init(&file->ticks, sizeof(SkyMusicTick_t));

  // Try to read and convert.
  if (readNBSFile(buffer, fileSize, &nbs, &err)) {
    result = convertFromNBS(&nbs, file);
    freeNBSFile(&nbs);
  } else if (readJsonABC(buffer, &abc)) {
    result = convertFromABC(&abc, file);
    freeABC(&abc);
  } else
    // Not a valid format.
    return 0;
  
  return result;
}

i32 freeSongFile(GeneralSongTicks_t *file) {
  free(file->author);
  free(file->name);
  free(file->originalAuthor);
  return vec_free(&file->ticks) != NULL;
}