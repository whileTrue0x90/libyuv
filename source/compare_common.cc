/*
 *  Copyright 2012 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/basic_types.h"

#include "libyuv/compare_row.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

#if ORIGINAL_OPT
uint32 HammingDistance_C(const uint8* src_a, const uint8* src_b, int count) {
  uint32 diff = 0u;

  int i;
  for (i = 0; i < count; ++i) {
    int x = src_a[i] ^ src_b[i];
    if (x & 1)
      ++diff;
    if (x & 2)
      ++diff;
    if (x & 4)
      ++diff;
    if (x & 8)
      ++diff;
    if (x & 16)
      ++diff;
    if (x & 32)
      ++diff;
    if (x & 64)
      ++diff;
    if (x & 128)
      ++diff;
  }
  return diff;
}
#endif

// Hakmem method for hamming distance.
// BenchmarkHammingDistance_C (234 ms)
uint32 HammingDistance_C1(const uint8* src_a, const uint8* src_b, int count) {
  uint32 diff = 0u;
  int i;
  for (i = 0; i < count - 7; i += 8) {
    uint64 x = *((uint64*)src_a) ^ *((uint64*)src_b);
    uint64 u = x - ((x >> 1) & 0x5555555555555555ULL);
    u = ((u >> 2) & 0x3333333333333333ULL) + (u & 0x3333333333333333ULL);
    diff += static_cast<int>(
        (((u + (u >> 4)) & 0xF0F0F0F0F0F0F0FULL) * 0x101010101010101ULL) >> 56);
    src_a += 8;
    src_b += 8;
  }
  return diff;
}

// Hakmem method for hamming distance.
// BenchmarkHammingDistance_C (330 ms)
uint32 HammingDistance_C(const uint8* src_a, const uint8* src_b, int count) {
  uint32 diff = 0u;

  int i;
  for (i = 0; i < count - 3; i += 4) {
    uint32 x = *((uint32*)src_a) ^ *((uint32*)src_b);
    uint32 u = x - ((x >> 1) & 033333333333) - ((x >> 2) & 011111111111);
    diff += ((u + (u >> 3)) & 030707070707) % 63;
    src_a += 4;
    src_b += 4;
  }
  return diff;
}

#if 0

// Hakmem method for hamming distance.
uint32 HammingDistance_C(const uint8* src_a, const uint8* src_b, int count) {
  uint32 diff = 0u;
  int i;
  for (i = 0; i < count - 7; i += 8) {
    uint64 x = *((uint64*)src_a) ^ *((uint64*)src_b);


    uint64 u = x - ((x >> 1) & 0x7777777777777777ULL) -
               ((x >> 2) & 0x3333333333333333ULL) -
               ((x >> 3) & 0x1111111111111111ULL);
    diff += ((u + (u >> 4)) & 0x0F0F0F0F0F0F0F0FULL) % 255;


    src_a += 8;
    src_b += 8;
  }
  return diff;
}

    n -= (n >> 1) & 0x5555555555555555ULL;
    n = ((n >> 2) & 0x3333333333333333ULL) + (n & 0x3333333333333333ULL);
    return static_cast<int>(
        (((n + (n >> 4)) & 0xF0F0F0F0F0F0F0FULL) * 0x101010101010101ULL) >> 56);
#endif

uint32 SumSquareError_C(const uint8* src_a, const uint8* src_b, int count) {
  uint32 sse = 0u;
  int i;
  for (i = 0; i < count; ++i) {
    int diff = src_a[i] - src_b[i];
    sse += (uint32)(diff * diff);
  }
  return sse;
}

// hash seed of 5381 recommended.
// Internal C version of HashDjb2 with int sized count for efficiency.
uint32 HashDjb2_C(const uint8* src, int count, uint32 seed) {
  uint32 hash = seed;
  int i;
  for (i = 0; i < count; ++i) {
    hash += (hash << 5) + src[i];
  }
  return hash;
}

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
