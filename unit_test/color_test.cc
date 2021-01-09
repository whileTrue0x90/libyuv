/*
 *  Copyright 2015 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>

// use SIMD_ALIGNED macro from row.h
#include "libyuv/row.h"

#include "../unit_test/unit_test.h"
#include "libyuv/basic_types.h"
#include "libyuv/convert.h"
#include "libyuv/convert_argb.h"
#include "libyuv/convert_from.h"
#include "libyuv/convert_from_argb.h"
#include "libyuv/cpu_id.h"

namespace libyuv {

// TODO(fbarchard): clang x86 has a higher accuracy YUV to RGB.
// Port to Visual C and other CPUs
#if !defined(LIBYUV_DISABLE_X86) && \
    (defined(__x86_64__) || (defined(__i386__) && !defined(_MSC_VER)))
#define ERROR_FULL 5
#define ERROR_J420 4
#else
#define ERROR_FULL 6
#define ERROR_J420 6
#endif
#define ERROR_R 2
#define ERROR_G 2
#define ERROR_B 3

#define TESTCS(TESTNAME, YUVTOARGB, ARGBTOYUV, HS1, HS, HN, DIFF)              \
  TEST_F(LibYUVColorTest, TESTNAME) {                                          \
    const int kPixels = benchmark_width_ * benchmark_height_;                  \
    const int kHalfPixels =                                                    \
        ((benchmark_width_ + 1) / 2) * ((benchmark_height_ + HS1) / HS);       \
    align_buffer_page_end(orig_y, kPixels);                                    \
    align_buffer_page_end(orig_u, kHalfPixels);                                \
    align_buffer_page_end(orig_v, kHalfPixels);                                \
    align_buffer_page_end(orig_pixels, kPixels * 4);                           \
    align_buffer_page_end(temp_y, kPixels);                                    \
    align_buffer_page_end(temp_u, kHalfPixels);                                \
    align_buffer_page_end(temp_v, kHalfPixels);                                \
    align_buffer_page_end(dst_pixels_opt, kPixels * 4);                        \
    align_buffer_page_end(dst_pixels_c, kPixels * 4);                          \
                                                                               \
    MemRandomize(orig_pixels, kPixels * 4);                                    \
    MemRandomize(orig_y, kPixels);                                             \
    MemRandomize(orig_u, kHalfPixels);                                         \
    MemRandomize(orig_v, kHalfPixels);                                         \
    MemRandomize(temp_y, kPixels);                                             \
    MemRandomize(temp_u, kHalfPixels);                                         \
    MemRandomize(temp_v, kHalfPixels);                                         \
    MemRandomize(dst_pixels_opt, kPixels * 4);                                 \
    MemRandomize(dst_pixels_c, kPixels * 4);                                   \
                                                                               \
    /* The test is overall for color conversion matrix being reversible, so */ \
    /* this initializes the pixel with 2x2 blocks to eliminate subsampling. */ \
    uint8_t* p = orig_y;                                                       \
    for (int y = 0; y < benchmark_height_ - HS1; y += HS) {                    \
      for (int x = 0; x < benchmark_width_ - 1; x += 2) {                      \
        uint8_t r = static_cast<uint8_t>(fastrand());                          \
        p[0] = r;                                                              \
        p[1] = r;                                                              \
        p[HN] = r;                                                             \
        p[HN + 1] = r;                                                         \
        p += 2;                                                                \
      }                                                                        \
      if (benchmark_width_ & 1) {                                              \
        uint8_t r = static_cast<uint8_t>(fastrand());                          \
        p[0] = r;                                                              \
        p[HN] = r;                                                             \
        p += 1;                                                                \
      }                                                                        \
      p += HN;                                                                 \
    }                                                                          \
    if ((benchmark_height_ & 1) && HS == 2) {                                  \
      for (int x = 0; x < benchmark_width_ - 1; x += 2) {                      \
        uint8_t r = static_cast<uint8_t>(fastrand());                          \
        p[0] = r;                                                              \
        p[1] = r;                                                              \
        p += 2;                                                                \
      }                                                                        \
      if (benchmark_width_ & 1) {                                              \
        uint8_t r = static_cast<uint8_t>(fastrand());                          \
        p[0] = r;                                                              \
        p += 1;                                                                \
      }                                                                        \
    }                                                                          \
    /* Start with YUV converted to ARGB. */                                    \
    YUVTOARGB(orig_y, benchmark_width_, orig_u, (benchmark_width_ + 1) / 2,    \
              orig_v, (benchmark_width_ + 1) / 2, orig_pixels,                 \
              benchmark_width_ * 4, benchmark_width_, benchmark_height_);      \
                                                                               \
    ARGBTOYUV(orig_pixels, benchmark_width_ * 4, temp_y, benchmark_width_,     \
              temp_u, (benchmark_width_ + 1) / 2, temp_v,                      \
              (benchmark_width_ + 1) / 2, benchmark_width_,                    \
              benchmark_height_);                                              \
                                                                               \
    MaskCpuFlags(disable_cpu_flags_);                                          \
    YUVTOARGB(temp_y, benchmark_width_, temp_u, (benchmark_width_ + 1) / 2,    \
              temp_v, (benchmark_width_ + 1) / 2, dst_pixels_c,                \
              benchmark_width_ * 4, benchmark_width_, benchmark_height_);      \
    MaskCpuFlags(benchmark_cpu_info_);                                         \
                                                                               \
    for (int i = 0; i < benchmark_iterations_; ++i) {                          \
      YUVTOARGB(temp_y, benchmark_width_, temp_u, (benchmark_width_ + 1) / 2,  \
                temp_v, (benchmark_width_ + 1) / 2, dst_pixels_opt,            \
                benchmark_width_ * 4, benchmark_width_, benchmark_height_);    \
    }                                                                          \
    /* Test C and SIMD match. */                                               \
    for (int i = 0; i < kPixels * 4; ++i) {                                    \
      EXPECT_EQ(dst_pixels_c[i], dst_pixels_opt[i]);                           \
    }                                                                          \
    /* Test SIMD is close to original. */                                      \
    for (int i = 0; i < kPixels * 4; ++i) {                                    \
      EXPECT_NEAR(static_cast<int>(orig_pixels[i]),                            \
                  static_cast<int>(dst_pixels_opt[i]), DIFF);                  \
    }                                                                          \
                                                                               \
    free_aligned_buffer_page_end(orig_pixels);                                 \
    free_aligned_buffer_page_end(orig_y);                                      \
    free_aligned_buffer_page_end(orig_u);                                      \
    free_aligned_buffer_page_end(orig_v);                                      \
    free_aligned_buffer_page_end(temp_y);                                      \
    free_aligned_buffer_page_end(temp_u);                                      \
    free_aligned_buffer_page_end(temp_v);                                      \
    free_aligned_buffer_page_end(dst_pixels_opt);                              \
    free_aligned_buffer_page_end(dst_pixels_c);                                \
  }

TESTCS(TestI420, I420ToARGB, ARGBToI420, 1, 2, benchmark_width_, ERROR_FULL)
TESTCS(TestI422, I422ToARGB, ARGBToI422, 0, 1, 0, ERROR_FULL)
TESTCS(TestJ420, J420ToARGB, ARGBToJ420, 1, 2, benchmark_width_, ERROR_J420)
TESTCS(TestJ422, J422ToARGB, ARGBToJ422, 0, 1, 0, ERROR_J420)

static void YUVToRGB(int y, int u, int v, int* r, int* g, int* b) {
  const int kWidth = 16;
  const int kHeight = 1;
  const int kPixels = kWidth * kHeight;
  const int kHalfPixels = ((kWidth + 1) / 2) * ((kHeight + 1) / 2);

  SIMD_ALIGNED(uint8_t orig_y[16]);
  SIMD_ALIGNED(uint8_t orig_u[8]);
  SIMD_ALIGNED(uint8_t orig_v[8]);
  SIMD_ALIGNED(uint8_t orig_pixels[16 * 4]);
  memset(orig_y, y, kPixels);
  memset(orig_u, u, kHalfPixels);
  memset(orig_v, v, kHalfPixels);

  /* YUV converted to ARGB. */
  I422ToARGB(orig_y, kWidth, orig_u, (kWidth + 1) / 2, orig_v, (kWidth + 1) / 2,
             orig_pixels, kWidth * 4, kWidth, kHeight);

  *b = orig_pixels[0];
  *g = orig_pixels[1];
  *r = orig_pixels[2];
}

static void YUVJToRGB(int y, int u, int v, int* r, int* g, int* b) {
  const int kWidth = 16;
  const int kHeight = 1;
  const int kPixels = kWidth * kHeight;
  const int kHalfPixels = ((kWidth + 1) / 2) * ((kHeight + 1) / 2);

  SIMD_ALIGNED(uint8_t orig_y[16]);
  SIMD_ALIGNED(uint8_t orig_u[8]);
  SIMD_ALIGNED(uint8_t orig_v[8]);
  SIMD_ALIGNED(uint8_t orig_pixels[16 * 4]);
  memset(orig_y, y, kPixels);
  memset(orig_u, u, kHalfPixels);
  memset(orig_v, v, kHalfPixels);

  /* YUV converted to ARGB. */
  J422ToARGB(orig_y, kWidth, orig_u, (kWidth + 1) / 2, orig_v, (kWidth + 1) / 2,
             orig_pixels, kWidth * 4, kWidth, kHeight);

  *b = orig_pixels[0];
  *g = orig_pixels[1];
  *r = orig_pixels[2];
}

static void YUVHToRGB(int y, int u, int v, int* r, int* g, int* b) {
  const int kWidth = 16;
  const int kHeight = 1;
  const int kPixels = kWidth * kHeight;
  const int kHalfPixels = ((kWidth + 1) / 2) * ((kHeight + 1) / 2);

  SIMD_ALIGNED(uint8_t orig_y[16]);
  SIMD_ALIGNED(uint8_t orig_u[8]);
  SIMD_ALIGNED(uint8_t orig_v[8]);
  SIMD_ALIGNED(uint8_t orig_pixels[16 * 4]);
  memset(orig_y, y, kPixels);
  memset(orig_u, u, kHalfPixels);
  memset(orig_v, v, kHalfPixels);

  /* YUV converted to ARGB. */
  H422ToARGB(orig_y, kWidth, orig_u, (kWidth + 1) / 2, orig_v, (kWidth + 1) / 2,
             orig_pixels, kWidth * 4, kWidth, kHeight);

  *b = orig_pixels[0];
  *g = orig_pixels[1];
  *r = orig_pixels[2];
}

static void YUVRec2020ToRGB(int y, int u, int v, int* r, int* g, int* b) {
  const int kWidth = 16;
  const int kHeight = 1;
  const int kPixels = kWidth * kHeight;
  const int kHalfPixels = ((kWidth + 1) / 2) * ((kHeight + 1) / 2);

  SIMD_ALIGNED(uint8_t orig_y[16]);
  SIMD_ALIGNED(uint8_t orig_u[8]);
  SIMD_ALIGNED(uint8_t orig_v[8]);
  SIMD_ALIGNED(uint8_t orig_pixels[16 * 4]);
  memset(orig_y, y, kPixels);
  memset(orig_u, u, kHalfPixels);
  memset(orig_v, v, kHalfPixels);

  /* YUV converted to ARGB. */
  U422ToARGB(orig_y, kWidth, orig_u, (kWidth + 1) / 2, orig_v, (kWidth + 1) / 2,
             orig_pixels, kWidth * 4, kWidth, kHeight);

  *b = orig_pixels[0];
  *g = orig_pixels[1];
  *r = orig_pixels[2];
}

static void YUVMatrixToRGB(int y,
                           int u,
                           int v,
                           int* r,
                           int* g,
                           int* b,
                           const struct YuvConstants* yuvconstants) {
  const int kWidth = 16;
  const int kHeight = 1;
  const int kPixels = kWidth * kHeight;
  const int kHalfPixels = ((kWidth + 1) / 2) * ((kHeight + 1) / 2);

  SIMD_ALIGNED(uint8_t orig_y[16]);
  SIMD_ALIGNED(uint8_t orig_u[8]);
  SIMD_ALIGNED(uint8_t orig_v[8]);
  SIMD_ALIGNED(uint8_t orig_pixels[16 * 4]);
  memset(orig_y, y, kPixels);
  memset(orig_u, u, kHalfPixels);
  memset(orig_v, v, kHalfPixels);

  /* YUV converted to ARGB. */
  I422ToARGBMatrix(orig_y, kWidth, orig_u, (kWidth + 1) / 2, orig_v,
                   (kWidth + 1) / 2, orig_pixels, kWidth * 4, yuvconstants,
                   kWidth, kHeight);

  *b = orig_pixels[0];
  *g = orig_pixels[1];
  *r = orig_pixels[2];
}

static void YToRGB(int y, int* r, int* g, int* b) {
  const int kWidth = 16;
  const int kHeight = 1;
  const int kPixels = kWidth * kHeight;

  SIMD_ALIGNED(uint8_t orig_y[16]);
  SIMD_ALIGNED(uint8_t orig_pixels[16 * 4]);
  memset(orig_y, y, kPixels);

  /* YUV converted to ARGB. */
  I400ToARGB(orig_y, kWidth, orig_pixels, kWidth * 4, kWidth, kHeight);

  *b = orig_pixels[0];
  *g = orig_pixels[1];
  *r = orig_pixels[2];
}

static void YJToRGB(int y, int* r, int* g, int* b) {
  const int kWidth = 16;
  const int kHeight = 1;
  const int kPixels = kWidth * kHeight;

  SIMD_ALIGNED(uint8_t orig_y[16]);
  SIMD_ALIGNED(uint8_t orig_pixels[16 * 4]);
  memset(orig_y, y, kPixels);

  /* YUV converted to ARGB. */
  J400ToARGB(orig_y, kWidth, orig_pixels, kWidth * 4, kWidth, kHeight);

  *b = orig_pixels[0];
  *g = orig_pixels[1];
  *r = orig_pixels[2];
}

// Pick a method for clamping.
//  #define CLAMPMETHOD_IF 1
//  #define CLAMPMETHOD_TABLE 1
#define CLAMPMETHOD_TERNARY 1
//  #define CLAMPMETHOD_MASK 1

// Pick a method for rounding.
#define ROUND(f) static_cast<int>(f + 0.5f)
//  #define ROUND(f) lrintf(f)
//  #define ROUND(f) static_cast<int>(round(f))
//  #define ROUND(f) _mm_cvt_ss2si(_mm_load_ss(&f))

#if defined(CLAMPMETHOD_IF)
static int RoundToByte(float f) {
  int i = ROUND(f);
  if (i < 0) {
    i = 0;
  }
  if (i > 255) {
    i = 255;
  }
  return i;
}
#elif defined(CLAMPMETHOD_TABLE)
static const unsigned char clamptable[811] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   1,   2,   3,   4,   5,   6,   7,   8,
    9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,
    24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
    39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,
    54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,
    69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
    84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,
    99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
    114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
    129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
    159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173,
    174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
    189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,
    204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
    219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
    234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
    249, 250, 251, 252, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255};

static int RoundToByte(float f) {
  return clamptable[ROUND(f) + 276];
}
#elif defined(CLAMPMETHOD_TERNARY)
static int RoundToByte(float f) {
  int i = ROUND(f);
  return (i < 0) ? 0 : ((i > 255) ? 255 : i);
}
#elif defined(CLAMPMETHOD_MASK)
static int RoundToByte(float f) {
  int i = ROUND(f);
  i = ((-(i) >> 31) & (i));                  // clamp to 0.
  return (((255 - (i)) >> 31) | (i)) & 255;  // clamp to 255.
}
#endif

#define RANDOM256(s) ((s & 1) ? ((s >> 1) ^ 0xb8) : (s >> 1))

TEST_F(LibYUVColorTest, TestRoundToByte) {
  int allb = 0;
  int count = benchmark_width_ * benchmark_height_;
  for (int i = 0; i < benchmark_iterations_; ++i) {
    float f = (fastrand() & 255) * 3.14f - 260.f;
    for (int j = 0; j < count; ++j) {
      int b = RoundToByte(f);
      f += 0.91f;
      allb |= b;
    }
  }
  EXPECT_GE(allb, 0);
  EXPECT_LE(allb, 255);
}

// BT.601 YUV to RGB reference
static void YUVToRGBReference(int y, int u, int v, int* r, int* g, int* b) {
  *r = RoundToByte((y - 16) * 1.164 - (v - 128) * -1.596);
  *g = RoundToByte((y - 16) * 1.164 - (u - 128) * 0.391 - (v - 128) * 0.813);
  *b = RoundToByte((y - 16) * 1.164 - (u - 128) * -2.018);
}

// JPEG YUV to RGB reference
static void YUVJToRGBReference(int y, int u, int v, int* r, int* g, int* b) {
  *r = RoundToByte(y - (v - 128) * -1.40200);
  *g = RoundToByte(y - (u - 128) * 0.34414 - (v - 128) * 0.71414);
  *b = RoundToByte(y - (u - 128) * -1.77200);
}

// BT.709 YUV to RGB reference
// See also http://www.equasys.de/colorconversion.html
static void YUVHToRGBReference(int y, int u, int v, int* r, int* g, int* b) {
  *r = RoundToByte((y - 16) * 1.164 - (v - 128) * -1.793);
  *g = RoundToByte((y - 16) * 1.164 - (u - 128) * 0.213 - (v - 128) * 0.533);
  *b = RoundToByte((y - 16) * 1.164 - (u - 128) * -2.112);
}

// BT.2020 YUV to RGB reference
static void YUVRec2020ToRGBReference(int y,
                                     int u,
                                     int v,
                                     int* r,
                                     int* g,
                                     int* b) {
  *r = RoundToByte((y - 16) * 1.164384 - (v - 128) * -1.67867);
  *g = RoundToByte((y - 16) * 1.164384 - (u - 128) * 0.187326 -
                   (v - 128) * 0.65042);
  *b = RoundToByte((y - 16) * 1.164384 - (u - 128) * -2.14177);
}

// Pre-computed coefficients for YUV to RGB conversion

static float MatrixFromMCCodePointFull[][3][4] = {
    // Identity (not supported currently)
    {{0, 0, 1, 0}, {1, 0, 0, 0}, {0, 1, 0, 0}},
    // BT.709
    {{1, 0, 1.5748, -201.5744},
     {1, -0.18732427293064876957, -0.46812427293064876957,
      83.89741387024608501},
     {1, 1.8556, 0, -237.5168}},
    // Unspecified
    {},
    // Reserved
    {},
    // FCC
    {{1, 0, 1.4, -179.2},
     {1, -0.33186440677966101695, -0.7118644067796610169,
      133.59728813559322034},
     {1, 1.78, 0, -227.84}},
    // BT.601 625
    {{1, 0, 1.402, -179.456},
     {1, -0.34413628620102214651, -0.7141362862010221465,
      135.45888926746166951},
     {1, 1.772, 0, -226.816}},
    // BT.601 525
    {{1, 0, 1.402, -179.456},
     {1, -0.34413628620102214651, -0.7141362862010221465,
      135.45888926746166951},
     {1, 1.772, 0, -226.816}},
    // SMPTE 240M
    {{1, 0, 1.576, -201.728},
     {1, -0.22662196861626248217, -0.47662196861626248217,
      90.01522396576319544},
     {1, 1.826, 0, -233.728}},
    // YCgCo (not supported currently)
    {{1, -1, 1, 0}, {1, 1, 0, -128}, {1, -1, -1, 256}},
    // BT.2020 (non-constant luminance)
    {{1, 0, 1.4746, -188.7488},
     {1, -0.16455312684365781711, -0.5713531268436578171, 94.19600047197640118},
     {1, 1.8814, 0, -240.8192}},
    // BT.2020 (constant luminance) (not linear transformation)
    {},
    // SMPTE ST 2085 (not supported currently)
    {{0.991902, 0, 2, -256},
     {1, 0, 0, 0},
     {1.0136169298354088829, 2.0272338596708177659, 0, -259.48593403786467403}},
    // Chromaticity-derived non-constant luminance (see other)
    {},
    // Chromaticity-derived constant luminance (not linear transformation)
    {},
    // ICtCp (not linear transformation)
    {},
};

static float MatrixFromMCCodePointLimited[][3][4] = {
    // Identity (not supported currently)
    {{0, 0, 85. / 73., -1360. / 73.},
     {85. / 73., 0, 0, -1360. / 73.},
     {0, 85. / 73., 0, -1360. / 73.}},
    // BT.709
    {{85. / 73., 0, 1.79274107142857142857, -248.100994129158512720},
     {85. / 73., -0.21324861427372962608, -0.53290932855944391179,
      76.87807969634484298},
     {85. / 73., 2.11240178571428571429, 0, -289.017565557729941292}},
    // Unspecified
    {},
    // Reserved
    {},
    // FCC
    {{85. / 73., 0, 1.59375, -222.630136986301369863},
     {85. / 73., -0.37779207021791767554, -0.8103813559322033898,
      133.45606156091412651},
     {85. / 73., 2.02633928571428571429, 0, -278.001565557729941292}},
    // BT.601 625
    {{85. / 73., 0, 1.59602678571428571429, -222.921565557729941292},
     {85. / 73., -0.39176229009491360428, -0.8129676472377707471,
      135.57529499228222712},
     {85. / 73., 2.01723214285714285714, 0, -276.835851272015655577}},
    // BT.601 525
    {{85. / 73., 0, 1.59602678571428571429, -222.921565557729941292},
     {85. / 73., -0.39176229009491360428, -0.8129676472377707471,
      135.57529499228222712},
     {85. / 73., 2.01723214285714285714, 0, -276.835851272015655577}},
    // SMPTE 240M
    {{85. / 73., 0, 1.79410714285714285714, -248.275851272015655577},
     {85. / 73., -0.25798483034440595068, -0.54258304463012023640,
      83.84255101043798208},
     {85. / 73., 2.07870535714285714286, 0, -284.704422700587084149}},
    // YCgCo (not supported currently)
    {{85. / 73., -85. / 73., 85. / 73., -1360. / 73.},
     {85. / 73., 85. / 73., 0, -12240. / 73.},
     {85. / 73., -85. / 73., -85. / 73., 20400. / 73.}},
    // BT.2020 (non-constant luminance)
    {{85. / 73., 0, 1.67867410714285714286, -233.500422700587084149},
     {85. / 73., -0.18732610421934260430, -0.65042431850505689,
      88.60191712242176541},
     {85. / 73., 2.14177232142857142857, 0, -292.776994129158512720}},
    // BT.2020 (constant luminance) (not linear transformation)
    {},
    // SMPTE ST 2085 (not supported currently)
    {{1.1549543835616438356, 0, 2.2767857142857142857, -309.90784156555772994},
     {1.1643835616438356164, 0, 0, -18.630136986301369863},
     {1.1802388909042432199, 2.3077885456074041531, 0, -314.28075609221562312}},
    // Chromaticity-derived non-constant luminance (see other)
    {},
    // Chromaticity-derived constant luminance (not linear transformation)
    {},
    // ICtCp (not linear transformation)
    {},
};

static float MatrixFromCPCodePointFull[][3][4] = {
    // Reserved
    {},
    // BT.709
    {{1, 0, 1.5747219882569792849, -201.5644144968933485},
     {1, -0.1873140895347889792, -0.468207470556342264, 83.9067596916647992},
     {1, 1.8556153692785325700, 0, -237.51876726765216896}},
    // Unspecified
    {},
    // Reserved
    {},
    // FCC
    {{1, 0, 1.4020667637504200739, -179.4645457600537695},
     {1, -0.3460864650777044737, -0.714795357842831136, 135.7928733338285580},
     {1, 1.7707756565155467362, 0, -226.65928403398998224}},
    // BT.601 625
    {{1, 0, 1.5559913800035380582, -199.1668966404528715},
     {1, -0.1875071104675869672, -0.488833882311076136, 86.5716470756688772},
     {1, 1.8573181518470272270, 0, -237.73672343641948505}},
    // BT.601 525
    {{1, 0, 1.575247278589864846, -201.6316516595027003},
     {1, -0.2255741593817236071, -0.477199316053439653, 89.9550048557008972},
     {1, 1.8268724352615808913, 0, -233.83967171348235409}},
    // SMPTE 240M
    {{1, 0, 1.575247278589864846, -201.6316516595027003},
     {1, -0.2255741593817236071, -0.477199316053439653, 89.9550048557008972},
     {1, 1.8268724352615808913, 0, -233.83967171348235409}},
    // Generic Film
    {{1, 0, 1.4928292731325306765, -191.08214696096392659},
     {1, -0.1870581851792938231, -0.558071190308177901, 95.3765600623963807},
     {1, 1.8638422782614147547, 0, -238.57181161746108860}},
    // BT.2020
    {{1, 0, 1.4745995759774659384, -188.74874572511564011},
     {1, -0.1645580577201903188, -0.571355048803000488, 94.1968776349684232},
     {1, 1.8813965670602761071, 0, -240.81876058371534171}},
    // SMPTE ST 428-1 (XYZ)
    {{1, 0, 2, -256}, {1, 0, 0, 0}, {1, 2, 0, -256}},
    // SMPTE RP 431-2
    {{1, 0, 1.5810166441745389114, -202.37013045434098066},
     {1, -0.1778394650608421271, -0.458996685033851574, 81.5150272121207937},
     {1, 1.8621738641475483581, 0, -238.35825461088618983}},
    // SMPTE EG 432-1
    {{1, 0, 1.5420508718605023196, -197.38251159814429691},
     {1, -0.2110638544559469287, -0.510439154407954607, 92.3523851345793966},
     {1, 1.8414261718125099984, 0, -235.70254999200127980}},
    // Reserved
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    // EBU Tech. 3213-E
    {{1, 0, 1.536498908655781539, -196.6718603079400370},
     {1, -0.2581861953299589906, -0.529689923627720535, 100.8481432265829792},
     {1, 1.8080026369535430832, 0, -231.42433753005351465}},
};

static float MatrixFromCPCodePointLimited[][3][4] = {
    // Reserved
    {},
    // BT.709
    {{85. / 73., 0, 1.792652263417543382, -248.0896267037469228},
     {85. / 73., -0.2132370215686213826, -0.533004040142264631,
      76.8887189126920399},
     {85. / 73., 2.1124192819911866310, 0, -289.01980508117325863}},
    // Unspecified
    {},
    // Reserved
    {},
    // FCC
    {{85. / 73., 0, 1.5961027890908799949, -222.9312939899340092},
     {85. / 73., -0.393982359798279646, -0.813717929687151516,
      135.9555000678338190},
     {85. / 73., 2.0158383589797518649, 0, -276.65744693570960857}},
    // BT.601 625
    {{85. / 73., 0, 1.771329472771884843, -245.3603095011026298},
     {85. / 73., -0.2134567552197976635, -0.556484999952341137,
      79.9224076757323966},
     {85. / 73., 2.1143577175044283164, 0, -289.26792482686819436}},
    // BT.601 525
    {{85. / 73., 0, 1.793250250180426499, -248.1661690093959617},
     {85. / 73., -0.2567920117961585706, -0.543240292828692462,
      83.7739980056795623},
     {85. / 73., 2.0796985312129603897, 0, -284.83154898156029974}},
    // SMPTE 240M
    {{85. / 73., 0, 1.793250250180426499, -248.1661690093959617},
     {85. / 73., -0.2567920117961585706, -0.543240292828692462,
      83.7739980056795623},
     {85. / 73., 2.0796985312129603897, 0, -284.83154898156029974}},
    // Generic Film
    {{85. / 73., 0, 1.6994261814678362612, -236.1566882141844113},
     {85. / 73., -0.2129457018782139504, -0.635304256824041807,
      89.9458577275873671},
     {85. / 73., 2.1217847364136641181, 0, -290.21858324725037698}},
    // BT.2020
    {{85. / 73., 0, 1.6786736244386330995, -233.50036091444640660},
     {85. / 73., -0.1873317174939666576, -0.650426506449844305,
      88.6029156785064334},
     {85. / 73., 2.1417684133945107469, 0, -292.77649390079874547}},
    // SMPTE ST 428-1 (XYZ)
    {{85. / 73., 0, 255. / 112., -158440. / 511.},
     {85. / 73., 0, 0, -1360. / 73.},
     {85. / 73., 255. / 112., 0, -158440. / 511.}},
    // SMPTE RP 431-2
    {{85. / 73., 0, 1.7998180547522652786, -249.0068479945913255},
     {85. / 73., -0.2024511767433693857, -0.522518547694786390,
      74.1659877417825694},
     {85. / 73., 2.1198854257036822826, 0, -289.97547147637270204}},
    // SMPTE EG 432-1
    {{85. / 73., 0, 1.7554596978769111228, -243.3289783145459936},
     {85. / 73., -0.2402735843136895840, -0.581080287384055468,
      86.5031585910099968},
     {85. / 73., 2.0962664009472770071, 0, -286.95223630755282677}},
    // Reserved
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    // EBU Tech. 3213-E
    {{85. / 73., 0, 1.749139382621537020, -242.5199779618581084},
     {85. / 73., -0.2939173205765158152, -0.602995225558342573,
      96.1746689189605038},
     {85. / 73., 2.0582172876033637777, 0, -282.08194979953193341}},
};

// YUV to RGB custom coefficient reference
static void YUVMatrixToRGBReference(int y,
                                    int u,
                                    int v,
                                    int* r,
                                    int* g,
                                    int* b,
                                    const float matrix[3][4]) {
  *r = RoundToByte(matrix[0][0] * y + matrix[0][1] * u + matrix[0][2] * v +
                   matrix[0][3]);
  *g = RoundToByte(matrix[1][0] * y + matrix[1][1] * u + matrix[1][2] * v +
                   matrix[1][3]);
  *b = RoundToByte(matrix[2][0] * y + matrix[2][1] * u + matrix[2][2] * v +
                   matrix[2][3]);
}

TEST_F(LibYUVColorTest, TestYUV) {
  int r0, g0, b0, r1, g1, b1;

  // cyan (less red)
  YUVToRGBReference(240, 255, 0, &r0, &g0, &b0);
  EXPECT_EQ(56, r0);
  EXPECT_EQ(255, g0);
  EXPECT_EQ(255, b0);

  YUVToRGB(240, 255, 0, &r1, &g1, &b1);
  EXPECT_EQ(57, r1);
  EXPECT_EQ(255, g1);
  EXPECT_EQ(255, b1);

  // green (less red and blue)
  YUVToRGBReference(240, 0, 0, &r0, &g0, &b0);
  EXPECT_EQ(56, r0);
  EXPECT_EQ(255, g0);
  EXPECT_EQ(2, b0);

  YUVToRGB(240, 0, 0, &r1, &g1, &b1);
  EXPECT_EQ(57, r1);
  EXPECT_EQ(255, g1);
  EXPECT_EQ(5, b1);

  for (int i = 0; i < 256; ++i) {
    YUVToRGBReference(i, 128, 128, &r0, &g0, &b0);
    YUVToRGB(i, 128, 128, &r1, &g1, &b1);
    EXPECT_NEAR(r0, r1, ERROR_R);
    EXPECT_NEAR(g0, g1, ERROR_G);
    EXPECT_NEAR(b0, b1, ERROR_B);

    YUVToRGBReference(i, 0, 0, &r0, &g0, &b0);
    YUVToRGB(i, 0, 0, &r1, &g1, &b1);
    EXPECT_NEAR(r0, r1, ERROR_R);
    EXPECT_NEAR(g0, g1, ERROR_G);
    EXPECT_NEAR(b0, b1, ERROR_B);

    YUVToRGBReference(i, 0, 255, &r0, &g0, &b0);
    YUVToRGB(i, 0, 255, &r1, &g1, &b1);
    EXPECT_NEAR(r0, r1, ERROR_R);
    EXPECT_NEAR(g0, g1, ERROR_G);
    EXPECT_NEAR(b0, b1, ERROR_B);
  }
}

TEST_F(LibYUVColorTest, TestGreyYUV) {
  int r0, g0, b0, r1, g1, b1, r2, g2, b2;

  // black
  YUVToRGBReference(16, 128, 128, &r0, &g0, &b0);
  EXPECT_EQ(0, r0);
  EXPECT_EQ(0, g0);
  EXPECT_EQ(0, b0);

  YUVToRGB(16, 128, 128, &r1, &g1, &b1);
  EXPECT_EQ(0, r1);
  EXPECT_EQ(0, g1);
  EXPECT_EQ(0, b1);

  // white
  YUVToRGBReference(240, 128, 128, &r0, &g0, &b0);
  EXPECT_EQ(255, r0);
  EXPECT_EQ(255, g0);
  EXPECT_EQ(255, b0);

  YUVToRGB(240, 128, 128, &r1, &g1, &b1);
  EXPECT_EQ(255, r1);
  EXPECT_EQ(255, g1);
  EXPECT_EQ(255, b1);

  // grey
  YUVToRGBReference(128, 128, 128, &r0, &g0, &b0);
  EXPECT_EQ(130, r0);
  EXPECT_EQ(130, g0);
  EXPECT_EQ(130, b0);

  YUVToRGB(128, 128, 128, &r1, &g1, &b1);
  EXPECT_EQ(130, r1);
  EXPECT_EQ(130, g1);
  EXPECT_EQ(130, b1);

  for (int y = 0; y < 256; ++y) {
    YUVToRGBReference(y, 128, 128, &r0, &g0, &b0);
    YUVToRGB(y, 128, 128, &r1, &g1, &b1);
    YToRGB(y, &r2, &g2, &b2);
    EXPECT_EQ(r0, r1);
    EXPECT_EQ(g0, g1);
    EXPECT_EQ(b0, b1);
    EXPECT_EQ(r0, r2);
    EXPECT_EQ(g0, g2);
    EXPECT_EQ(b0, b2);
  }
}

static void PrintHistogram(int rh[256], int gh[256], int bh[256]) {
  int i;
  printf("hist");
  for (i = 0; i < 256; ++i) {
    if (rh[i] || gh[i] || bh[i]) {
      printf("\t%8d", i - 128);
    }
  }
  printf("\nred ");
  for (i = 0; i < 256; ++i) {
    if (rh[i] || gh[i] || bh[i]) {
      printf("\t%8d", rh[i]);
    }
  }
  printf("\ngreen");
  for (i = 0; i < 256; ++i) {
    if (rh[i] || gh[i] || bh[i]) {
      printf("\t%8d", gh[i]);
    }
  }
  printf("\nblue");
  for (i = 0; i < 256; ++i) {
    if (rh[i] || gh[i] || bh[i]) {
      printf("\t%8d", bh[i]);
    }
  }
  printf("\n");
}

// Step by 5 on inner loop goes from 0 to 255 inclusive.
// Set to 1 for better converage.  3, 5 or 17 for faster testing.
#ifdef ENABLE_SLOW_TESTS
#define FASTSTEP 1
#else
#define FASTSTEP 5
#endif
TEST_F(LibYUVColorTest, TestFullYUV) {
  printf("Test kYuvI601Constants:\n");
  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVToRGBReference(y, u, v, &r0, &g0, &b0);
        YUVToRGB(y, u, v, &r1, &g1, &b1);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        EXPECT_NEAR(b0, b1, ERROR_B);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVJ) {
  printf("Test kYuvJPEGConstants:\n");
  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVJToRGBReference(y, u, v, &r0, &g0, &b0);
        YUVJToRGB(y, u, v, &r1, &g1, &b1);
        EXPECT_NEAR(r0, r1, 1);
        EXPECT_NEAR(g0, g1, 1);
        EXPECT_NEAR(b0, b1, 1);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVH) {
  printf("Test kYuvH709Constants:\n");
  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVHToRGBReference(y, u, v, &r0, &g0, &b0);
        YUVHToRGB(y, u, v, &r1, &g1, &b1);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        // TODO(crbug.com/libyuv/862): Reduce the errors in the B channel.
        EXPECT_NEAR(b0, b1, 15);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVRec2020) {
  printf("Test kYuv2020Constants:\n");
  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVRec2020ToRGBReference(y, u, v, &r0, &g0, &b0);
        YUVRec2020ToRGB(y, u, v, &r1, &g1, &b1);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        // TODO(crbug.com/libyuv/863): Reduce the errors in the B channel.
        EXPECT_NEAR(b0, b1, 18);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

// cross test between matrix derived constants and hardcoded constants
TEST_F(LibYUVColorTest, TestFullYUVWithMatrix) {
  printf("Test kYuvI601Constants with Matrix:\n");
  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVMatrixToRGBReference(y, u, v, &r0, &g0, &b0,
                                MatrixFromMCCodePointLimited[6]);
        YUVToRGB(y, u, v, &r1, &g1, &b1);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        EXPECT_NEAR(b0, b1, ERROR_B);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVJWithMatrix) {
  printf("Test kYuvJPEGConstants with Matrix:\n");
  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVMatrixToRGBReference(y, u, v, &r0, &g0, &b0,
                                MatrixFromMCCodePointFull[6]);
        YUVJToRGB(y, u, v, &r1, &g1, &b1);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        EXPECT_NEAR(b0, b1, ERROR_B);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVHWithMatrix) {
  printf("Test kYuvH709Constants with Matrix:\n");
  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVMatrixToRGBReference(y, u, v, &r0, &g0, &b0,
                                MatrixFromMCCodePointLimited[1]);
        YUVHToRGB(y, u, v, &r1, &g1, &b1);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_R);
        // TODO(crbug.com/libyuv/862): Reduce the errors in the B channel.
        EXPECT_NEAR(b0, b1, 15);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVRec2020WithMatrix) {
  printf("Test kYuv2020Constants with Matrix:\n");
  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVMatrixToRGBReference(y, u, v, &r0, &g0, &b0,
                                MatrixFromMCCodePointLimited[9]);
        YUVRec2020ToRGB(y, u, v, &r1, &g1, &b1);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        // TODO(crbug.com/libyuv/863): Reduce the errors in the B channel.
        EXPECT_NEAR(b0, b1, 18);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVMatrixWithI601) {
  printf("Test BT.601 Limited Matrix:\n");
  struct YuvConstants SIMD_ALIGNED(constants) {};
  auto result = InitYuvConstantsWithMCCodePoint(&constants, 6, 0, 1);
  ASSERT_EQ(0, result);

  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVToRGBReference(y, u, v, &r0, &g0, &b0);
        YUVMatrixToRGB(y, u, v, &r1, &g1, &b1, &constants);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        EXPECT_NEAR(b0, b1, ERROR_B);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVMatrixWithJPEG) {
  printf("Test JPEG (BT.601 Full) Matrix:\n");
  struct YuvConstants SIMD_ALIGNED(constants) {};
  auto result = InitYuvConstantsWithMCCodePoint(&constants, 6, 1, 1);
  ASSERT_EQ(0, result);

  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVJToRGBReference(y, u, v, &r0, &g0, &b0);
        YUVMatrixToRGB(y, u, v, &r1, &g1, &b1, &constants);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        EXPECT_NEAR(b0, b1, ERROR_B);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVMatrixWithH709) {
  printf("Test BT.709 Limited Matrix:\n");
  struct YuvConstants SIMD_ALIGNED(constants) {};
  auto result = InitYuvConstantsWithMCCodePoint(&constants, 1, 0, 1);
  ASSERT_EQ(0, result);

  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVHToRGBReference(y, u, v, &r0, &g0, &b0);
        YUVMatrixToRGB(y, u, v, &r1, &g1, &b1, &constants);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        // TODO(crbug.com/libyuv/862): Reduce the errors in the B channel.
        EXPECT_NEAR(b0, b1, 15);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVMatrixWith2020) {
  printf("Test BT.2020 Limited Matrix:\n");
  struct YuvConstants SIMD_ALIGNED(constants) {};
  auto result = InitYuvConstantsWithMCCodePoint(&constants, 9, 0, 1);
  ASSERT_EQ(0, result);

  int rh[256] = {
      0,
  };
  int gh[256] = {
      0,
  };
  int bh[256] = {
      0,
  };
  for (int u = 0; u < 256; ++u) {
    for (int v = 0; v < 256; ++v) {
      for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
        int r0, g0, b0, r1, g1, b1;
        int y = RANDOM256(y2);
        YUVRec2020ToRGBReference(y, u, v, &r0, &g0, &b0);
        YUVMatrixToRGB(y, u, v, &r1, &g1, &b1, &constants);
        EXPECT_NEAR(r0, r1, ERROR_R);
        EXPECT_NEAR(g0, g1, ERROR_G);
        // TODO(crbug.com/libyuv/863): Reduce the errors in the B channel.
        EXPECT_NEAR(b0, b1, 18);
        ++rh[r1 - r0 + 128];
        ++gh[g1 - g0 + 128];
        ++bh[b1 - b0 + 128];
      }
    }
  }
  PrintHistogram(rh, gh, bh);
}

TEST_F(LibYUVColorTest, TestFullYUVMatrixWithMatrixCoefficients) {
  printf("Test Matrix from MatrixCoefficients code point:\n");
  const uint8_t MC[] = {1, 4, 5, 6, 7, 9};
  const int full[] = {0, 1};

  for (auto f : full) {
    if (f) {
      printf("\n\nTest full range:");
    } else {
      printf("Test limited range:");
    }

    for (auto code_point : MC) {
      int rh[256] = {
          0,
      };
      int gh[256] = {
          0,
      };
      int bh[256] = {
          0,
      };

      float(*mat)[3][4];
      if (f) {
        mat = &MatrixFromMCCodePointFull[code_point];
      } else {
        mat = &MatrixFromMCCodePointLimited[code_point];
      }

      struct YuvConstants SIMD_ALIGNED(constants) {};
      auto result =
          InitYuvConstantsWithMCCodePoint(&constants, code_point, f, 1);
      ASSERT_EQ(0, result);

      // if coefficient > 2, loosen restrict for now
      const int error_r_loosen = static_cast<int>(((*mat)[0][2] - 2) * 160);
      const int error_b_loosen = static_cast<int>(((*mat)[2][1] - 2) * 160);

      printf("\nResult for MC=%d:\n", code_point);
      for (int u = 0; u < 256; ++u) {
        for (int v = 0; v < 256; ++v) {
          for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
            int r0, g0, b0, r1, g1, b1;
            int y = RANDOM256(y2);
            YUVMatrixToRGBReference(y, u, v, &r0, &g0, &b0, *mat);
            YUVMatrixToRGB(y, u, v, &r1, &g1, &b1, &constants);
            if (error_r_loosen > ERROR_R) {
              EXPECT_NEAR(r0, r1, error_r_loosen);
            } else {
              EXPECT_NEAR(r0, r1, ERROR_R);
            }
            EXPECT_NEAR(g0, g1, ERROR_G);
            if (error_b_loosen > ERROR_B) {
              EXPECT_NEAR(b0, b1, error_b_loosen);
            } else {
              EXPECT_NEAR(b0, b1, ERROR_B);
            }
            ++rh[r1 - r0 + 128];
            ++gh[g1 - g0 + 128];
            ++bh[b1 - b0 + 128];
          }
        }
      }
      PrintHistogram(rh, gh, bh);
    }
  }
}

TEST_F(LibYUVColorTest, TestFullYUVMatrixWithColorPrimaries) {
  printf("Test Matrix from ColorPrimaries code point:\n");
  const uint8_t CP[] = {1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 22};
  const int full[] = {0, 1};

  for (auto f : full) {
    if (f) {
      printf("\n\nTest full range:");
    } else {
      printf("Test limited range:");
    }

    for (auto code_point : CP) {
      int rh[256] = {
          0,
      };
      int gh[256] = {
          0,
      };
      int bh[256] = {
          0,
      };

      float(*mat)[3][4];
      if (f) {
        mat = &MatrixFromCPCodePointFull[code_point];
      } else {
        mat = &MatrixFromCPCodePointLimited[code_point];
      }

      struct YuvConstants SIMD_ALIGNED(constants) {};
      auto result =
          InitYuvConstantsWithCPCodePoint(&constants, code_point, f, 1);
      ASSERT_EQ(0, result);

      // if coefficient > 2, loosen restrict for now
      const int error_r_loosen = static_cast<int>(((*mat)[0][2] - 2) * 160);
      const int error_b_loosen = static_cast<int>(((*mat)[2][1] - 2) * 160);

      printf("\nResult for CP=%d:\n", code_point);
      for (int u = 0; u < 256; ++u) {
        for (int v = 0; v < 256; ++v) {
          for (int y2 = 0; y2 < 256; y2 += FASTSTEP) {
            int r0, g0, b0, r1, g1, b1;
            int y = RANDOM256(y2);
            YUVMatrixToRGBReference(y, u, v, &r0, &g0, &b0, *mat);
            YUVMatrixToRGB(y, u, v, &r1, &g1, &b1, &constants);
            if (error_r_loosen > ERROR_R) {
              EXPECT_NEAR(r0, r1, error_r_loosen);
            } else {
              EXPECT_NEAR(r0, r1, ERROR_R);
            }
            EXPECT_NEAR(g0, g1, ERROR_G);
            if (error_b_loosen > ERROR_B) {
              EXPECT_NEAR(b0, b1, error_b_loosen);
            } else {
              EXPECT_NEAR(b0, b1, ERROR_B);
            }
            ++rh[r1 - r0 + 128];
            ++gh[g1 - g0 + 128];
            ++bh[b1 - b0 + 128];
          }
        }
      }
      PrintHistogram(rh, gh, bh);
    }
  }
}
#undef FASTSTEP

TEST_F(LibYUVColorTest, TestGreyYUVJ) {
  int r0, g0, b0, r1, g1, b1, r2, g2, b2;

  // black
  YUVJToRGBReference(0, 128, 128, &r0, &g0, &b0);
  EXPECT_EQ(0, r0);
  EXPECT_EQ(0, g0);
  EXPECT_EQ(0, b0);

  YUVJToRGB(0, 128, 128, &r1, &g1, &b1);
  EXPECT_EQ(0, r1);
  EXPECT_EQ(0, g1);
  EXPECT_EQ(0, b1);

  // white
  YUVJToRGBReference(255, 128, 128, &r0, &g0, &b0);
  EXPECT_EQ(255, r0);
  EXPECT_EQ(255, g0);
  EXPECT_EQ(255, b0);

  YUVJToRGB(255, 128, 128, &r1, &g1, &b1);
  EXPECT_EQ(255, r1);
  EXPECT_EQ(255, g1);
  EXPECT_EQ(255, b1);

  // grey
  YUVJToRGBReference(128, 128, 128, &r0, &g0, &b0);
  EXPECT_EQ(128, r0);
  EXPECT_EQ(128, g0);
  EXPECT_EQ(128, b0);

  YUVJToRGB(128, 128, 128, &r1, &g1, &b1);
  EXPECT_EQ(128, r1);
  EXPECT_EQ(128, g1);
  EXPECT_EQ(128, b1);

  for (int y = 0; y < 256; ++y) {
    YUVJToRGBReference(y, 128, 128, &r0, &g0, &b0);
    YUVJToRGB(y, 128, 128, &r1, &g1, &b1);
    YJToRGB(y, &r2, &g2, &b2);
    EXPECT_EQ(r0, r1);
    EXPECT_EQ(g0, g1);
    EXPECT_EQ(b0, b1);
    EXPECT_EQ(r0, r2);
    EXPECT_EQ(g0, g2);
    EXPECT_EQ(b0, b2);
  }
}

}  // namespace libyuv
