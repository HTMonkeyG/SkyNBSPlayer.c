#ifndef _SKY_VECTOR_
#define _SKY_VECTOR_

#include <stdlib.h>
#include <string.h>

#include "macros.h"

#define INITIAL_LENGTH 1

typedef struct {
  void *begin;
  void *end;
  void *endOfSpace;
  size_t perElement;
} Vector_t;

Vector_t *vec_init(Vector_t *v, size_t sizePerElement);
Vector_t *vec_free(Vector_t *v);
Vector_t *vec_push(Vector_t *v, const void *element);
Vector_t *vec_pop(Vector_t *v, void *element);
Vector_t *vec_splice(Vector_t *v, i64 start, i64 deleteCount, const void *elements, i64 elementCount);
Vector_t *vec_from(Vector_t *v, i64 index, void *element);
Vector_t *vec_at(Vector_t *v, i64 index, void **element);
Vector_t *vec_size(Vector_t *v, size_t *result);

#endif
