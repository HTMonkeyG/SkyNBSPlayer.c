#include "nbs.h"

/** Read 16-bit int in little endian. */
static short readInt16LE(const char *buffer, size_t cursor) {
  return buffer[cursor] | buffer[cursor + 1] << 8;
}

/** Read 32-bit int in little endian. */
static int readInt32LE(const char *buffer, size_t cursor) {
  return buffer[cursor] | buffer[cursor + 1] << 8 | buffer[cursor + 2] << 16 | buffer[cursor + 3] << 24;
}

/** Read a string with specified length. */
static size_t readLengthedString(const char *buffer, size_t cursor, char **string) {
  size_t p = cursor;
  int length = readInt32LE(buffer, p);
  p += 4;
  *string = malloc(length + 1);
  memcpy(*string, buffer + p, length);
  (*string)[length] = 0;
  return p + length;
}

/** Read NBS file header. */
static size_t readHeader(const char *buffer, size_t cursor, NBSHeader *header) {
  size_t p = cursor;
  header->songLengthOld = readInt16LE(buffer, p), p += 2;
  header->version = buffer[p], p++;
  header->instCtr = buffer[p], p++;
  header->songLength = readInt16LE(buffer, p), p += 2;
  header->layerCtr = readInt16LE(buffer, p), p += 2;
  p = readLengthedString(buffer, p, &header->name);
  p = readLengthedString(buffer, p, &header->author);
  p = readLengthedString(buffer, p, &header->originAuthor);
  p = readLengthedString(buffer, p, &header->desc);
  header->tempo = readInt16LE(buffer, p), p += 2;
  header->autoSave = buffer[p], p++;
  header->autoSaveDur = buffer[p], p++;
  header->timeSign = buffer[p], p++;
  header->minutes = readInt32LE(buffer, p), p += 4;
  header->lc = readInt32LE(buffer, p), p += 4;
  header->rc = readInt32LE(buffer, p), p += 4;
  header->noteAdded = readInt32LE(buffer, p), p += 4;
  header->noteRemoved = readInt32LE(buffer, p), p += 4;
  p = readLengthedString(buffer, p, &header->midiName);
  header->loop = buffer[p], p++;
  header->maxLoopCtr = buffer[p], p++;
  header->loopStartTick = readInt16LE(buffer, p), p += 2;
  return p;
}

/** Read a single note. */
static size_t readNoteBlock(const char *buffer, size_t cursor, NBSNoteBlock *note) {
  size_t p = cursor;
  note->instrument = buffer[p], p++;
  note->key = buffer[p], p++;
  note->velocity = buffer[p], p++;
  note->panning = buffer[p], p++;
  note->pitch = readInt16LE(buffer, p), p += 2;
  return p;
}

/** Read an effective tick. */
static size_t readEffectiveTick(const char *buffer, size_t cursor, NBSTickEffective *tickData, int *lastTick) {
  size_t p = cursor, q;
  int tickJmp = readInt16LE(buffer, p), curTick, noteCtr = 0, layerJmp;
  p += 2;
  if (!tickJmp)
    return p;
  curTick = *lastTick + tickJmp;
  tickData->tick = curTick;
  *lastTick = curTick;
  q = p;
  while (1) {
    layerJmp = readInt16LE(buffer, p);
    p += 2;
    if (!layerJmp)
      break;
    p += 6;
    noteCtr++;
  }
  tickData->noteCtr = noteCtr;
  tickData->notes = malloc(noteCtr * sizeof(NBSNoteBlock));
  p = q;
  noteCtr = 0;
  while (1) {
    layerJmp = readInt16LE(buffer, p);
    p += 2;
    if (!layerJmp)
      break;
    p = readNoteBlock(buffer, p, &tickData->notes[noteCtr]);
    noteCtr++;
  }
  return p;
}

/** Read NBS file without layer and custom instuments. */
size_t readNBSFile(const char *buffer, size_t cursor, NBS *nbs) {
  size_t p = cursor, q;
  int tick = -1, effectiveTickCtr = 0;
  NBSTickEffective **t = &nbs->ticks, *r;
  p = readHeader(buffer, cursor, &nbs->header);
  *t = malloc(sizeof(NBSTickEffective));
  (*t)->prev = NULL;
  while (1) {
    q = readEffectiveTick(buffer, p, *t, &tick);
    if (q - p == 2) {
      free(*t);
      *t = NULL;
      break;
    }
    p = q;
    (*t)->next = malloc(sizeof(NBSTickEffective));
    r = *t;
    t = &(*t)->next;
    (*t)->prev = r;
    effectiveTickCtr++;
  }
  nbs->effectiveTick = effectiveTickCtr;
  return p;
}

/** Free an NBS struct. */
void freeNBSFile(NBS *nbs) {
  free(nbs->header.name);
  free(nbs->header.author);
  free(nbs->header.originAuthor);
  free(nbs->header.desc);
  free(nbs->header.midiName);
  NBSTickEffective *r, *t;
  for (t = nbs->ticks; t; t = t->next) {
    free(t->notes);
    if (t)
      r = t;
  }
  for (t = r; t; t = t->prev)
    if (t->next)
      free(t->next);
  free(nbs->ticks);
}
