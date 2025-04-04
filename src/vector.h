#include <stdlib.h>
#include <string.h>

#include "macros.h"

typedef struct {
  void *begin;
  void *end;
  void *endOfSpace;
  size_t perElement;
} Vector_t;

Vector_t *vec_init(Vector_t *v, size_t sizePerElement);
Vector_t *vec_free(Vector_t *v);
Vector_t *vec_push(Vector_t *v, void *element);
Vector_t *vec_pop(Vector_t *v, void *element);
Vector_t *vec_at(Vector_t *v, size_t index, void *element);
Vector_t *vec_size(Vector_t *v, size_t *result);