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

Vector_t *vec_push(Vector_t *v, void *element) {
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
    return NULL;
  else {
    v->end -= v->perElement;
    memcpy(element, v->end, v->perElement);
  }
  return v;
}

Vector_t *vec_at(Vector_t *v, size_t index, void *element) {
  i64 s;
  if (!v || !element)
    return NULL;
  s = (v->endOfSpace - v->begin) / v->perElement;
  if (index < -s || s < index)
    return NULL;
  if (index < 0)
    memcpy(element, v->begin + (index + s) * v->perElement, v->perElement);
  else
    memcpy(element, v->begin + index * v->perElement, v->perElement);
  return v;
}

Vector_t *vec_size(Vector_t *v, size_t *result) {
  if (!v)
    return NULL;
  *result = (v->endOfSpace - v->begin) / v->perElement;
  return v;
}