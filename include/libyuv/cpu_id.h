/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef INCLUDE_LIBYUV_CPU_ID_H_  // NOLINT
#define INCLUDE_LIBYUV_CPU_ID_H_

#include "libyuv/basic_types.h"

// Define LIBYUV_HAVE_STDATOMIC_H if the compiler supports C11 atomics.

// GCC 4.9 has stdatomic.h. (See https://gcc.gnu.org/wiki/C11Status.)
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
#define LIBYUV_HAVE_STDATOMIC_H 1
#endif

#if defined(__clang__) && !defined(_MSC_VER)
#if __has_include(<stdatomic.h>)
//#define LIBYUV_HAVE_STDATOMIC_H 1
#endif
#endif  // __clang__

#ifdef LIBYUV_HAVE_STDATOMIC_H
#error oops
#include <stdatomic.h>
#endif

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

// Internal flag to indicate cpuid requires initialization.
static const int kCpuInitialized = 0x1;

// These flags are only valid on ARM processors.
static const int kCpuHasARM = 0x2;
static const int kCpuHasNEON = 0x4;
// 0x8 reserved for future ARM flag.

// These flags are only valid on x86 processors.
static const int kCpuHasX86 = 0x10;
static const int kCpuHasSSE2 = 0x20;
static const int kCpuHasSSSE3 = 0x40;
static const int kCpuHasSSE41 = 0x80;
static const int kCpuHasSSE42 = 0x100;
static const int kCpuHasAVX = 0x200;
static const int kCpuHasAVX2 = 0x400;
static const int kCpuHasERMS = 0x800;
static const int kCpuHasFMA3 = 0x1000;
static const int kCpuHasAVX3 = 0x2000;
// 0x2000, 0x4000, 0x8000 reserved for future X86 flags.

// These flags are only valid on MIPS processors.
static const int kCpuHasMIPS = 0x10000;
static const int kCpuHasDSPR2 = 0x20000;

// Internal function used to auto-init.
LIBYUV_API
int InitCpuFlags(void);

// Internal function for parsing /proc/cpuinfo.
LIBYUV_API
int ArmCpuCaps(const char* cpuinfo_name);

// Detect CPU has SSE2 etc.
// Test_flag parameter should be one of kCpuHas constants above.
// returns non-zero if instruction set is detected
static __inline int TestCpuFlag(int test_flag) {
#ifdef LIBYUV_HAVE_STDATOMIC_H
  LIBYUV_API extern atomic_int cpu_info_;
  int cpu_info = atomic_load_explicit(&cpu_info_, memory_order_relaxed);
#else
  LIBYUV_API extern int cpu_info_;
  int cpu_info = cpu_info_;
#endif  // LIBYUV_HAVE_STDATOMIC_H
  return (!cpu_info ? InitCpuFlags() : cpu_info) & test_flag;
}

// For testing, allow CPU flags to be disabled.
// ie MaskCpuFlags(~kCpuHasSSSE3) to disable SSSE3.
// MaskCpuFlags(-1) to enable all cpu specific optimizations.
// MaskCpuFlags(1) to disable all cpu specific optimizations.
LIBYUV_API
void MaskCpuFlags(int enable_flags);

// Low level cpuid for X86. Returns zeros on other CPUs.
// eax is the info type that you want.
// ecx is typically the cpu number, and should normally be zero.
LIBYUV_API
void CpuId(uint32 eax, uint32 ecx, uint32* cpu_info);

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // INCLUDE_LIBYUV_CPU_ID_H_  NOLINT
