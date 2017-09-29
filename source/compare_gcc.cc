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

// This module is for GCC x86 and x64.
#if !defined(LIBYUV_DISABLE_X86) && \
    (defined(__x86_64__) || (defined(__i386__) && !defined(_MSC_VER)))

uint32 HammingDistance_X86(const uint8* src_a, const uint8* src_b, int count) {
  uint32 diff = 0u;

  int i;
  for (i = 0; i < count - 7; i += 8) {
    uint64 x = *((uint64*)src_a) ^ *((uint64*)src_b);
    src_a += 8;
    src_b += 8;
    diff += __builtin_popcountll(x);
  }
  return diff;
}

// SSE version
uint32 HammingDistance_SSE2(const uint8* src_a, const uint8* src_b, int count) {
  uint64 diff = 0u;
  uint64 d0 = 0u;
  uint64 d1 = 0u;
  uint64 d2 = 0u;
  uint64 d3 = 0u;

  asm volatile (
                "pxor                   %%xmm0,%%xmm0                                           \n"
                "cmp                    $0x40,%2                                                        \n"
                "jge                    1f                                                                      \n"
                "cmp                    $0x10,%2                                                        \n"
                "jge                    2f                                                                      \n"

        LABELALIGN
        "1:                                                                                                             \n"
                "movdqa         " MEMACCESS(0) ",%%xmm0                         \n" 
                "movdqa                 0x10(%0),%%xmm1                                         \n" 
                "movdqa                 0x20(%0),%%xmm2                                         \n" 
                "movdqa                 " MEMACCESS(0) ",%%xmm0                         \n" 
                "pxor               (%1),%%xmm0                                                 \n" 
                "movq                   %%xmm0,%3                                                       \n" 
                "popcnt                 %3,%3                                                           \n" 
                "add                    %4,%3                                                           \n" 
                "pextrq                 $0x1,%%xmm0,%4                                          \n" 
                "pxor                   0x10(%1),%%xmm1                                         \n" 
                "popcnt                 %4,%5                                                           \n" 
                "add                    %6,%5                                                           \n" 
                "movq                   %%xmm1,%4                                                       \n" 
                "popcnt                 %4,%4                                                           \n" 
                "add                    %3,%4                                                           \n" 
                "pextrq                 $0x1,%%xmm1,%6                                          \n" 
                "movdqa                 0x30(%0),%%xmm0                                         \n" 
                "popcnt                 %6,%7                                                           \n"     
                "pxor                   0x20(%1),%%xmm2                                         \n" 
                "add                    %7,%5                                                           \n" 
                "movq                   %%xmm2, %7                                                      \n" 
                "popcnt                 %7,%7                                                           \n" 
                "add                    %4,%5                                                           \n" 
                "pextrq                 $0x1,%%xmm2,%4                                          \n" 
                "popcnt                 %4,%3                                                           \n" 
                "add                    %7,%3                                                           \n" 
                "pxor                   0x30(%1),%%xmm0                                         \n" 
                "add                    $0x40,%0                                                        \n" 
                "movq                   %%xmm0,%4                                                       \n" 
                "popcnt                 %4,%4                                                           \n" 
                "add                    %5,%4                                                           \n" 
                "pextrq                 $0x1,%%xmm0,%5                                          \n" 
                "add                    $0x40,%1                                                        \n" 
                "popcnt                 %5,%6                                                           \n" 
                "add                    %3,%6                                                           \n" 
                "sub                    $0x40,%2                                                        \n"  
                "cmp                    $0x40,%2                                                        \n"
            "jge                        1b                                                                      \n"
                "cmp                    $0x10,%2                                                        \n"
                "jl                             3f                                                                      \n"
                
        LABELALIGN
        "2:                                                                                                             \n"
                "movdqa                 " MEMACCESS(0) ",%%xmm0                         \n" 
                "pxor                   (%1),%%xmm0                                                     \n" 
                "movq                   %%xmm0,%5                                                       \n" 
                "popcnt                 %5,%5                                                           \n" 
                "add                    %5,%4                                                           \n" 
                "pextrq                 $0x1,%%xmm0,%5                                          \n" 
                "popcnt                 %5,%5                                                           \n" 
                "add                    %5,%6                                                           \n" 
                "add                    $0x10,%0                                                        \n"
                "add                    $0x10,%1                                                        \n"
                "sub                    $0x10,%2                                                        \n"
                "cmp                    $0x10,%2                                                        \n"
                "jge                    2b                                                                      \n"

        LABELALIGN
        "3:                                                                                                             \n"
           "add            %4,%6                                                                \n"


        :       "+r"(src_a),      // %0
                "+r"(src_b),      // %1
                "+r"(count),      // %2
                "+r"(d0),         // %3 
                "+r"(d1),         // %4 
                "+r"(d2),         // %5 
                "+r"(diff),       // %6
                "+r"(d3)          // %7 
        :: "memory", "cc", "xmm1", "xmm0", "xmm2"
        );
  return diff;
}

// AVX2 version
uint32 HammingDistance_AVX2(const uint8* src_a, const uint8* src_b, int count) {
  char m1[32] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};
  char m2[32] = {0x00, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x03,
                 0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
                 0x00, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x03,
                 0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04};
  uint32 diff = 0u;
  uint64 d0 = 0u;

  asm volatile (
                "vpxor                  %%xmm0,%%xmm0,%%xmm0                            \n"
                "vmovdqa                %3,%%ymm2                                                       \n"
                "cmp                    $0x40,%2                                                        \n"
                "jge                    1f                                                                      \n"
                "cmp                    $0x20,%2                                                        \n"
                "jge                    2f                                                                      \n"

                LABELALIGN
        "1:                                                                                                             \n"
                "vpxor                  %%xmm1,%%xmm1,%%xmm1                            \n"
                "vmovdqa                %4,%%ymm3                                                       \n"
                "vmovdqa                " MEMACCESS(0) ",%%ymm4                         \n"
                "vmovdqa                0x20(%0), %%ymm5                                        \n"
                "vpxor                  " MEMACCESS(1) ", %%ymm4, %%ymm4        \n"
                "vpand                  %%ymm2,%%ymm4,%%ymm6                            \n"
                "vpsrlw                 $0x4,%%ymm4,%%ymm4                                      \n"
                "vpshufb                %%ymm6,%%ymm3,%%ymm6                            \n"
                "vpand                  %%ymm2,%%ymm4,%%ymm4                            \n"
                "vpshufb                %%ymm4,%%ymm3,%%ymm4                            \n"
                "vpaddb                 %%ymm6,%%ymm4,%%ymm4                            \n"
                "vpsadbw                %%ymm1,%%ymm4,%%ymm4                            \n"
                "vpaddq                 %%ymm0,%%ymm4,%%ymm0                            \n"
                "vpxor                  0x20(%1),%%ymm5,%%ymm4                          \n"
                "add                    $0x40,%0                                                        \n"
                "add                    $0x40,%1                                                        \n"
                "vpand                  %%ymm2,%%ymm4,%%ymm5                            \n"
                "vpsrlw                 $0x4,%%ymm4,%%ymm4                                      \n"
                "vpshufb                %%ymm5,%%ymm3,%%ymm5                            \n"
                "vpand                  %%ymm2,%%ymm4,%%ymm4                            \n"
                "vpshufb                %%ymm4,%%ymm3,%%ymm4                            \n"
                "vpaddb                 %%ymm5,%%ymm4,%%ymm4                            \n"
                "vpsadbw                %%ymm1,%%ymm4,%%ymm4                            \n"
                "vpaddq                 %%ymm0,%%ymm4,%%ymm0                            \n"
                "sub                    $0x40,%2                                                        \n"
                "cmp                    $0x40,%2                                                        \n"
                "jge                    1b                                                                      \n"
                "cmp                    $0x20,%2                                                    \n"
                "jl                             3f                                                                      \n"

                LABELALIGN
        "2:                                                                                                             \n"
                "vmovdqa                (%0),%%ymm1                                                     \n"
                "vpxor                  (%1),%%ymm1,%%ymm1                                      \n"
                "vpand                  %%ymm2,%%ymm1,%%ymm3                            \n"
                "vmovdqa                %4,%%ymm4                                                       \n"
                "vpsrlw                 $0x4,%%ymm1,%%ymm1                                      \n"
                "vpshufb                %%ymm3,%%ymm4,%%ymm3                            \n"
                "vpand                  %%ymm2,%%ymm1,%%ymm1                            \n"
                "vpshufb                %%ymm1,%%ymm4,%%ymm1                            \n"
                "vpaddb                 %%ymm3,%%ymm1,%%ymm1                            \n"
                "vpxor                  %%xmm2,%%xmm2,%%xmm2                            \n"
                "vpsadbw                %%ymm2,%%ymm1,%%ymm1                            \n"
                "vpaddq                 %%ymm0,%%ymm1,%%ymm0                            \n"

                LABELALIGN
        "3:                                                                                                     \n"
                "vpextrq                $0x1,%%xmm0,%6                                  \n"
                "vmovq                  %%xmm0,%5                                               \n"
                "add                    %5,%6                                                   \n"
                "vextracti128   $0x1,%%ymm0,%%xmm0                              \n"
                "vmovq                  %%xmm0,%5                                               \n"
                "add                    %6,%5                                                   \n"
                "vpextrq                $0x1,%%xmm0,%6                                  \n"
                "add                    %5,%6                                                   \n"
                "vzeroupper                                                                             \n"

        :       "+r"(src_a),      // %0
                "+r"(src_b),      // %1
                "+r"(count),      // %2
                "+m"(m1),                 // %3
                "+m"(m2),         // %4
                "+r"(d0),         // %5
                "=g"(diff)        // %6

                :: "memory", "cc", "xmm1", "xmm0", "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6"
                );
  return diff;
}

uint32 SumSquareError_SSE2(const uint8* src_a, const uint8* src_b, int count) {
  uint32 sse;
  asm volatile (
    "pxor      %%xmm0,%%xmm0                   \n"
    "pxor      %%xmm5,%%xmm5                   \n"
    LABELALIGN
  "1:                                          \n"
    "movdqu    " MEMACCESS(0) ",%%xmm1         \n"
    "lea       " MEMLEA(0x10, 0) ",%0          \n"
    "movdqu    " MEMACCESS(1) ",%%xmm2         \n"
    "lea       " MEMLEA(0x10, 1) ",%1          \n"
    "movdqa    %%xmm1,%%xmm3                   \n"
    "psubusb   %%xmm2,%%xmm1                   \n"
    "psubusb   %%xmm3,%%xmm2                   \n"
    "por       %%xmm2,%%xmm1                   \n"
    "movdqa    %%xmm1,%%xmm2                   \n"
    "punpcklbw %%xmm5,%%xmm1                   \n"
    "punpckhbw %%xmm5,%%xmm2                   \n"
    "pmaddwd   %%xmm1,%%xmm1                   \n"
    "pmaddwd   %%xmm2,%%xmm2                   \n"
    "paddd     %%xmm1,%%xmm0                   \n"
    "paddd     %%xmm2,%%xmm0                   \n"
    "sub       $0x10,%2                        \n"
    "jg        1b                              \n"

    "pshufd    $0xee,%%xmm0,%%xmm1             \n"
    "paddd     %%xmm1,%%xmm0                   \n"
    "pshufd    $0x1,%%xmm0,%%xmm1              \n"
    "paddd     %%xmm1,%%xmm0                   \n"
    "movd      %%xmm0,%3                       \n"

  : "+r"(src_a),      // %0
    "+r"(src_b),      // %1
    "+r"(count),      // %2
    "=g"(sse)         // %3
  :: "memory", "cc", "xmm0", "xmm1", "xmm2", "xmm3", "xmm5"
  );
  return sse;
}

static uvec32 kHash16x33 = {0x92d9e201, 0, 0, 0};  // 33 ^ 16
static uvec32 kHashMul0 = {
    0x0c3525e1,  // 33 ^ 15
    0xa3476dc1,  // 33 ^ 14
    0x3b4039a1,  // 33 ^ 13
    0x4f5f0981,  // 33 ^ 12
};
static uvec32 kHashMul1 = {
    0x30f35d61,  // 33 ^ 11
    0x855cb541,  // 33 ^ 10
    0x040a9121,  // 33 ^ 9
    0x747c7101,  // 33 ^ 8
};
static uvec32 kHashMul2 = {
    0xec41d4e1,  // 33 ^ 7
    0x4cfa3cc1,  // 33 ^ 6
    0x025528a1,  // 33 ^ 5
    0x00121881,  // 33 ^ 4
};
static uvec32 kHashMul3 = {
    0x00008c61,  // 33 ^ 3
    0x00000441,  // 33 ^ 2
    0x00000021,  // 33 ^ 1
    0x00000001,  // 33 ^ 0
};

uint32 HashDjb2_SSE41(const uint8* src, int count, uint32 seed) {
  uint32 hash;
  asm volatile (
    "movd      %2,%%xmm0                       \n"
    "pxor      %%xmm7,%%xmm7                   \n"
    "movdqa    %4,%%xmm6                       \n"
    LABELALIGN
  "1:                                          \n"
    "movdqu    " MEMACCESS(0) ",%%xmm1         \n"
    "lea       " MEMLEA(0x10, 0) ",%0          \n"
    "pmulld    %%xmm6,%%xmm0                   \n"
    "movdqa    %5,%%xmm5                       \n"
    "movdqa    %%xmm1,%%xmm2                   \n"
    "punpcklbw %%xmm7,%%xmm2                   \n"
    "movdqa    %%xmm2,%%xmm3                   \n"
    "punpcklwd %%xmm7,%%xmm3                   \n"
    "pmulld    %%xmm5,%%xmm3                   \n"
    "movdqa    %6,%%xmm5                       \n"
    "movdqa    %%xmm2,%%xmm4                   \n"
    "punpckhwd %%xmm7,%%xmm4                   \n"
    "pmulld    %%xmm5,%%xmm4                   \n"
    "movdqa    %7,%%xmm5                       \n"
    "punpckhbw %%xmm7,%%xmm1                   \n"
    "movdqa    %%xmm1,%%xmm2                   \n"
    "punpcklwd %%xmm7,%%xmm2                   \n"
    "pmulld    %%xmm5,%%xmm2                   \n"
    "movdqa    %8,%%xmm5                       \n"
    "punpckhwd %%xmm7,%%xmm1                   \n"
    "pmulld    %%xmm5,%%xmm1                   \n"
    "paddd     %%xmm4,%%xmm3                   \n"
    "paddd     %%xmm2,%%xmm1                   \n"
    "paddd     %%xmm3,%%xmm1                   \n"
    "pshufd    $0xe,%%xmm1,%%xmm2              \n"
    "paddd     %%xmm2,%%xmm1                   \n"
    "pshufd    $0x1,%%xmm1,%%xmm2              \n"
    "paddd     %%xmm2,%%xmm1                   \n"
    "paddd     %%xmm1,%%xmm0                   \n"
    "sub       $0x10,%1                        \n"
    "jg        1b                              \n"
    "movd      %%xmm0,%3                       \n"
  : "+r"(src),        // %0
    "+r"(count),      // %1
    "+rm"(seed),      // %2
    "=g"(hash)        // %3
  : "m"(kHash16x33),  // %4
    "m"(kHashMul0),   // %5
    "m"(kHashMul1),   // %6
    "m"(kHashMul2),   // %7
    "m"(kHashMul3)    // %8
  : "memory", "cc"
    , "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
  );
  return hash;
}
#endif  // defined(__x86_64__) || (defined(__i386__) && !defined(__pic__)))

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
