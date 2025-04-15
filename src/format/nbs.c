#include "nbs.h"

/** Read 8-bit int in little endian. */
static i08 readInt8(NBSReader *reader) {
  size_t p = reader->cursor;
  if (reader->error)
    // If previous function thrown an error,
    // then do nothing.
    return 0;
  if (p + 1 < reader->size) {
    reader->error = 0;
    reader->cursor++;
    return reader->buffer[p];
  }
  // If out of bound, then do nothing.
  reader->error = NBS_OUTOFBOUND;
  return 0;
}

/** Read 16-bit int in little endian. */
static i16 readInt16LE(NBSReader *reader) {
  size_t p = reader->cursor;
  if (reader->error)
    return 0;
  if (p + 2 < reader->size) {
    reader->error = 0;
    reader->cursor += 2;
    return (u8)(reader->buffer[p])
      | (u8)(reader->buffer[p + 1]) << 8;
  }
  reader->error = NBS_OUTOFBOUND;
  return 0;
}

/** Read 32-bit int in little endian. */
static i32 readInt32LE(NBSReader *reader) {
  size_t p = reader->cursor;
  if (reader->error)
    return 0;
  if (p + 4 < reader->size) {
    reader->error = 0;
    reader->cursor += 4;
    return (u8)(reader->buffer[p])
      | (u8)(reader->buffer[p + 1]) << 8
      | (u8)(reader->buffer[p + 2]) << 16
      | (u8)(reader->buffer[p + 3]) << 24;
  }
  reader->error = NBS_OUTOFBOUND;
  return 0;
}

/** Read a string with specified length. */
static void readLengthedString(NBSReader *reader, char **string) {
  i32 length;

  if (reader->error) {
    // If previous function throws an error, then do nothing.
    *string = NULL;
    return;
  }

  // Try to read the length of the string.
  length = readInt32LE(reader);
  if (reader->error) {
    *string = NULL;
    return;
  }

  if (reader->cursor + length >= reader->size) {
    // If out of bound, then do nothing.
    reader->error = NBS_OUTOFBOUND;
    *string = NULL;
    return;
  }

  *string = malloc(length + 1);
  if (!*string) {
    reader->error = NBS_ALLOCFAILED;
    return;
  }

  reader->cursor += length;
  memcpy(*string, reader->buffer + reader->cursor, length);
  (*string)[length] = 0;
}

/** Read NBS file header. */
static void readHeader(NBSReader *reader, NBSHeader *header) {
  header->songLengthOld = readInt16LE(reader);
  header->version = readInt8(reader);
  header->instCtr = readInt8(reader);
  header->songLength = readInt16LE(reader);
  header->layerCtr = readInt16LE(reader);
  readLengthedString(reader, &header->name);
  readLengthedString(reader, &header->author);
  readLengthedString(reader, &header->originAuthor);
  readLengthedString(reader, &header->desc);
  header->tempo = readInt16LE(reader);
  header->autoSave = readInt8(reader);
  header->autoSaveDur = readInt8(reader);
  header->timeSign = readInt8(reader);
  header->minutes = readInt32LE(reader);
  header->lc = readInt32LE(reader);
  header->rc = readInt32LE(reader);
  header->noteAdded = readInt32LE(reader);
  header->noteRemoved = readInt32LE(reader);
  readLengthedString(reader, &header->midiName);
  header->loop = readInt8(reader);
  header->maxLoopCtr = readInt8(reader);
  header->loopStartTick = readInt16LE(reader);
}

/** Read a single note. */
static void readNoteBlock(NBSReader *reader, NBSNoteBlock *note) {
  note->instrument = readInt8(reader);
  note->key = readInt8(reader);
  note->velocity = readInt8(reader);
  note->panning = readInt8(reader);
  note->pitch = readInt16LE(reader);
}

/** Read an effective tick. */
static void readEffectiveTick(
  NBSReader *reader,
  NBSTickEffective *tickData,
  int *lastTick
) {
  NBSNoteBlock note;
  size_t noteCtr;
  int tickJmp = readInt16LE(reader)
    , curTick, layerJmp;
  if (!tickJmp || reader->error)
    return;
  curTick = *lastTick + tickJmp;
  *lastTick = curTick;
  tickData->tick = curTick;
  memset(&tickData->notes, 0, sizeof(Vector_t));
  vec_init(&tickData->notes, sizeof(NBSNoteBlock));
  while (1) {
    // Get total note count.
    layerJmp = readInt16LE(reader);
    if (reader->error)
      return;
    if (!layerJmp)
      break;
    readNoteBlock(reader, &note);
    if (reader->error)
      return;
    vec_push(&tickData->notes, &note);
  }
  vec_size(&tickData->notes, &noteCtr);
  tickData->noteCtr = (i32)noteCtr;
  return;
}

/** Read NBS file without layer and custom instuments. */
i32 readNBSFile(const char *buffer, size_t fileSize, NBS *nbs, i32 *err) {
  size_t p = 0;
  int tick = -1, effectiveTickCtr = 0;
  NBSTickEffective tickData;
  NBSReader reader = {
    .buffer = buffer,
    .cursor = 0,
    .size = fileSize,
    .error = 0
  };
  if (!buffer || !nbs) {
    *err = NBS_NULLPOINTER;
    return 0;
  }
  readHeader(&reader, &nbs->header);
  if (reader.error)
    goto ErrorRet;
  memset(&nbs->ticks, 0, sizeof(Vector_t));
  vec_init(&nbs->ticks, sizeof(NBSTickEffective));
  while (1) {
    p = reader.cursor;
    readEffectiveTick(&reader, &tickData, &tick);
    if (reader.error)
      goto ErrorRet;
    if (reader.cursor - p == 2)
      break;
    vec_push(&nbs->ticks, &tickData);
    effectiveTickCtr++;
  }
  nbs->tickCtr = effectiveTickCtr;
  return 1;

ErrorRet:
  if (err)
    *err = reader.error;
  return 0;
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
