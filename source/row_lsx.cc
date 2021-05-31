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

// Fill YUV -> RGB conversion constants into vectors
#define YUVTORGB_SETUP(yuvconst, vr, ub, vg, ug, yg, yb)            \
  {                                                                 \
    ub = __lsx_vreplgr2vr_h(yuvconst->kUVToB[0]);                   \
    vr = __lsx_vreplgr2vr_h(yuvconst->kUVToR[1]);                   \
    ug = __lsx_vreplgr2vr_h(yuvconst->kUVToG[0]);                   \
    vg = __lsx_vreplgr2vr_h(yuvconst->kUVToG[1]);                   \
    yg = __lsx_vreplgr2vr_h(yuvconst->kYToRgb[0]);                  \
    yb = __lsx_vreplgr2vr_w(yuvconst->kYBiasToRgb[0]);              \
  }

// Convert 8 pixels of YUV420 to RGB.
#define YUVTORGB(in_y, in_vu, vrub, vgug,                           \
                 yg, yb, out_b, out_g, out_r)                       \
  {                                                                 \
    __m128i y_ev, y_od, u_l, v_l;                                   \
    __m128i tmp0, tmp1, tmp2, tmp3;                                 \
                                                                    \
    tmp0  = __lsx_vilvl_b(in_y, in_y);                              \
    y_ev  = __lsx_vmulwev_w_hu_h(tmp0, yg);                         \
    y_od  = __lsx_vmulwod_w_hu_h(tmp0, yg);                         \
    y_ev  = __lsx_vsrai_w(y_ev, 16);                                \
    y_od  = __lsx_vsrai_w(y_od, 16);                                \
    y_ev  = __lsx_vadd_w(y_ev, yb);                                 \
    y_od  = __lsx_vadd_w(y_od, yb);                                 \
    in_vu = __lsx_vilvl_b(zero, in_vu);                             \
    in_vu = __lsx_vsub_h(in_vu, const_80);                          \
    u_l   = __lsx_vmulwev_w_h(in_vu, vrub);                         \
    v_l   = __lsx_vmulwod_w_h(in_vu, vrub);                         \
    tmp0  = __lsx_vadd_w(y_ev, u_l);                                \
    tmp1  = __lsx_vadd_w(y_od, u_l);                                \
    tmp2  = __lsx_vadd_w(y_ev, v_l);                                \
    tmp3  = __lsx_vadd_w(y_od, v_l);                                \
    tmp0  = __lsx_vsrai_w(tmp0, 6);                                 \
    tmp1  = __lsx_vsrai_w(tmp1, 6);                                 \
    tmp2  = __lsx_vsrai_w(tmp2, 6);                                 \
    tmp3  = __lsx_vsrai_w(tmp3, 6);                                 \
    tmp0  = __lsx_vclip255_w(tmp0);                                 \
    tmp1  = __lsx_vclip255_w(tmp1);                                 \
    tmp2  = __lsx_vclip255_w(tmp2);                                 \
    tmp3  = __lsx_vclip255_w(tmp3);                                 \
    out_b = __lsx_vpackev_h(tmp1, tmp0);                            \
    out_r = __lsx_vpackev_h(tmp3, tmp2);                            \
    tmp0  = __lsx_vdp2_w_h(in_vu, vgug);                            \
    tmp1  = __lsx_vsub_w(y_ev, tmp0);                               \
    tmp2  = __lsx_vsub_w(y_od, tmp0);                               \
    tmp1  = __lsx_vsrai_w(tmp1, 6);                                 \
    tmp2  = __lsx_vsrai_w(tmp2, 6);                                 \
    tmp1  = __lsx_vclip255_w(tmp1);                                 \
    tmp2  = __lsx_vclip255_w(tmp2);                                 \
    out_g = __lsx_vpackev_h(tmp2, tmp1);                            \
  }

// Convert I444 pixels of YUV420 to RGB.
#define I444TORGB(in_yy, in_u, in_v, ub, vr, ugvg,                  \
                  yg, yb, out_b, out_g, out_r)                      \
  {                                                                 \
    __m128i y_ev, y_od, u_ev, v_ev, u_od, v_od;                     \
    __m128i tmp0, tmp1, tmp2, tmp3;                                 \
                                                                    \
    y_ev  = __lsx_vmulwev_w_hu_h(in_yy, yg);                        \
    y_od  = __lsx_vmulwod_w_hu_h(in_yy, yg);                        \
    y_ev  = __lsx_vsrai_w(y_ev, 16);                                \
    y_od  = __lsx_vsrai_w(y_od, 16);                                \
    y_ev  = __lsx_vadd_w(y_ev, yb);                                 \
    y_od  = __lsx_vadd_w(y_od, yb);                                 \
    in_u  = __lsx_vsub_h(in_u, const_80);                           \
    in_v  = __lsx_vsub_h(in_v, const_80);                           \
    u_ev  = __lsx_vmulwev_w_h(in_u, ub);                            \
    u_od  = __lsx_vmulwod_w_h(in_u, ub);                            \
    v_ev  = __lsx_vmulwev_w_h(in_v, vr);                            \
    v_od  = __lsx_vmulwod_w_h(in_v, vr);                            \
    tmp0  = __lsx_vadd_w(y_ev, u_ev);                               \
    tmp1  = __lsx_vadd_w(y_od, u_od);                               \
    tmp2  = __lsx_vadd_w(y_ev, v_ev);                               \
    tmp3  = __lsx_vadd_w(y_od, v_od);                               \
    tmp0  = __lsx_vsrai_w(tmp0, 6);                                 \
    tmp1  = __lsx_vsrai_w(tmp1, 6);                                 \
    tmp2  = __lsx_vsrai_w(tmp2, 6);                                 \
    tmp3  = __lsx_vsrai_w(tmp3, 6);                                 \
    tmp0  = __lsx_vclip255_w(tmp0);                                 \
    tmp1  = __lsx_vclip255_w(tmp1);                                 \
    tmp2  = __lsx_vclip255_w(tmp2);                                 \
    tmp3  = __lsx_vclip255_w(tmp3);                                 \
    out_b = __lsx_vpackev_h(tmp1, tmp0);                            \
    out_r = __lsx_vpackev_h(tmp3, tmp2);                            \
    u_ev  = __lsx_vpackev_h(in_u, in_v);                            \
    u_od  = __lsx_vpackod_h(in_u, in_v);                            \
    v_ev  = __lsx_vdp2_w_h(u_ev, ugvg);                             \
    v_od  = __lsx_vdp2_w_h(u_od, ugvg);                             \
    tmp0  = __lsx_vsub_w(y_ev, v_ev);                               \
    tmp1  = __lsx_vsub_w(y_od, v_od);                               \
    tmp0  = __lsx_vsrai_w(tmp0, 6);                                 \
    tmp1  = __lsx_vsrai_w(tmp1, 6);                                 \
    tmp0  = __lsx_vclip255_w(tmp0);                                 \
    tmp1  = __lsx_vclip255_w(tmp1);                                 \
    out_g = __lsx_vpackev_h(tmp1, tmp0);                            \
  }

// Pack and Store 8 ARGB values.
#define STOREARGB(in_a, in_r, in_g, in_b, pdst_argb)         \
  {                                                          \
    __m128i temp0, temp1;                                    \
    __m128i dst0, dst1;                                      \
                                                             \
    temp0 = __lsx_vpackev_b(in_g, in_b);                     \
    temp1 = __lsx_vpackev_b(in_a, in_r);                     \
    dst0  = __lsx_vilvl_h(temp1, temp0);                     \
    dst1  = __lsx_vilvh_h(temp1, temp0);                     \
    __lsx_vst(dst0, pdst_argb, 0);                           \
    __lsx_vst(dst1, pdst_argb, 16);                          \
    pdst_argb += 32;                                         \
  }

#define RGBTOUV(_tmpb, _tmpg, _tmpr, _nexb, _nexg, _nexr, _dst0)   \
  {                                                                \
    __m128i _tmp0, _tmp1, _tmp2, _tmp3;                            \
    __m128i _reg0, _reg1;                                          \
    _tmp0 = __lsx_vaddwev_h_bu(_tmpb, _nexb);                      \
    _tmp1 = __lsx_vaddwod_h_bu(_tmpb, _nexb);                      \
    _tmp2 = __lsx_vaddwev_h_bu(_tmpg, _nexg);                      \
    _tmp3 = __lsx_vaddwod_h_bu(_tmpg, _nexg);                      \
    _reg0 = __lsx_vaddwev_h_bu(_tmpr, _nexr);                      \
    _reg1 = __lsx_vaddwod_h_bu(_tmpr, _nexr);                      \
    _tmpb = __lsx_vavgr_hu(_tmp0, _tmp1);                          \
    _tmpg = __lsx_vavgr_hu(_tmp2, _tmp3);                          \
    _tmpr = __lsx_vavgr_hu(_reg0, _reg1);                          \
    _reg0 = __lsx_vmadd_h(const_8080, const_112, _tmpb);           \
    _reg1 = __lsx_vmadd_h(const_8080, const_112, _tmpr);           \
    _reg0 = __lsx_vmsub_h(_reg0, const_74, _tmpg);                 \
    _reg1 = __lsx_vmsub_h(_reg1, const_94, _tmpg);                 \
    _reg0 = __lsx_vmsub_h(_reg0, const_38, _tmpr);                 \
    _reg1 = __lsx_vmsub_h(_reg1, const_18, _tmpb);                 \
    _dst0 = __lsx_vsrlni_b_h(_reg1, _reg0, 8);                     \
  }

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
    RGBTOUV(tmpb, tmpg, tmpr, nexb, nexg, nexr, dst0);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_argb1555 += 32;
    next_argb1555 += 32;
  }
}

void RGB565ToYRow_LSX(const uint8_t* src_rgb565, uint8_t* dst_y, int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1;
  __m128i tmp0, tmp1, tmpb, tmpg, tmpr;
  __m128i reg0, reg1, dst0;
  __m128i const_66  = __lsx_vldi(66);
  __m128i const_129 = __lsx_vldi(129);
  __m128i const_25  = __lsx_vldi(25);
  __m128i const_1080 = {0x1080108010801080, 0x1080108010801080};
  __m128i shuff = {0x0B030A0209010800, 0x0F070E060D050C04};

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
    src_rgb565 += 32;
  }
}

void RGB565ToUVRow_LSX(const uint8_t* src_rgb565,
                       int src_stride_rgb565,
                       uint8_t* dst_u,
                       uint8_t* dst_v,
                       int width) {
  int x;
  int len = width >> 4;
  const uint8_t* next_rgb565 = src_rgb565 + src_stride_rgb565;
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
    DUP4_ARG2(__lsx_vld, src_rgb565, 0, src_rgb565, 16,
              next_rgb565, 0, next_rgb565, 16, src0, src1, src2, src3);
    DUP2_ARG2(__lsx_vpickev_b, src1, src0, src3, src2, tmp0, tmp2);
    DUP2_ARG2(__lsx_vpickod_b, src1, src0, src3, src2, tmp1, tmp3);
    tmpb = __lsx_vandi_b(tmp0, 0x1F);
    tmpr = __lsx_vandi_b(tmp1, 0xF8);
    nexb = __lsx_vandi_b(tmp2, 0x1F);
    nexr = __lsx_vandi_b(tmp3, 0xF8);
    reg1 = __lsx_vandi_b(tmp1, 0x07);
    reg3 = __lsx_vandi_b(tmp3, 0x07);
    reg0 = __lsx_vsrli_b(tmp0, 5);
    reg1 = __lsx_vslli_b(reg1, 3);
    reg2 = __lsx_vsrli_b(tmp2, 5);
    reg3 = __lsx_vslli_b(reg3, 3);
    tmpg = __lsx_vor_v(reg1, reg0);
    nexg = __lsx_vor_v(reg2, reg3);
    reg0 = __lsx_vslli_b(tmpb, 3);
    reg1 = __lsx_vsrli_b(tmpb, 2);
    reg2 = __lsx_vslli_b(nexb, 3);
    reg3 = __lsx_vsrli_b(nexb, 2);
    tmpb = __lsx_vor_v(reg1, reg0);
    nexb = __lsx_vor_v(reg2, reg3);
    reg0 = __lsx_vslli_b(tmpg, 2);
    reg1 = __lsx_vsrli_b(tmpg, 4);
    reg2 = __lsx_vslli_b(nexg, 2);
    reg3 = __lsx_vsrli_b(nexg, 4);
    tmpg = __lsx_vor_v(reg1, reg0);
    nexg = __lsx_vor_v(reg2, reg3);
    reg0 = __lsx_vsrli_b(tmpr, 5);
    reg2 = __lsx_vsrli_b(nexr, 5);
    tmpr = __lsx_vor_v(tmpr, reg0);
    nexr = __lsx_vor_v(nexr, reg2);
    RGBTOUV(tmpb, tmpg, tmpr, nexb, nexg, nexr, dst0);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_rgb565 += 32;
    next_rgb565 += 32;
  }
}

void RGB24ToYRow_LSX(const uint8_t* src_rgb24, uint8_t* dst_y, int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, src2;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i reg0, reg1, dst0;
  __m128i const_129 = __lsx_vldi(129);
  __m128i const_br = {0x4219421942194219, 0x4219421942194219};
  __m128i const_1080 = {0x1080108010801080, 0x1080108010801080};
  __m128i shuff0 = {0x0B09080605030200, 0x17151412110F0E0C};
  __m128i shuff1 = {0x0301001E1D1B1A18, 0x0F0D0C0A09070604};
  __m128i shuff2 = {0x000A000700040001, 0x001600130010000D};
  __m128i shuff3 = {0x0002001F001C0019, 0x000E000B00080005};

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_rgb24, 0);
    src1 = __lsx_vld(src_rgb24, 16);
    src2 = __lsx_vld(src_rgb24, 32);
    tmp0 = __lsx_vshuf_b(src1, src0, shuff0);
    tmp1 = __lsx_vshuf_b(src1, src2, shuff1);
    tmp2 = __lsx_vshuf_b(src1, src0, shuff2);
    tmp3 = __lsx_vshuf_b(src1, src2, shuff3);
    reg0 = __lsx_vmaddwev_h_bu(const_1080, tmp2, const_129);
    reg1 = __lsx_vmaddwev_h_bu(const_1080, tmp3, const_129);
    reg0 = __lsx_vdp2add_h_bu(reg0, const_br, tmp0);
    reg1 = __lsx_vdp2add_h_bu(reg1, const_br, tmp1);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    __lsx_vst(dst0, dst_y, 0);
    dst_y += 16;
    src_rgb24 += 48;
  }
}

void RGB24ToUVRow_LSX(const uint8_t* src_rgb24,
                      int src_stride_rgb24,
                      uint8_t* dst_u,
                      uint8_t* dst_v,
                      int width) {
  int x;
  const uint8_t* next_rgb24 = src_rgb24 + src_stride_rgb24;
  int len = width >> 4;
  __m128i src0, src1, src2;
  __m128i nex0, nex1, nex2, dst0;
  __m128i tmpb, tmpg, tmpr, nexb, nexg, nexr;
  __m128i const_112 = __lsx_vldi(0x438);
  __m128i const_74  = __lsx_vldi(0x425);
  __m128i const_38  = __lsx_vldi(0x413);
  __m128i const_94  = __lsx_vldi(0x42F);
  __m128i const_18  = __lsx_vldi(0x409);
  __m128i const_8080 = {0x8080808080808080, 0x8080808080808080};
  __m128i shuff0_b = {0x15120F0C09060300, 0x00000000001E1B18};
  __m128i shuff1_b = {0x0706050403020100, 0x1D1A1714110A0908};
  __m128i shuff0_g = {0x1613100D0A070401, 0x00000000001F1C19};
  __m128i shuff1_g = {0x0706050403020100, 0x1E1B1815120A0908};
  __m128i shuff0_r = {0x1714110E0B080502, 0x0000000000001D1A};
  __m128i shuff1_r = {0x0706050403020100, 0x1F1C191613100908};

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_rgb24, 0);
    src1 = __lsx_vld(src_rgb24, 16);
    src2 = __lsx_vld(src_rgb24, 32);
    nex0 = __lsx_vld(next_rgb24, 0);
    nex1 = __lsx_vld(next_rgb24, 16);
    nex2 = __lsx_vld(next_rgb24, 32);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, shuff0_b, nex1, nex0, shuff0_b, tmpb, nexb);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, shuff0_g, nex1, nex0, shuff0_g, tmpg, nexg);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, shuff0_r, nex1, nex0, shuff0_r, tmpr, nexr);
    DUP2_ARG3(__lsx_vshuf_b, src2, tmpb, shuff1_b, nex2, nexb, shuff1_b, tmpb, nexb);
    DUP2_ARG3(__lsx_vshuf_b, src2, tmpg, shuff1_g, nex2, nexg, shuff1_g, tmpg, nexg);
    DUP2_ARG3(__lsx_vshuf_b, src2, tmpr, shuff1_r, nex2, nexr, shuff1_r, tmpr, nexr);
    RGBTOUV(tmpb, tmpg, tmpr, nexb, nexg, nexr, dst0);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_rgb24 += 48;
    next_rgb24 += 48;
  }
}

void RAWToYRow_LSX(const uint8_t* src_raw, uint8_t* dst_y, int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, src2;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i reg0, reg1, dst0;
  __m128i const_129 = __lsx_vldi(129);
  __m128i const_br = {0x1942194219421942, 0x1942194219421942};
  __m128i const_1080 = {0x1080108010801080, 0x1080108010801080};
  __m128i shuff0 = {0x0B09080605030200, 0x17151412110F0E0C};
  __m128i shuff1 = {0x0301001E1D1B1A18, 0x0F0D0C0A09070604};
  __m128i shuff2 = {0x000A000700040001, 0x001600130010000D};
  __m128i shuff3 = {0x0002001F001C0019, 0x000E000B00080005};

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_raw, 0);
    src1 = __lsx_vld(src_raw, 16);
    src2 = __lsx_vld(src_raw, 32);
    tmp0 = __lsx_vshuf_b(src1, src0, shuff0);
    tmp1 = __lsx_vshuf_b(src1, src2, shuff1);
    tmp2 = __lsx_vshuf_b(src1, src0, shuff2);
    tmp3 = __lsx_vshuf_b(src1, src2, shuff3);
    reg0 = __lsx_vmaddwev_h_bu(const_1080, tmp2, const_129);
    reg1 = __lsx_vmaddwev_h_bu(const_1080, tmp3, const_129);
    reg0 = __lsx_vdp2add_h_bu(reg0, const_br, tmp0);
    reg1 = __lsx_vdp2add_h_bu(reg1, const_br, tmp1);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    __lsx_vst(dst0, dst_y, 0);
    dst_y += 16;
    src_raw += 48;
  }
}

void RAWToUVRow_LSX(const uint8_t* src_raw,
                    int src_stride_raw,
                    uint8_t* dst_u,
                    uint8_t* dst_v,
                    int width) {
  int x;
  const uint8_t* next_raw = src_raw + src_stride_raw;
  int len = width >> 4;
  __m128i src0, src1, src2;
  __m128i nex0, nex1, nex2, dst0;
  __m128i tmpb, tmpg, tmpr, nexb, nexg, nexr;
  __m128i const_112 = __lsx_vldi(0x438);
  __m128i const_74  = __lsx_vldi(0x425);
  __m128i const_38  = __lsx_vldi(0x413);
  __m128i const_94  = __lsx_vldi(0x42F);
  __m128i const_18  = __lsx_vldi(0x409);
  __m128i const_8080 = {0x8080808080808080, 0x8080808080808080};
  __m128i shuff0_r = {0x15120F0C09060300, 0x00000000001E1B18};
  __m128i shuff1_r = {0x0706050403020100, 0x1D1A1714110A0908};
  __m128i shuff0_g = {0x1613100D0A070401, 0x00000000001F1C19};
  __m128i shuff1_g = {0x0706050403020100, 0x1E1B1815120A0908};
  __m128i shuff0_b = {0x1714110E0B080502, 0x0000000000001D1A};
  __m128i shuff1_b = {0x0706050403020100, 0x1F1C191613100908};

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_raw, 0);
    src1 = __lsx_vld(src_raw, 16);
    src2 = __lsx_vld(src_raw, 32);
    nex0 = __lsx_vld(next_raw, 0);
    nex1 = __lsx_vld(next_raw, 16);
    nex2 = __lsx_vld(next_raw, 32);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, shuff0_b, nex1, nex0, shuff0_b, tmpb, nexb);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, shuff0_g, nex1, nex0, shuff0_g, tmpg, nexg);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, shuff0_r, nex1, nex0, shuff0_r, tmpr, nexr);
    DUP2_ARG3(__lsx_vshuf_b, src2, tmpb, shuff1_b, nex2, nexb, shuff1_b, tmpb, nexb);
    DUP2_ARG3(__lsx_vshuf_b, src2, tmpg, shuff1_g, nex2, nexg, shuff1_g, tmpg, nexg);
    DUP2_ARG3(__lsx_vshuf_b, src2, tmpr, shuff1_r, nex2, nexr, shuff1_r, tmpr, nexr);
    RGBTOUV(tmpb, tmpg, tmpr, nexb, nexg, nexr, dst0);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_raw += 48;
    next_raw += 48;
  }
}

void NV12ToARGBRow_LSX(const uint8_t* src_y,
                       const uint8_t* src_uv,
                       uint8_t* dst_argb,
                       const struct YuvConstants* yuvconstants,
                       int width) {
  int x;
  int len = width >> 3;
  __m128i vec_y, vec_vu;
  __m128i vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb;
  __m128i vec_vrub, vec_vgug;
  __m128i out_b, out_g, out_r;
  __m128i const_80 = __lsx_vldi(0x480);
  __m128i alpha = __lsx_vldi(0xFF);
  __m128i zero  = __lsx_vldi(0);

  YUVTORGB_SETUP(yuvconstants, vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb);
  vec_vrub = __lsx_vilvl_h(vec_vr, vec_ub);
  vec_vgug = __lsx_vilvl_h(vec_vg, vec_ug);

  for (x = 0; x < len; x++) {
    vec_y  = __lsx_vld(src_y, 0);
    vec_vu = __lsx_vld(src_uv, 0);
    YUVTORGB(vec_y, vec_vu, vec_vrub, vec_vgug, vec_yg, vec_yb,
             out_b, out_g, out_r);
    STOREARGB(alpha, out_r, out_g, out_b, dst_argb);
    src_y  += 8;
    src_uv += 8;
  }
}

void NV12ToRGB565Row_LSX(const uint8_t* src_y,
                         const uint8_t* src_uv,
                         uint8_t* dst_rgb565,
                         const struct YuvConstants* yuvconstants,
                         int width) {
  int x;
  int len = width >> 3;
  __m128i vec_y, vec_vu;
  __m128i vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb;
  __m128i vec_vrub, vec_vgug;
  __m128i out_b, out_g, out_r;
  __m128i const_80 = __lsx_vldi(0x480);
  __m128i zero  = __lsx_vldi(0);

  YUVTORGB_SETUP(yuvconstants, vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb);
  vec_vrub = __lsx_vilvl_h(vec_vr, vec_ub);
  vec_vgug = __lsx_vilvl_h(vec_vg, vec_ug);

  for (x = 0; x < len; x++) {
    vec_y  = __lsx_vld(src_y, 0);
    vec_vu = __lsx_vld(src_uv, 0);
    YUVTORGB(vec_y, vec_vu, vec_vrub, vec_vgug, vec_yg, vec_yb,
             out_b, out_g, out_r);
    out_b = __lsx_vsrli_h(out_b, 3);
    out_g = __lsx_vsrli_h(out_g, 2);
    out_r = __lsx_vsrli_h(out_r, 3);
    out_g = __lsx_vslli_h(out_g, 5);
    out_r = __lsx_vslli_h(out_r, 11);
    out_r = __lsx_vor_v(out_r, out_g);
    out_r = __lsx_vor_v(out_r, out_b);
    __lsx_vst(out_r, dst_rgb565, 0);
    src_y  += 8;
    src_uv += 8;
    dst_rgb565 += 16;
  }
}

void NV21ToARGBRow_LSX(const uint8_t* src_y,
                       const uint8_t* src_vu,
                       uint8_t* dst_argb,
                       const struct YuvConstants* yuvconstants,
                       int width) {
  int x;
  int len = width >> 3;
  __m128i vec_y, vec_uv;
  __m128i vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb;
  __m128i vec_ubvr, vec_ugvg;
  __m128i out_b, out_g, out_r;
  __m128i const_80 = __lsx_vldi(0x480);
  __m128i alpha = __lsx_vldi(0xFF);
  __m128i zero  = __lsx_vldi(0);

  YUVTORGB_SETUP(yuvconstants, vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb);
  vec_ubvr = __lsx_vilvl_h(vec_ub, vec_vr);
  vec_ugvg = __lsx_vilvl_h(vec_ug, vec_vg);

  for (x = 0; x < len; x++) {
    vec_y  = __lsx_vld(src_y, 0);
    vec_uv = __lsx_vld(src_vu, 0);
    YUVTORGB(vec_y, vec_uv, vec_ubvr, vec_ugvg, vec_yg, vec_yb,
             out_r, out_g, out_b);
    STOREARGB(alpha, out_r, out_g, out_b, dst_argb);
    src_y  += 8;
    src_vu += 8;
  }
}

void SobelRow_LSX(const uint8_t* src_sobelx,
                  const uint8_t* src_sobely,
                  uint8_t* dst_argb,
                  int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, tmp0;
  __m128i out0, out1, out2, out3;
  __m128i alpha  = __lsx_vldi(0xFF);
  __m128i shuff0 = {0x1001010110000000, 0x1003030310020202};
  __m128i shuff1 = __lsx_vaddi_bu(shuff0, 0x04);
  __m128i shuff2 = __lsx_vaddi_bu(shuff1, 0x04);
  __m128i shuff3 = __lsx_vaddi_bu(shuff2, 0x04);

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_sobelx, 0);
    src1 = __lsx_vld(src_sobely, 0);
    tmp0 = __lsx_vsadd_bu(src0, src1);
    DUP4_ARG3(__lsx_vshuf_b, alpha, tmp0, shuff0, alpha, tmp0, shuff1,
              alpha, tmp0, shuff2, alpha, tmp0, shuff3, out0, out1, out2, out3);
    __lsx_vst(out0, dst_argb, 0);
    __lsx_vst(out1, dst_argb, 16);
    __lsx_vst(out2, dst_argb, 32);
    __lsx_vst(out3, dst_argb, 48);
    src_sobelx += 16;
    src_sobely += 16;
    dst_argb += 64;
  }
}

void SobelToPlaneRow_LSX(const uint8_t* src_sobelx,
                         const uint8_t* src_sobely,
                         uint8_t* dst_y,
                         int width) {
  int x;
  int len = width >> 5;
  __m128i src0, src1, src2, src3, dst0, dst1;

  for (x = 0; x < len; x++) {
    DUP2_ARG2(__lsx_vld, src_sobelx, 0, src_sobelx, 16, src0, src1);
    DUP2_ARG2(__lsx_vld, src_sobely, 0, src_sobely, 16, src2, src3);
    dst0 = __lsx_vsadd_bu(src0, src2);
    dst1 = __lsx_vsadd_bu(src1, src3);
    __lsx_vst(dst0, dst_y, 0);
    __lsx_vst(dst1, dst_y, 16);
    src_sobelx += 32;
    src_sobely += 32;
    dst_y += 32;
  }
}

void SobelXYRow_LSX(const uint8_t* src_sobelx,
                    const uint8_t* src_sobely,
                    uint8_t* dst_argb,
                    int width) {
  int x;
  int len = width >> 4;
  __m128i src_r, src_b, src_g;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i dst0, dst1, dst2, dst3;
  __m128i alpha = __lsx_vldi(0xFF);

  for (x = 0; x < len; x++) {
    src_r = __lsx_vld(src_sobelx, 0);
    src_b = __lsx_vld(src_sobely, 0);
    src_g = __lsx_vsadd_bu(src_r, src_b);
    tmp0 = __lsx_vilvl_b(src_g, src_b);
    tmp1 = __lsx_vilvh_b(src_g, src_b);
    tmp2 = __lsx_vilvl_b(alpha, src_r);
    tmp3 = __lsx_vilvh_b(alpha, src_r);
    dst0 = __lsx_vilvl_h(tmp2, tmp0);
    dst1 = __lsx_vilvh_h(tmp2, tmp0);
    dst2 = __lsx_vilvl_h(tmp3, tmp1);
    dst3 = __lsx_vilvh_h(tmp3, tmp1);
    __lsx_vst(dst0, dst_argb, 0);
    __lsx_vst(dst1, dst_argb, 16);
    __lsx_vst(dst2, dst_argb, 32);
    __lsx_vst(dst3, dst_argb, 48);
    src_sobelx += 16;
    src_sobely += 16;
    dst_argb += 64;
  }
}

void ARGBToYJRow_LSX(const uint8_t* src_argb, uint8_t* dst_y, int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, src2, src3, dst0;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i reg0, reg1;
  __m128i const_128 = __lsx_vldi(0x480);
  __m128i const_150 = __lsx_vldi(0x96);
  __m128i const_br  = {0x4D1D4D1D4D1D4D1D, 0x4D1D4D1D4D1D4D1D};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_argb, 0, src_argb, 16, src_argb, 32,
              src_argb, 48, src0, src1, src2, src3);
    tmp0 = __lsx_vpickev_b(src1, src0);
    tmp1 = __lsx_vpickod_b(src1, src0);
    tmp2 = __lsx_vpickev_b(src3, src2);
    tmp3 = __lsx_vpickod_b(src3, src2);
    reg0 = __lsx_vmaddwev_h_bu(const_128, tmp1, const_150);
    reg1 = __lsx_vmaddwev_h_bu(const_128, tmp3, const_150);
    reg0 = __lsx_vdp2add_h_bu(reg0, const_br, tmp0);
    reg1 = __lsx_vdp2add_h_bu(reg1, const_br, tmp2);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    __lsx_vst(dst0, dst_y, 0);
    dst_y += 16;
    src_argb += 64;
  }
}

void BGRAToYRow_LSX(const uint8_t* src_bgra, uint8_t* dst_y, int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, src2, src3, dst0;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i reg0, reg1;
  __m128i const_129 = __lsx_vldi(0x81);
  __m128i const_br  = {0x1942194219421942, 0x1942194219421942};
  __m128i const_1080 = {0x1080108010801080, 0x1080108010801080};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_bgra, 0, src_bgra, 16, src_bgra, 32,
              src_bgra, 48, src0, src1, src2, src3);
    tmp0 = __lsx_vpickod_b(src1, src0);
    tmp1 = __lsx_vpickev_b(src1, src0);
    tmp2 = __lsx_vpickod_b(src3, src2);
    tmp3 = __lsx_vpickev_b(src3, src2);
    reg0 = __lsx_vmaddwod_h_bu(const_1080, tmp1, const_129);
    reg1 = __lsx_vmaddwod_h_bu(const_1080, tmp3, const_129);
    reg0 = __lsx_vdp2add_h_bu(reg0, const_br, tmp0);
    reg1 = __lsx_vdp2add_h_bu(reg1, const_br, tmp2);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    __lsx_vst(dst0, dst_y, 0);
    dst_y += 16;
    src_bgra += 64;
  }
}

void BGRAToUVRow_LSX(const uint8_t* src_bgra,
                     int src_stride_bgra,
                     uint8_t* dst_u,
                     uint8_t* dst_v,
                     int width) {
  int x;
  const uint8_t* next_bgra = src_bgra + src_stride_bgra;
  int len = width >> 4;
  __m128i src0, src1, src2, src3;
  __m128i nex0, nex1, nex2, nex3;
  __m128i tmp0, tmp1, tmp2, tmp3, dst0;
  __m128i tmpb, tmpg, tmpr, nexb, nexg, nexr;
  __m128i const_112 = __lsx_vldi(0x438);
  __m128i const_74  = __lsx_vldi(0x425);
  __m128i const_38  = __lsx_vldi(0x413);
  __m128i const_94  = __lsx_vldi(0x42F);
  __m128i const_18  = __lsx_vldi(0x409);
  __m128i const_8080 = {0x8080808080808080, 0x8080808080808080};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_bgra, 0, src_bgra, 16, src_bgra, 32,
              src_bgra, 48, src0, src1, src2, src3);
    DUP4_ARG2(__lsx_vld, next_bgra, 0, next_bgra, 16, next_bgra, 32,
              next_bgra, 48, nex0, nex1, nex2, nex3);
    tmp0 = __lsx_vpickod_b(src1, src0);
    tmp1 = __lsx_vpickev_b(src1, src0);
    tmp2 = __lsx_vpickod_b(src3, src2);
    tmp3 = __lsx_vpickev_b(src3, src2);
    tmpb = __lsx_vpickod_b(tmp2, tmp0);
    tmpr = __lsx_vpickev_b(tmp2, tmp0);
    tmpg = __lsx_vpickod_b(tmp3, tmp1);
    tmp0 = __lsx_vpickod_b(nex1, nex0);
    tmp1 = __lsx_vpickev_b(nex1, nex0);
    tmp2 = __lsx_vpickod_b(nex3, nex2);
    tmp3 = __lsx_vpickev_b(nex3, nex2);
    nexb = __lsx_vpickod_b(tmp2, tmp0);
    nexr = __lsx_vpickev_b(tmp2, tmp0);
    nexg = __lsx_vpickod_b(tmp3, tmp1);
    RGBTOUV(tmpb, tmpg, tmpr, nexb, nexg, nexr, dst0);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_bgra += 64;
    next_bgra += 64;
  }
}

void ABGRToYRow_LSX(const uint8_t* src_abgr, uint8_t* dst_y, int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, src2, src3, dst0;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i reg0, reg1;
  __m128i const_129 = __lsx_vldi(0x81);
  __m128i const_br  = {0x1942194219421942, 0x1942194219421942};
  __m128i const_1080 = {0x1080108010801080, 0x1080108010801080};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_abgr, 0, src_abgr, 16, src_abgr, 32,
              src_abgr, 48, src0, src1, src2, src3);
    tmp0 = __lsx_vpickev_b(src1, src0);
    tmp1 = __lsx_vpickod_b(src1, src0);
    tmp2 = __lsx_vpickev_b(src3, src2);
    tmp3 = __lsx_vpickod_b(src3, src2);
    reg0 = __lsx_vmaddwev_h_bu(const_1080, tmp1, const_129);
    reg1 = __lsx_vmaddwev_h_bu(const_1080, tmp3, const_129);
    reg0 = __lsx_vdp2add_h_bu(reg0, const_br, tmp0);
    reg1 = __lsx_vdp2add_h_bu(reg1, const_br, tmp2);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    __lsx_vst(dst0, dst_y, 0);
    dst_y += 16;
    src_abgr += 64;
  }
}

void ABGRToUVRow_LSX(const uint8_t* src_abgr,
                     int src_stride_abgr,
                     uint8_t* dst_u,
                     uint8_t* dst_v,
                     int width) {
  int x;
  const uint8_t* next_abgr = src_abgr + src_stride_abgr;
  int len = width >> 4;
  __m128i src0, src1, src2, src3;
  __m128i nex0, nex1, nex2, nex3;
  __m128i tmp0, tmp1, tmp2, tmp3, dst0;
  __m128i tmpb, tmpg, tmpr, nexb, nexg, nexr;
  __m128i const_112 = __lsx_vldi(0x438);
  __m128i const_74  = __lsx_vldi(0x425);
  __m128i const_38  = __lsx_vldi(0x413);
  __m128i const_94  = __lsx_vldi(0x42F);
  __m128i const_18  = __lsx_vldi(0x409);
  __m128i const_8080 = {0x8080808080808080, 0x8080808080808080};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_abgr, 0, src_abgr, 16, src_abgr, 32,
              src_abgr, 48, src0, src1, src2, src3);
    DUP4_ARG2(__lsx_vld, next_abgr, 0, next_abgr, 16, next_abgr, 32,
              next_abgr, 48, nex0, nex1, nex2, nex3);
    tmp0 = __lsx_vpickev_b(src1, src0);
    tmp1 = __lsx_vpickod_b(src1, src0);
    tmp2 = __lsx_vpickev_b(src3, src2);
    tmp3 = __lsx_vpickod_b(src3, src2);
    tmpb = __lsx_vpickod_b(tmp2, tmp0);
    tmpr = __lsx_vpickev_b(tmp2, tmp0);
    tmpg = __lsx_vpickev_b(tmp3, tmp1);
    tmp0 = __lsx_vpickev_b(nex1, nex0);
    tmp1 = __lsx_vpickod_b(nex1, nex0);
    tmp2 = __lsx_vpickev_b(nex3, nex2);
    tmp3 = __lsx_vpickod_b(nex3, nex2);
    nexb = __lsx_vpickod_b(tmp2, tmp0);
    nexr = __lsx_vpickev_b(tmp2, tmp0);
    nexg = __lsx_vpickev_b(tmp3, tmp1);
    RGBTOUV(tmpb, tmpg, tmpr, nexb, nexg, nexr, dst0);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_abgr += 64;
    next_abgr += 64;
  }
}

void RGBAToYRow_LSX(const uint8_t* src_rgba, uint8_t* dst_y, int width) {
  int x;
  int len = width >> 4;
  __m128i src0, src1, src2, src3, dst0;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i reg0, reg1;
  __m128i const_129 = __lsx_vldi(0x81);
  __m128i const_br  = {0x4219421942194219, 0x4219421942194219};
  __m128i const_1080 = {0x1080108010801080, 0x1080108010801080};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_rgba, 0, src_rgba, 16, src_rgba, 32,
              src_rgba, 48, src0, src1, src2, src3);
    tmp0 = __lsx_vpickod_b(src1, src0);
    tmp1 = __lsx_vpickev_b(src1, src0);
    tmp2 = __lsx_vpickod_b(src3, src2);
    tmp3 = __lsx_vpickev_b(src3, src2);
    reg0 = __lsx_vmaddwod_h_bu(const_1080, tmp1, const_129);
    reg1 = __lsx_vmaddwod_h_bu(const_1080, tmp3, const_129);
    reg0 = __lsx_vdp2add_h_bu(reg0, const_br, tmp0);
    reg1 = __lsx_vdp2add_h_bu(reg1, const_br, tmp2);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    __lsx_vst(dst0, dst_y, 0);
    dst_y += 16;
    src_rgba += 64;
  }
}

void RGBAToUVRow_LSX(const uint8_t* src_rgba,
                     int src_stride_rgba,
                     uint8_t* dst_u,
                     uint8_t* dst_v,
                     int width) {
  int x;
  const uint8_t* next_rgba = src_rgba + src_stride_rgba;
  int len = width >> 4;
  __m128i src0, src1, src2, src3;
  __m128i nex0, nex1, nex2, nex3;
  __m128i tmp0, tmp1, tmp2, tmp3, dst0;
  __m128i tmpb, tmpg, tmpr, nexb, nexg, nexr;
  __m128i const_112 = __lsx_vldi(0x438);
  __m128i const_74  = __lsx_vldi(0x425);
  __m128i const_38  = __lsx_vldi(0x413);
  __m128i const_94  = __lsx_vldi(0x42F);
  __m128i const_18  = __lsx_vldi(0x409);
  __m128i const_8080 = {0x8080808080808080, 0x8080808080808080};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_rgba, 0, src_rgba, 16, src_rgba, 32,
              src_rgba, 48, src0, src1, src2, src3);
    DUP4_ARG2(__lsx_vld, next_rgba, 0, next_rgba, 16, next_rgba, 32,
              next_rgba, 48, nex0, nex1, nex2, nex3);
    tmp0 = __lsx_vpickod_b(src1, src0);
    tmp1 = __lsx_vpickev_b(src1, src0);
    tmp2 = __lsx_vpickod_b(src3, src2);
    tmp3 = __lsx_vpickev_b(src3, src2);
    tmpr = __lsx_vpickod_b(tmp2, tmp0);
    tmpb = __lsx_vpickev_b(tmp2, tmp0);
    tmpg = __lsx_vpickod_b(tmp3, tmp1);
    tmp0 = __lsx_vpickod_b(nex1, nex0);
    tmp1 = __lsx_vpickev_b(nex1, nex0);
    tmp2 = __lsx_vpickod_b(nex3, nex2);
    tmp3 = __lsx_vpickev_b(nex3, nex2);
    nexr = __lsx_vpickod_b(tmp2, tmp0);
    nexb = __lsx_vpickev_b(tmp2, tmp0);
    nexg = __lsx_vpickod_b(tmp3, tmp1);
    RGBTOUV(tmpb, tmpg, tmpr, nexb, nexg, nexr, dst0);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_rgba += 64;
    next_rgba += 64;
  }
}

void ARGBToUVJRow_LSX(const uint8_t* src_argb,
                      int src_stride_argb,
                      uint8_t* dst_u,
                      uint8_t* dst_v,
                      int width) {
  int x;
  const uint8_t* next_argb = src_argb + src_stride_argb;
  int len = width >> 4;
  __m128i src0, src1, src2, src3;
  __m128i nex0, nex1, nex2, nex3;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i reg0, reg1,  dst0;
  __m128i tmpb, tmpg, tmpr, nexb, nexg, nexr;
  __m128i const_63  = __lsx_vldi(0x43F);
  __m128i const_42  = __lsx_vldi(0x42A);
  __m128i const_21  = __lsx_vldi(0x415);
  __m128i const_53  = __lsx_vldi(0x435);
  __m128i const_10  = __lsx_vldi(0x40A);
  __m128i const_8080 = {0x8080808080808080, 0x8080808080808080};

  for (x = 0; x < len; x++) {
    DUP4_ARG2(__lsx_vld, src_argb, 0, src_argb, 16, src_argb, 32,
              src_argb, 48, src0, src1, src2, src3);
    DUP4_ARG2(__lsx_vld, next_argb, 0, next_argb, 16, next_argb, 32,
              next_argb, 48, nex0, nex1, nex2, nex3);
    tmp0 = __lsx_vpickev_b(src1, src0);
    tmp1 = __lsx_vpickod_b(src1, src0);
    tmp2 = __lsx_vpickev_b(src3, src2);
    tmp3 = __lsx_vpickod_b(src3, src2);
    tmpr = __lsx_vpickod_b(tmp2, tmp0);
    tmpb = __lsx_vpickev_b(tmp2, tmp0);
    tmpg = __lsx_vpickev_b(tmp3, tmp1);
    tmp0 = __lsx_vpickev_b(nex1, nex0);
    tmp1 = __lsx_vpickod_b(nex1, nex0);
    tmp2 = __lsx_vpickev_b(nex3, nex2);
    tmp3 = __lsx_vpickod_b(nex3, nex2);
    nexr = __lsx_vpickod_b(tmp2, tmp0);
    nexb = __lsx_vpickev_b(tmp2, tmp0);
    nexg = __lsx_vpickev_b(tmp3, tmp1);
    tmp0 = __lsx_vaddwev_h_bu(tmpb, nexb);
    tmp1 = __lsx_vaddwod_h_bu(tmpb, nexb);
    tmp2 = __lsx_vaddwev_h_bu(tmpg, nexg);
    tmp3 = __lsx_vaddwod_h_bu(tmpg, nexg);
    reg0 = __lsx_vaddwev_h_bu(tmpr, nexr);
    reg1 = __lsx_vaddwod_h_bu(tmpr, nexr);
    tmpb = __lsx_vavgr_hu(tmp0, tmp1);
    tmpg = __lsx_vavgr_hu(tmp2, tmp3);
    tmpr = __lsx_vavgr_hu(reg0, reg1);
    reg0 = __lsx_vmadd_h(const_8080, const_63, tmpb);
    reg1 = __lsx_vmadd_h(const_8080, const_63, tmpr);
    reg0 = __lsx_vmsub_h(reg0, const_42, tmpg);
    reg1 = __lsx_vmsub_h(reg1, const_53, tmpg);
    reg0 = __lsx_vmsub_h(reg0, const_21, tmpr);
    reg1 = __lsx_vmsub_h(reg1, const_10, tmpb);
    dst0 = __lsx_vsrlni_b_h(reg1, reg0, 8);
    __lsx_vstelm_d(dst0, dst_u, 0, 0);
    __lsx_vstelm_d(dst0, dst_v, 0, 1);
    dst_u += 8;
    dst_v += 8;
    src_argb += 64;
    next_argb += 64;
  }
}

void I444ToARGBRow_LSX(const uint8_t* src_y,
                       const uint8_t* src_u,
                       const uint8_t* src_v,
                       uint8_t* dst_argb,
                       const struct YuvConstants* yuvconstants,
                       int width) {
  int x;
  int len = width >> 4;
  __m128i vec_y, vec_u, vec_v, out_b, out_g, out_r;
  __m128i vec_yl, vec_yh, vec_ul, vec_vl, vec_uh, vec_vh;
  __m128i vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb, vec_ugvg;
  __m128i const_80 = __lsx_vldi(0x480);
  __m128i alpha = __lsx_vldi(0xFF);
  __m128i zero  = __lsx_vldi(0);

  YUVTORGB_SETUP(yuvconstants, vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb);
  vec_ugvg = __lsx_vilvl_h(vec_ug, vec_vg);

  for (x = 0; x < len; x++) {
    vec_y  = __lsx_vld(src_y, 0);
    vec_u  = __lsx_vld(src_u, 0);
    vec_v  = __lsx_vld(src_v, 0);
    vec_yl = __lsx_vilvl_b(vec_y, vec_y);
    vec_ul = __lsx_vilvl_b(zero, vec_u);
    vec_vl = __lsx_vilvl_b(zero, vec_v);
    I444TORGB(vec_yl, vec_ul, vec_vl, vec_ub, vec_vr, vec_ugvg,
              vec_yg, vec_yb, out_b, out_g, out_r);
    STOREARGB(alpha, out_r, out_g, out_b, dst_argb);
    vec_yh = __lsx_vilvh_b(vec_y, vec_y);
    vec_uh = __lsx_vilvh_b(zero, vec_u);
    vec_vh = __lsx_vilvh_b(zero, vec_v);
    I444TORGB(vec_yh, vec_uh, vec_vh, vec_ub, vec_vr, vec_ugvg,
              vec_yg, vec_yb, out_b, out_g, out_r);
    STOREARGB(alpha, out_r, out_g, out_b, dst_argb);
    src_y += 16;
    src_u += 16;
    src_v += 16;
  }
}

void I400ToARGBRow_LSX(const uint8_t* src_y,
                       uint8_t* dst_argb,
                       const struct YuvConstants* yuvconstants,
                       int width) {
  int x;
  int len = width >> 4;
  __m128i vec_y, vec_yl, vec_yh, out0;
  __m128i y_ev, y_od, dst0, dst1, dst2, dst3;
  __m128i temp0, temp1;
  __m128i alpha = __lsx_vldi(0xFF);
  __m128i vec_yg = __lsx_vreplgr2vr_h(yuvconstants->kYToRgb[0]);
  __m128i vec_yb = __lsx_vreplgr2vr_w(yuvconstants->kYBiasToRgb[0]);

  for (x = 0; x < len; x++) {
    vec_y = __lsx_vld(src_y, 0);
    vec_yl = __lsx_vilvl_b(vec_y, vec_y);
    y_ev  = __lsx_vmulwev_w_hu_h(vec_yl, vec_yg);
    y_od  = __lsx_vmulwod_w_hu_h(vec_yl, vec_yg);
    y_ev  = __lsx_vsrai_w(y_ev, 16);
    y_od  = __lsx_vsrai_w(y_od, 16);
    y_ev  = __lsx_vadd_w(y_ev, vec_yb);
    y_od  = __lsx_vadd_w(y_od, vec_yb);
    y_ev  = __lsx_vsrai_w(y_ev, 6);
    y_od  = __lsx_vsrai_w(y_od, 6);
    y_ev  = __lsx_vclip255_w(y_ev);
    y_od  = __lsx_vclip255_w(y_od);
    out0  = __lsx_vpackev_h(y_od, y_ev);
    temp0 = __lsx_vpackev_b(out0, out0);
    temp1 = __lsx_vpackev_b(alpha, out0);
    dst0  = __lsx_vilvl_h(temp1, temp0);
    dst1  = __lsx_vilvh_h(temp1, temp0);
    vec_yh = __lsx_vilvh_b(vec_y, vec_y);
    y_ev  = __lsx_vmulwev_w_hu_h(vec_yh, vec_yg);
    y_od  = __lsx_vmulwod_w_hu_h(vec_yh, vec_yg);
    y_ev  = __lsx_vsrai_w(y_ev, 16);
    y_od  = __lsx_vsrai_w(y_od, 16);
    y_ev  = __lsx_vadd_w(y_ev, vec_yb);
    y_od  = __lsx_vadd_w(y_od, vec_yb);
    y_ev  = __lsx_vsrai_w(y_ev, 6);
    y_od  = __lsx_vsrai_w(y_od, 6);
    y_ev  = __lsx_vclip255_w(y_ev);
    y_od  = __lsx_vclip255_w(y_od);
    out0  = __lsx_vpackev_h(y_od, y_ev);
    temp0 = __lsx_vpackev_b(out0, out0);
    temp1 = __lsx_vpackev_b(alpha, out0);
    dst2  = __lsx_vilvl_h(temp1, temp0);
    dst3  = __lsx_vilvh_h(temp1, temp0);
    __lsx_vst(dst0, dst_argb, 0);
    __lsx_vst(dst1, dst_argb, 16);
    __lsx_vst(dst2, dst_argb, 32);
    __lsx_vst(dst3, dst_argb, 48);
    dst_argb += 64;
    src_y += 16;
  }
}

void J400ToARGBRow_LSX(const uint8_t* src_y, uint8_t* dst_argb, int width) {
  int x;
  int len = width >> 4;
  __m128i vec_y, dst0, dst1, dst2, dst3;
  __m128i tmp0, tmp1, tmp2, tmp3;
  __m128i alpha = __lsx_vldi(0xFF);

  for (x = 0; x < len; x++) {
    vec_y = __lsx_vld(src_y, 0);
    tmp0 = __lsx_vilvl_b(vec_y, vec_y);
    tmp1 = __lsx_vilvh_b(vec_y, vec_y);
    tmp2 = __lsx_vilvl_b(alpha, vec_y);
    tmp3 = __lsx_vilvh_b(alpha, vec_y);
    dst0 = __lsx_vilvl_h(tmp2, tmp0);
    dst1 = __lsx_vilvh_h(tmp2, tmp0);
    dst2 = __lsx_vilvl_h(tmp3, tmp1);
    dst3 = __lsx_vilvh_h(tmp3, tmp1);
    __lsx_vst(dst0, dst_argb, 0);
    __lsx_vst(dst1, dst_argb, 16);
    __lsx_vst(dst2, dst_argb, 32);
    __lsx_vst(dst3, dst_argb, 48);
    dst_argb += 64;
    src_y += 16;
  }
}

void YUY2ToARGBRow_LSX(const uint8_t* src_yuy2,
                       uint8_t* dst_argb,
                       const struct YuvConstants* yuvconstants,
                       int width) {
  int x;
  int len = width >> 3;
  __m128i src0, vec_y, vec_vu;
  __m128i vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb;
  __m128i vec_vrub, vec_vgug;
  __m128i out_b, out_g, out_r;
  __m128i const_80 = __lsx_vldi(0x480);
  __m128i zero  = __lsx_vldi(0);
  __m128i alpha = __lsx_vldi(0xFF);

  YUVTORGB_SETUP(yuvconstants, vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb);
  vec_vrub = __lsx_vilvl_h(vec_vr, vec_ub);
  vec_vgug = __lsx_vilvl_h(vec_vg, vec_ug);

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_yuy2, 0);
    vec_y  = __lsx_vpickev_b(src0, src0);
    vec_vu = __lsx_vpickod_b(src0, src0);
    YUVTORGB(vec_y, vec_vu, vec_vrub, vec_vgug, vec_yg, vec_yb,
             out_b, out_g, out_r);
    STOREARGB(alpha, out_r, out_g, out_b, dst_argb);
    src_yuy2 += 16;
  }
}

void UYVYToARGBRow_LSX(const uint8_t* src_uyvy,
                       uint8_t* dst_argb,
                       const struct YuvConstants* yuvconstants,
                       int width) {
  int x;
  int len = width >> 3;
  __m128i src0, vec_y, vec_vu;
  __m128i vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb;
  __m128i vec_vrub, vec_vgug;
  __m128i out_b, out_g, out_r;
  __m128i const_80 = __lsx_vldi(0x480);
  __m128i zero  = __lsx_vldi(0);
  __m128i alpha = __lsx_vldi(0xFF);

  YUVTORGB_SETUP(yuvconstants, vec_vr, vec_ub, vec_vg, vec_ug, vec_yg, vec_yb);
  vec_vrub = __lsx_vilvl_h(vec_vr, vec_ub);
  vec_vgug = __lsx_vilvl_h(vec_vg, vec_ug);

  for (x = 0; x < len; x++) {
    src0 = __lsx_vld(src_uyvy, 0);
    vec_y  = __lsx_vpickod_b(src0, src0);
    vec_vu = __lsx_vpickev_b(src0, src0);
    YUVTORGB(vec_y, vec_vu, vec_vrub, vec_vgug, vec_yg, vec_yb,
             out_b, out_g, out_r);
    STOREARGB(alpha, out_r, out_g, out_b, dst_argb);
    src_uyvy += 16;
  }
}

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // !defined(LIBYUV_DISABLE_LSX) && defined(__loongarch_sx)
