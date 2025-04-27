#include "abc.h"

static i32 readJsonString(
  const cJSON *const obj,
  const char *name,
  char **result
) {
  cJSON *kv = cJSON_GetObjectItemCaseSensitive(obj, name);
  char *str;
  if (!cJSON_IsString(kv)) {
    *result = NULL;
    return 0;
  }
  str = cJSON_GetStringValue(kv);
  *result = malloc(strlen(str) + 1);
  strcpy(*result, str);
  return 1;
}

static i32 readJsonNumber(
  const cJSON *const obj,
  const char *name,
  double *result
) {
  cJSON *kv = cJSON_GetObjectItemCaseSensitive(obj, name);
  if (!result || !cJSON_IsNumber(kv))
    return 0;
  *result = cJSON_GetNumberValue(kv);
  return 1;
}

static i32 readJsonNote(const char *str, ABCTick_t *result) {
  char buffer[16] = {0}
    , *p
    , l;
  i32 type, note, mask;
  
  if (!str || !result)
    return 0;

  strncpy(buffer, str, 15);
  l = strlen(buffer);
  
  for (char i = 0; i < l; i++) {
    if (!strncmp(buffer + i, "Key", 3)) {
      memset(buffer + i, 0, 3);
      type = strtol(buffer, &p, 10);
      note = strtol(buffer + i + 3, &p, 10);
      if (!note)
        return 0;
      mask = 1 << note;
      if (!type || type == 1)
        result->note1 = mask;
      else if (type == 2)
        result->note2 = mask;
      else
        return 0;
      return 1;
    }
  }

  return 0;
}

static i32 readJsonABCHeader(const cJSON *const content, SkyStudioABC *abc) {
  double t1, t2;
  if (
    !readJsonNumber(content, "bpm", &abc->bpm)
    || !readJsonNumber(content, "pitchLevel", &t1)
    || !readJsonNumber(content, "bitsPerPage", &t2)
  )
    return 0;
  
  readJsonString(content, "name", &abc->name);
  readJsonString(content, "transcribedBy", &abc->author);
  readJsonString(content, "author", &abc->oriAuthor);

  abc->pitchLevel = (int)t1;
  abc->bitsPerPage = (int)t2;
  // Only process files less than 15 keys.
  if (abc->bitsPerPage > 16)
    return 0;

  return 1;
}

static void mergeTickInto(Vector_t *ticks, ABCTick_t *data) {
  size_t length;
  ABCTick_t *last;

  vec_size(ticks, &length);
  if (!length)
    vec_push(ticks, data);
  else {
    vec_at(ticks, -1, (void **)&last);
    if (last->time == data->time) {
      last->note1 |= data->note1;
      last->note2 |= data->note2;
    } else if (last->time < data->time)
      vec_push(ticks, data);
    else
      // Insertion sort.
      for (i64 i = 0; i < length; i++) {
        vec_at(ticks, i, (void **)&last);
        if (last->time > data->time) {
          vec_splice(ticks, i + 1, 0, data, 1);
          break;
        }
      }
  }
}

i32 readJsonABC(const char *input, SkyStudioABC *abc) {
  cJSON *json
    , *content
    , *notes
    , *note
    , *key;
  double time;
  ABCTick_t data;
  Vector_t *tickVec;

  if (!input || !abc)
    return 0;

  json = cJSON_Parse(input);
  memset(abc, 0, sizeof(SkyStudioABC));
  tickVec = &abc->ticks;

  // ABC json file must start with an 1 element array.
  if (cJSON_GetArraySize(json) < 1)
    goto ErrExit;
  content = cJSON_GetArrayItem(json, 0);

  // Only process decrypted file.
  if (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(content, "isEncrypted")))
    goto ErrExit;

  if (!readJsonABCHeader(content, abc))
    goto ErrExit;

  notes = cJSON_GetObjectItemCaseSensitive(content, "songNotes");
  if (!cJSON_IsArray(notes))
    goto ErrExit;

  vec_init(tickVec, sizeof(ABCTick_t));

  cJSON_ArrayForEach(note, notes) {
    readJsonNumber(note, "time", &time);
    key = cJSON_GetObjectItemCaseSensitive(note, "key");
    if (readJsonNote(cJSON_GetStringValue(key), &data)) {
      data.time = (int)time;
      mergeTickInto(tickVec, &data);
    }
  }

  cJSON_Delete(json);
  return 1;

ErrExit:
  cJSON_Delete(json);
  return 0;
}

i32 readABC(const wchar_t *input) {
  return 0;
}

void freeABC(SkyStudioABC *abc) {
  if (!abc)
    return;
  free(abc->author);
  free(abc->oriAuthor);
  free(abc->name);
  vec_free(&abc->ticks);
}