#include "abc.h"

static i32 readJsonString(
  const cJSON *const obj,
  const char *name,
  char **result
) {
  cJSON *kv = cJSON_GetObjectItemCaseSensitive(obj, name);
  char *str;
  if (!cJSON_IsString(kv))
    return 0;
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
  i32 type = 0
    , note = 0;
  
  if (!str || !result)
    return 0;

  strncpy(buffer, str, 15);
  l = strlen(buffer);
  
  for (char i = 0; i < l; i++) {
    if (!strncmp(buffer + i, "Key", 3)) {
      memset(buffer + i, 0, 3);
      type = strtol(buffer, &p, 10);
      note = strtol(buffer + i + 3, &p, 10);
    }
  }

  printf("%d %d\n", type, note);
}

static i32 readJsonABCHeader(const cJSON *const content, SkyStudioABC *abc) {
  double t1, t2;
  if (
    !readJsonNumber(content, "bpm", &abc->bpm)
    || !readJsonString(content, "name", &abc->name)
    || !readJsonString(content, "transcribedBy", &abc->author)
    || !readJsonString(content, "author", &abc->oriAuthor)
    || !readJsonNumber(content, "pitchLevel", &t1)
    || !readJsonNumber(content, "bitsPerPage", &t2)
  )
    return 0;

  abc->pitchLevel = (int)t1;
  abc->bitsPerPage = (int)t2;
  // Only process files less than 15 keys.
  if (abc->bitsPerPage > 16)
    return 0;

  return 1;
}

i32 readABC(const wchar_t *input) {
  
}

i32 readJsonABC(const char *input, SkyStudioABC *abc) {
  i32 result = 1;
  cJSON *json
    , *content
    , *notes
    , *note
    , *key;
  size_t length;
  double time;

  if (!input || !abc)
    return 0;

  json = cJSON_Parse(input);
  memset(abc, 0, sizeof(SkyStudioABC));

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

  vec_init(&abc->ticks, sizeof(ABCTick_t));

  cJSON_ArrayForEach(note, notes) {
    // Insertion sort.
    readJsonNumber(note, "time", &time);
    key = cJSON_GetObjectItemCaseSensitive(note, "key");
    readJsonNote(cJSON_GetStringValue(key), 1);
  }

  cJSON_Delete(json);
  return 1;

ErrExit:
  cJSON_Delete(json);
  return 0;
}

void freeABC(SkyStudioABC *abc) {

}