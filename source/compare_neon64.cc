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
#include "libyuv/row.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

#if !defined(LIBYUV_DISABLE_NEON) && defined(__aarch64__)

// 256 bits at a time
// uses short accumulator which restricts count to 131 KB
uint32 HammingDistance_NEON(const uint8* src_a, const uint8* src_b, int count) {
  uint32 diff;
  asm volatile(
      "movi       v5.8h, #0                      \n"

      "1:                                        \n"
      "ld1        {v1.16b, v2.16b}, [%0], #32    \n"
      "ld1        {v3.16b, v4.16b}, [%1], #32    \n"
      "eor        v1.16b, v1.16b, v3.16b         \n"
      "eor        v2.16b, v2.16b, v4.16b         \n"
      "cnt        v1.16b, v1.16b                 \n"
      "cnt        v2.16b, v2.16b                 \n"
      "subs       %w2, %w2, #32                  \n"
      "add        v1.16b, v1.16b, v2.16b         \n"
      "uadalp     v5.8h, v1.16b                  \n"
      "b.gt       1b                             \n"

      "uaddlv     s5, v5.8h                      \n"
      "fmov       %w3, s5                        \n"
      : "+r"(src_a), "+r"(src_b), "+r"(count), "=r"(diff)
      :
      : "cc", "v1", "v2", "v3", "v4", "v5");
  return diff;
}

uint32 SumSquareError_NEON(const uint8* src_a, const uint8* src_b, int count) {
  uint32 sse;
  asm volatile(
      "eor        v26.16b, v26.16b, v26.16b      \n"
      "eor        v28.16b, v28.16b, v28.16b      \n"
      "eor        v27.16b, v27.16b, v27.16b      \n"
      "eor        v29.16b, v29.16b, v29.16b      \n"

      "1:                                        \n"
      "ld1        {v1.16b}, [%0], #16            \n"
      "ld1        {v2.16b}, [%1], #16            \n"
      "subs       %w2, %w2, #16                  \n"
      "usubl      v3.8h, v1.8b, v2.8b            \n"
      "usubl2     v4.8h, v1.16b, v2.16b          \n"
      "smlal      v26.4s, v3.4h, v3.4h           \n"
      "smlal      v27.4s, v4.4h, v4.4h           \n"
      "smlal2     v28.4s, v3.8h, v3.8h           \n"
      "smlal2     v29.4s, v4.8h, v4.8h           \n"
      "b.gt       1b                             \n"

      "add        v26.4s, v26.4s, v27.4s         \n"
      "add        v28.4s, v28.4s, v29.4s         \n"
      "add        v29.4s, v26.4s, v28.4s         \n"
      "addv       s1, v29.4s                     \n"
      "fmov       %w3, s1                        \n"
      : "+r"(src_a), "+r"(src_b), "+r"(count), "=r"(sse)
      :
      : "cc", "v1", "v2", "v3", "v4", "v26", "v27", "v28", "v29");
  return sse;
}

#endif  // !defined(LIBYUV_DISABLE_NEON) && defined(__aarch64__)

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
