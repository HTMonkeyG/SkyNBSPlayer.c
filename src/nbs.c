#include "nbs.h"

/** Read 8-bit int in little endian. */
static i08 readInt8(NBSReader *reader, i08 *err) {
  size_t p = reader->cursor;
  if (*err)
    // If previous function thrown an error,
    // then do nothing.
    return 0;
  if (p + 1 < reader->size) {
    *err = 0;
    reader->cursor++;
    return reader->buffer[p];
  }
  // If out of bound, then do nothing.
  *err = 1;
  return 0;
}

/** Read 16-bit int in little endian. */
static i16 readInt16LE(NBSReader *reader, i08 *err) {
  size_t p = reader->cursor;
  if (*err)
    return 0;
  if (p + 2 < reader->size) {
    *err = 0;
    reader->cursor += 2;
    return (u8)(reader->buffer[p])
      | (u8)(reader->buffer[p + 1]) << 8;
  }
  *err = 1;
  return 0;
}

/** Read 32-bit int in little endian. */
static i32 readInt32LE(NBSReader *reader, i08 *err) {
  size_t p = reader->cursor;
  if (*err)
    return 0;
  if (p + 4 < reader->size) {
    *err = 0;
    reader->cursor += 4;
    return (u8)(reader->buffer[p])
      | (u8)(reader->buffer[p + 1]) << 8
      | (u8)(reader->buffer[p + 2]) << 16
      | (u8)(reader->buffer[p + 3]) << 24;
  }
  *err = 1;
  return 0;
}

/** Read a string with specified length. */
static void readLengthedString(NBSReader *reader, char **string, i08 *err) {
  i32 length;

  if (*err) {
    // If previous function throws an error, then do nothing.
    *string = NULL;
    return;
  }

  // Try to read the length of the string.
  length = readInt32LE(reader, err);
  if (*err) {
    *string = NULL;
    return;
  }

  if (reader->cursor + length >= reader->size) {
    // If out of bound, then do nothing.
    *err = 1;
    *string = NULL;
    return;
  }

  *string = malloc(length + 1);
  if (!*string) {
    *err = 1;
    return;
  }

  reader->cursor += length;
  memcpy(*string, reader->buffer + reader->cursor, length);
  (*string)[length] = 0;
}

/** Read NBS file header. */
static void readHeader(NBSReader *reader, NBSHeader *header, i08 *err) {
  header->songLengthOld = readInt16LE(reader, err);
  header->version = readInt8(reader, err);
  header->instCtr = readInt8(reader, err);
  header->songLength = readInt16LE(reader, err);
  header->layerCtr = readInt16LE(reader, err);
  readLengthedString(reader, &header->name, err);
  readLengthedString(reader, &header->author, err);
  readLengthedString(reader, &header->originAuthor, err);
  readLengthedString(reader, &header->desc, err);
  header->tempo = readInt16LE(reader, err);
  header->autoSave = readInt8(reader, err);
  header->autoSaveDur = readInt8(reader, err);
  header->timeSign = readInt8(reader, err);
  header->minutes = readInt32LE(reader, err);
  header->lc = readInt32LE(reader, err);
  header->rc = readInt32LE(reader, err);
  header->noteAdded = readInt32LE(reader, err);
  header->noteRemoved = readInt32LE(reader, err);
  readLengthedString(reader, &header->midiName, err);
  header->loop = readInt8(reader, err);
  header->maxLoopCtr = readInt8(reader, err);
  header->loopStartTick = readInt16LE(reader, err);
}

/** Read a single note. */
static void readNoteBlock(NBSReader *reader, NBSNoteBlock *note, i08 *err) {
  note->instrument = readInt8(reader, err);
  note->key = readInt8(reader, err);
  note->velocity = readInt8(reader, err);
  note->panning = readInt8(reader, err);
  note->pitch = readInt16LE(reader, err);
}

/** Read an effective tick. */
static void readEffectiveTick(
  NBSReader *reader,
  NBSTickEffective *tickData,
  int *lastTick,
  i08 *err
) {
  NBSNoteBlock note;
  size_t noteCtr;
  int tickJmp = readInt16LE(reader, err)
    , curTick, layerJmp;
  if (!tickJmp || *err)
    return;
  curTick = *lastTick + tickJmp;
  *lastTick = curTick;
  tickData->tick = curTick;
  memset(&tickData->notes, 0, sizeof(Vector_t));
  vec_init(&tickData->notes, sizeof(NBSNoteBlock));
  while (1) {
    // Get total note count.
    layerJmp = readInt16LE(reader, err);
    if (*err)
      return;
    if (!layerJmp)
      break;
    readNoteBlock(reader, &note, err);
    if (*err)
      return;
    vec_push(&tickData->notes, &note);
  }
  vec_size(&tickData->notes, &noteCtr);
  tickData->noteCtr = (i32)noteCtr;
  return;
}

/** Read NBS file without layer and custom instuments. */
i32 readNBSFile(const char *buffer, size_t fileSize, NBS *nbs) {
  size_t p = 0;
  int tick = -1, effectiveTickCtr = 0;
  i08 err = 0;
  NBSTickEffective tickData;
  NBSReader reader = {
    .buffer = buffer,
    .cursor = 0,
    .size = fileSize
  };
  readHeader(&reader, &nbs->header, &err);
  if (err)
    return 0;
  memset(&nbs->ticks, 0, sizeof(Vector_t));
  vec_init(&nbs->ticks, sizeof(NBSTickEffective));
  while (1) {
    p = reader.cursor;
    readEffectiveTick(&reader, &tickData, &tick, &err);
    if (err)
      return 0;
    if (reader.cursor - p == 2)
      break;
    vec_push(&nbs->ticks, &tickData);
    effectiveTickCtr++;
  }
  nbs->tickCtr = effectiveTickCtr;
  return 1;
}

/** Free an NBS struct. */
void freeNBSFile(NBS *nbs) {
  size_t r;
  NBSTickEffective *p;
  free(nbs->header.name);
  free(nbs->header.author);
  free(nbs->header.originAuthor);
  free(nbs->header.desc);
  free(nbs->header.midiName);
  vec_size(&nbs->ticks, &r);
  for (size_t i = 0; i < r; i++) {
    vec_at(&nbs->ticks, i, (void **)&p);
    vec_free(&p->notes);
  }
  vec_free(&nbs->ticks);
}
