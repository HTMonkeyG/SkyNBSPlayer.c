#include "vector.h"

Vector_t *vec_init(Vector_t *v, size_t sizePerElement) {
  void *p;
  if (!v)
    return NULL;
  vec_free(v);
  v->perElement = sizePerElement;
  p = malloc(sizePerElement);
  if (!p)
    return NULL;
  v->begin = v->end = p;
  v->endOfSpace = p + sizePerElement;
  return v;
}

Vector_t *vec_free(Vector_t *v) {
  if (!v)
    return NULL;
  free(v->begin);
  memset(v, 0, sizeof(Vector_t));
  return v;
}

Vector_t *vec_push(Vector_t *v, const void *element) {
  void *p;
  size_t s;
  if (!v || !element)
    return NULL;
  if (v->end == v->endOfSpace) {
    s = v->endOfSpace - v->begin;
    p = malloc(s * 2);
    memcpy(p, v->begin, s);
    v->begin = p;
    v->end = p + s;
    v->endOfSpace = p + s * 2;
  }
  memcpy(v->end, element, v->perElement);
  v->end += v->perElement;
  return v;
}

Vector_t *vec_pop(Vector_t *v, void *element) {
  if (!v || !element)
    return NULL;
  if (v->begin == v->end)
    return v;
  else {
    v->end -= v->perElement;
    memcpy(element, v->end, v->perElement);
  }
  return v;
}

Vector_t *vec_splice(Vector_t *v, i64 start, i64 deleteCount, const void *elements, i64 elementCount) {
  i64 s, pe, ms, e;
  void *p = NULL;
  if (!elements || !v)
    return NULL;

  pe = v->perElement;
  s = (v->end - v->begin) / pe;
  ms = (v->endOfSpace - v->begin) / pe;

  if (start < -s)
    start = 0;
  else if (start >= s)
    start = s;
  else if (start < 0)
    start += s;

  if (deleteCount > s - start)
    deleteCount = s - start;
  else if (deleteCount < 0 || start == s)
    deleteCount = 0;
  
  if (s - deleteCount + elementCount > ms)
    e = pe * ms * 2;
  else
    e = pe * ms;
    
  p = malloc(e);
  memcpy(p, v->begin, start * pe);
  memcpy(p + start * pe, elements, elementCount * pe);
  memcpy(
    p + (start + elementCount) * pe,
    v->begin + deleteCount * pe,
    (s - start - deleteCount) * pe
  );
  free(v->begin);
  v->begin = p;
  v->end = p + (s + elementCount - deleteCount) * pe;
  v->endOfSpace = p + e;

  return v;
}

Vector_t *vec_from(Vector_t *v, i64 index, void *element) {
  i64 s;
  if (!v || !element)
    return NULL;
  s = (v->end - v->begin) / v->perElement;
  if (index < -s || s <= index)
    return NULL;
  if (index < 0)
    memcpy(element, v->begin + (index + s) * v->perElement, v->perElement);
  else
    memcpy(element, v->begin + index * v->perElement, v->perElement);
  return v;
}

Vector_t *vec_at(Vector_t *v, i64 index, void **element) {
  i64 s;
  if (!v || !element)
    return NULL;
  s = (v->end - v->begin) / v->perElement;
  if (index < -s || s <= index)
    return NULL;
  if (index < 0)
    *element = v->begin + (index + s) * v->perElement;
  else
    *element = v->begin + index * v->perElement;
  return v;
}

Vector_t *vec_size(Vector_t *v, size_t *result) {
  if (!v)
    return NULL;
  *result = (v->end - v->begin) / v->perElement;
  return v;
}