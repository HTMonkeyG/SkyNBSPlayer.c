#ifndef __SKY_RANDOM__
#define __SKY_RANDOM__

#include <math.h>
#include "macros.h"

#define LOWER_MASK 0x7FFFFFFF
#define UPPER_MASK 0x80000000

typedef struct {
  u32 seed;
  u32 mt[624];
  i32 mti;
  i32 hasStoredGaussian;
  f32 storedGaussian;
} RandomMT_t;

static const u32 MATRIX_A[2] = { 0, 0x9908B0DF };

void MT_setSeed(RandomMT_t *mt, u32 seed);
u32 MT_genRandInt32(RandomMT_t *mt);
u32 MT_nextInt(RandomMT_t *mt);
u64 MT_nextLong(RandomMT_t *mt);
f32 MT_nextFloat(RandomMT_t *mt);
f32 MT_nextGaussian(RandomMT_t *mt);

#endif