/*
 *  Copyright 2016 The LibYuv Project Authors. All rights reserved.
 *
 *  Copyright (c) 2020 Loongson Technology Corporation Limited
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <string.h>

#include "libyuv/row.h"

#if !defined(LIBYUV_DISABLE_LSX) && defined(__loongarch_sx)
#include "libyuv/loongson_intrinsics.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

void ARGB4444ToARGBRow_LSX(const uint8_t* src_argb4444,
                           uint8_t* dst_argb,
                           int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i reg0, reg1, reg2, reg3;
  __m128i dst0, dst1, dst2, dst3;

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_argb4444, 0);
    src1 = __lsx_vld(src_argb4444, 16);
    tmp0 = __lsx_vandi_b(src0, 0x0F);
    tmp1 = __lsx_vandi_b(src0, 0xF0);
    tmp2 = __lsx_vandi_b(src1, 0x0F);
    tmp3 = __lsx_vandi_b(src1, 0xF0);
    reg0 = __lsx_vslli_b(tmp0, 4);
    reg2 = __lsx_vslli_b(tmp2, 4);
    reg1 = __lsx_vsrli_b(tmp1, 4);
    reg3 = __lsx_vsrli_b(tmp3, 4);
    DUP4_ARG2(__lsx_vor_v, tmp0, reg0, tmp1, reg1, tmp2, reg2,
              tmp3, reg3, tmp0, tmp1, tmp2, tmp3);
    dst0 = __lsx_vilvl_b(tmp1, tmp0);
    dst2 = __lsx_vilvl_b(tmp3, tmp2);
    dst1 = __lsx_vilvh_b(tmp1, tmp0);
    dst3 = __lsx_vilvh_b(tmp3, tmp2);
    __lsx_vst(dst0, dst_argb, 0);
    __lsx_vst(dst1, dst_argb, 16);
    __lsx_vst(dst2, dst_argb, 32);
    __lsx_vst(dst3, dst_argb, 48);
    dst_argb += 64;
    src_argb4444 += 32;
  }
}

void ARGB1555ToARGBRow_LSX(const uint8_t* src_argb1555,
                           uint8_t* dst_argb,
                           int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1;
  __m128i tmp0, tmp1, tmpb, tmpg, tmpr, tmpa;
  __m128i reg0, reg1, reg2;
  __m128i dst0, dst1, dst2, dst3;

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_argb1555, 0);
    src1 = __lsx_vld(src_argb1555, 16);
    tmp0 = __lsx_vpickev_b(src1, src0);
    tmp1 = __lsx_vpickod_b(src1, src0);
    tmpb = __lsx_vandi_b(tmp0, 0x1F);
    tmpg = __lsx_vsrli_b(tmp0, 5);
    reg0 = __lsx_vandi_b(tmp1, 0x03);
    reg0 = __lsx_vslli_b(reg0, 3);
    tmpg = __lsx_vor_v(tmpg, reg0);
    reg1 = __lsx_vandi_b(tmp1, 0x7C);
    tmpr = __lsx_vsrli_b(reg1, 2);
    tmpa = __lsx_vsrli_b(tmp1, 7);
    tmpa = __lsx_vneg_b(tmpa);
    reg0 = __lsx_vslli_b(tmpb, 3);
    reg1 = __lsx_vslli_b(tmpg, 3);
    reg2 = __lsx_vslli_b(tmpr, 3);
    tmpb = __lsx_vsrli_b(tmpb, 2);
    tmpg = __lsx_vsrli_b(tmpg, 2);
    tmpr = __lsx_vsrli_b(tmpr, 2);
    tmpb = __lsx_vor_v(reg0, tmpb);
    tmpg = __lsx_vor_v(reg1, tmpg);
    tmpr = __lsx_vor_v(reg2, tmpr);
    DUP2_ARG2(__lsx_vilvl_b, tmpg, tmpb, tmpa, tmpr, reg0, reg1);
    dst0 = __lsx_vilvl_h(reg1, reg0);
    dst1 = __lsx_vilvh_h(reg1, reg0);
    DUP2_ARG2(__lsx_vilvh_b, tmpg, tmpb, tmpa, tmpr, reg0, reg1);
    dst2 = __lsx_vilvl_h(reg1, reg0);
    dst3 = __lsx_vilvh_h(reg1, reg0);
    __lsx_vst(dst0, dst_argb, 0);
    __lsx_vst(dst1, dst_argb, 16);
    __lsx_vst(dst2, dst_argb, 32);
    __lsx_vst(dst3, dst_argb, 48);
    dst_argb += 64;
    src_argb1555 += 32;
  }
}

void RGB565ToARGBRow_LSX(const uint8_t* src_rgb565,
                         uint8_t* dst_argb,
                         int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1;
  __m128i tmp0, tmp1, tmpb, tmpg, tmpr;
  __m128i reg0, reg1, dst0, dst1, dst2, dst3;
  __m128i alpha = __lsx_vldi(0xFF);

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_rgb565, 0);
    src1 = __lsx_vld(src_rgb565, 16);
    tmp0 = __lsx_vpickev_b(src1, src0);
    tmp1 = __lsx_vpickod_b(src1, src0);
    tmpb = __lsx_vandi_b(tmp0, 0x1F);
    tmpr = __lsx_vandi_b(tmp1, 0xF8);
    reg1 = __lsx_vandi_b(tmp1, 0x07);
    reg0 = __lsx_vsrli_b(tmp0, 5);
    reg1 = __lsx_vslli_b(reg1, 3);
    tmpg = __lsx_vor_v(reg1, reg0);
    reg0 = __lsx_vslli_b(tmpb, 3);
    reg1 = __lsx_vsrli_b(tmpb, 2);
    tmpb = __lsx_vor_v(reg1, reg0);
    reg0 = __lsx_vslli_b(tmpg, 2);
    reg1 = __lsx_vsrli_b(tmpg, 4);
    tmpg = __lsx_vor_v(reg1, reg0);
    reg0 = __lsx_vsrli_b(tmpr, 5);
    tmpr = __lsx_vor_v(tmpr, reg0);
    DUP2_ARG2(__lsx_vilvl_b, tmpg, tmpb, alpha, tmpr, reg0, reg1);
    dst0 = __lsx_vilvl_h(reg1, reg0);
    dst1 = __lsx_vilvh_h(reg1, reg0);
    DUP2_ARG2(__lsx_vilvh_b, tmpg, tmpb, alpha, tmpr, reg0, reg1);
    dst2 = __lsx_vilvl_h(reg1, reg0);
    dst3 = __lsx_vilvh_h(reg1, reg0);
    __lsx_vst(dst0, dst_argb, 0);
    __lsx_vst(dst1, dst_argb, 16);
    __lsx_vst(dst2, dst_argb, 32);
    __lsx_vst(dst3, dst_argb, 48);
    dst_argb += 64;
    src_rgb565 += 32;
  }
}

void RGB24ToARGBRow_LSX(const uint8_t* src_rgb24,
                        uint8_t* dst_argb,
                        int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, src2;
  __m128i tmp0, tmp1, tmp2;
  __m128i dst0, dst1, dst2, dst3;
  __m128i alpha = __lsx_vldi(0xFF);
  __m128i shuf0 = {0x131211100F0E0D0C, 0x1B1A191817161514};
  __m128i shuf1 = {0x1F1E1D1C1B1A1918, 0x0706050403020100};
  __m128i shuf2 = {0x0B0A090807060504, 0x131211100F0E0D0C};
  __m128i shuf3 = {0x1005040310020100, 0x100B0A0910080706};

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_rgb24, 0);
    src1 = __lsx_vld(src_rgb24, 16);
    src2 = __lsx_vld(src_rgb24, 32);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, shuf0, src1, src2, shuf1, tmp0, tmp1);
    tmp2 = __lsx_vshuf_b(src1, src2, shuf2);
    DUP4_ARG3(__lsx_vshuf_b, alpha, src0, shuf3, alpha, tmp0, shuf3, alpha, tmp1, shuf3,
                  alpha, tmp2, shuf3, dst0, dst1, dst2, dst3);
    __lsx_vst(dst0, dst_argb, 0);
    __lsx_vst(dst1, dst_argb, 16);
    __lsx_vst(dst2, dst_argb, 32);
    __lsx_vst(dst3, dst_argb, 48);
    dst_argb += 64;
    src_rgb24 += 48;
  }
}

void RAWToARGBRow_LSX(const uint8_t* src_raw, uint8_t* dst_argb, int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, src2;
  __m128i tmp0, tmp1, tmp2;
  __m128i dst0, dst1, dst2, dst3;
  __m128i alpha = __lsx_vldi(0xFF);
  __m128i shuf0 = {0x131211100F0E0D0C, 0x1B1A191817161514};
  __m128i shuf1 = {0x1F1E1D1C1B1A1918, 0x0706050403020100};
  __m128i shuf2 = {0x0B0A090807060504, 0x131211100F0E0D0C};
  __m128i shuf3 = {0x1003040510000102, 0x10090A0B10060708};

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_raw, 0);
    src1 = __lsx_vld(src_raw, 16);
    src2 = __lsx_vld(src_raw, 32);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, shuf0, src1, src2, shuf1, tmp0, tmp1);
    tmp2 = __lsx_vshuf_b(src1, src2, shuf2);
    DUP4_ARG3(__lsx_vshuf_b, alpha, src0, shuf3, alpha, tmp0, shuf3, alpha, tmp1, shuf3,
              alpha, tmp2, shuf3, dst0, dst1, dst2, dst3);
    __lsx_vst(dst0, dst_argb, 0);
    __lsx_vst(dst1, dst_argb, 16);
    __lsx_vst(dst2, dst_argb, 32);
    __lsx_vst(dst3, dst_argb, 48);
    dst_argb += 64;
    src_raw += 48;
  }
}

void ARGB1555ToYRow_LSX(const uint8_t* src_argb1555,
                        uint8_t* dst_y,
                        int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1;
  __m128i tmp0, tmp1, tmpb, tmpg, tmpr;
  __m128i reg0, reg1, reg2, dst0;
  __m128i const_66  = __lsx_vldi(66);
  __m128i const_129 = __lsx_vldi(129);
  __m128i const_25  = __lsx_vldi(25);
  __m128i const_1080 = {0x1080108010801080, 0x1080108010801080};
  __m128i shuff = {0x0B030A0209010800, 0x0F070E060D050C04};

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_argb1555, 0);
    src1 = __lsx_vld(src_argb1555, 16);
    tmp0 = __lsx_vpickev_b(src1, src0);
    tmp1 = __lsx_vpickod_b(src1, src0);
    tmpb = __lsx_vandi_b(tmp0, 0x1F);
    tmpg = __lsx_vsrli_b(tmp0, 5);
    reg0 = __lsx_vandi_b(tmp1, 0x03);
    reg0 = __lsx_vslli_b(reg0, 3);
    tmpg = __lsx_vor_v(tmpg, reg0);
    reg1 = __lsx_vandi_b(tmp1, 0x7C);
    tmpr = __lsx_vsrli_b(reg1, 2);
    reg0 = __lsx_vslli_b(tmpb, 3);
    reg1 = __lsx_vslli_b(tmpg, 3);
    reg2 = __lsx_vslli_b(tmpr, 3);
    tmpb = __lsx_vsrli_b(tmpb, 2);
    tmpg = __lsx_vsrli_b(tmpg, 2);
    tmpr = __lsx_vsrli_b(tmpr, 2);
    tmpb = __lsx_vor_v(reg0, tmpb);
    tmpg = __lsx_vor_v(reg1, tmpg);
    tmpr = __lsx_vor_v(reg2, tmpr);
    reg0 = __lsx_vmaddwev_h_bu(const_1080, tmpb, const_25);
    reg1 = __lsx_vmaddwod_h_bu(const_1080, tmpb, const_25);
    reg0 = __lsx_vmaddwev_h_bu(reg0, tmpg, const_129);
    reg1 = __lsx_vmaddwod_h_bu(reg1, tmpg, const_129);
    reg0 = __lsx_vmaddwev_h_bu(reg0, tmpr, const_66);
    reg1 = __lsx_vmaddwod_h_bu(reg1, tmpr, const_66);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    dst0 = __lsx_vshuf_b(dst0, dst0, shuff);
    __lsx_vst(dst0, dst_y, 0);
    dst_y += 16;
    src_argb1555 += 32;
  }
}

void ARGB1555ToUVRow_LSX(const uint8_t* src_argb1555,
                         int src_stride_argb1555,
                         uint8_t* dst_u,
                         uint8_t* dst_v,
                         int width) {
  int x;
  int len = width >> 4;
  const uint8_t* next_argb1555 = src_argb1555 + src_stride_argb1555;
  __m128i src0, src1, src2, src3;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i tmpb, tmpg, tmpr, nexb, nexg, nexr;
  __m128i reg0, reg1, reg2, reg3, dst0;
  __m128i const_112 = __lsx_vldi(0x438);
  __m128i const_74  = __lsx_vldi(0x425);
  __m128i const_38  = __lsx_vldi(0x413);
  __m128i const_94  = __lsx_vldi(0x42F);
  __m128i const_18  = __lsx_vldi(0x409);
  __m128i const_8080 = {0x8080808080808080, 0x8080808080808080};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_argb1555, 0, src_argb1555, 16,
              next_argb1555, 0, next_argb1555, 16, src0, src1, src2, src3);
    DUP2_ARG2(__lsx_vpickev_b, src1, src0, src3, src2, tmp0, tmp2);
    DUP2_ARG2(__lsx_vpickod_b, src1, src0, src3, src2, tmp1, tmp3);
    tmpb = __lsx_vandi_b(tmp0, 0x1F);
    nexb = __lsx_vandi_b(tmp2, 0x1F);
    tmpg = __lsx_vsrli_b(tmp0, 5);
    nexg = __lsx_vsrli_b(tmp2, 5);
    reg0 = __lsx_vandi_b(tmp1, 0x03);
    reg2 = __lsx_vandi_b(tmp3, 0x03);
    reg0 = __lsx_vslli_b(reg0, 3);
    reg2 = __lsx_vslli_b(reg2, 3);
    tmpg = __lsx_vor_v(tmpg, reg0);
    nexg = __lsx_vor_v(nexg, reg2);
    reg1 = __lsx_vandi_b(tmp1, 0x7C);
    reg3 = __lsx_vandi_b(tmp3, 0x7C);
    tmpr = __lsx_vsrli_b(reg1, 2);
    nexr = __lsx_vsrli_b(reg3, 2);
    reg0 = __lsx_vslli_b(tmpb, 3);
    reg1 = __lsx_vslli_b(tmpg, 3);
    reg2 = __lsx_vslli_b(tmpr, 3);
    tmpb = __lsx_vsrli_b(tmpb, 2);
    tmpg = __lsx_vsrli_b(tmpg, 2);
    tmpr = __lsx_vsrli_b(tmpr, 2);
    tmpb = __lsx_vor_v(reg0, tmpb);
    tmpg = __lsx_vor_v(reg1, tmpg);
    tmpr = __lsx_vor_v(reg2, tmpr);
    reg0 = __lsx_vslli_b(nexb, 3);
    reg1 = __lsx_vslli_b(nexg, 3);
    reg2 = __lsx_vslli_b(nexr, 3);
    nexb = __lsx_vsrli_b(nexb, 2);
    nexg = __lsx_vsrli_b(nexg, 2);
    nexr = __lsx_vsrli_b(nexr, 2);
    nexb = __lsx_vor_v(reg0, nexb);
    nexg = __lsx_vor_v(reg1, nexg);
    nexr = __lsx_vor_v(reg2, nexr);
    tmp0 = __lsx_vaddwev_h_bu(tmpb, nexb);
    tmp1 = __lsx_vaddwod_h_bu(tmpb, nexb);
    tmp2 = __lsx_vaddwev_h_bu(tmpg, nexg);
    tmp3 = __lsx_vaddwod_h_bu(tmpg, nexg);
    reg0 = __lsx_vaddwev_h_bu(tmpr, nexr);
    reg1 = __lsx_vaddwod_h_bu(tmpr, nexr);
    tmpb = __lsx_vavgr_hu(tmp0, tmp1);
    tmpg = __lsx_vavgr_hu(tmp2, tmp3);
    tmpr = __lsx_vavgr_hu(reg0, reg1);
    reg0 = __lsx_vmadd_h(const_8080, const_112, tmpb);
    reg1 = __lsx_vmadd_h(const_8080, const_112, tmpr);
    reg0 = __lsx_vmsub_h(reg0, const_74, tmpg);
    reg1 = __lsx_vmsub_h(reg1, const_94, tmpg);
    reg0 = __lsx_vmsub_h(reg0, const_38, tmpr);
    reg1 = __lsx_vmsub_h(reg1, const_18, tmpb);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_argb1555 += 32;
    next_argb1555 += 32;
  }
}

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // !defined(LIBYUV_DISABLE_LSX) && defined(__loongarch_sx)
