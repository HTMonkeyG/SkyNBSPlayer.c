#include "random.h"

void MT_setSeed(RandomMT_t *mt, u32 seed) {
  seed == 0 && (seed = 5489);
  mt->mt[0] = mt->seed = seed;
  for (int i = 1; i < 624; i++)
    mt->mt[i] = 1812433253 * (mt->mt[i - 1] ^ (mt->mt[i - 1] >> 30)) + i;
  mt->mti = 624;
  mt->hasStoredGaussian = 0;
  mt->storedGaussian = 0;
}

u32 MT_genRandInt32(RandomMT_t *mt) {
  u32 x, y;
  if (mt->mti >= 624) {
    for (int i = 0; i < 624; i++) {
      x = (mt->mt[i] & UPPER_MASK) | (mt->mt[(i + 1) % 624] & LOWER_MASK);
      mt->mt[i] = mt->mt[(i + 397) % 624] ^ (x >> 1) ^ MATRIX_A[x & 1];
    }
    mt->mti = 0;
  }

  y = mt->mt[mt->mti++];
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9D2C5680;
  y ^= (y << 15) & 0xEFC60000;
  y ^= (y >> 18);

  return y;
}

u32 MT_nextInt(RandomMT_t *mt) {
  return MT_genRandInt32(mt);
}

u64 MT_nextLong(RandomMT_t *mt) {
  return (u64)MT_genRandInt32(mt) << 32 | MT_genRandInt32(mt);
}

f32 MT_nextFloat(RandomMT_t *mt) {
  return (f32)MT_genRandInt32(mt) / (f32)0xFFFFFFFF;
}

f32 MT_nextGaussian(RandomMT_t *mt) {
  float result, a, b, c, d;
  if (mt->hasStoredGaussian) {
    mt->hasStoredGaussian = 0;
    return mt->storedGaussian;
  } else {
    do {
      do {
        a = ((f32)(i32)MT_genRandInt32(mt) / (f32)0x7FFFFFFF) * 2 - 1.0;
        b = ((f32)(i32)MT_genRandInt32(mt) / (f32)0x7FFFFFFF) * 2 - 1.0;
        c = b * b + a * a;
      } while (c >= 1.0);
    } while (c == 0.0);
    d = sqrt(-2 * logf(c) / c);
    mt->hasStoredGaussian = 1;
    result = d * a;
    mt->storedGaussian = d * b;
  }
  return result;
}
