//============================================================================//
//                                                                            //
// ozz-animation, 3d skeletal animation libraries and tools.                  //
// https://code.google.com/p/ozz-animation/                                   //
//                                                                            //
//----------------------------------------------------------------------------//
//                                                                            //
// Copyright (c) 2012-2015 Guillaume Blanc                                    //
//                                                                            //
// This software is provided 'as-is', without any express or implied          //
// warranty. In no event will the authors be held liable for any damages      //
// arising from the use of this software.                                     //
//                                                                            //
// Permission is granted to anyone to use this software for any purpose,      //
// including commercial applications, and to alter it and redistribute it     //
// freely, subject to the following restrictions:                             //
//                                                                            //
// 1. The origin of this software must not be misrepresented; you must not    //
// claim that you wrote the original software. If you use this software       //
// in a product, an acknowledgment in the product documentation would be      //
// appreciated but is not required.                                           //
//                                                                            //
// 2. Altered source versions must be plainly marked as such, and must not be //
// misrepresented as being the original software.                             //
//                                                                            //
// 3. This notice may not be removed or altered from any source               //
// distribution.                                                              //
//                                                                            //
//============================================================================//

#include "ozz/base/maths/simd_math.h"

#include <stdint.h>

#include "gtest/gtest.h"

#include "ozz/base/gtest_helper.h"
#include "ozz/base/maths/math_constant.h"
#include "ozz/base/maths/math_ex.h"
#include "ozz/base/maths/gtest_math_helper.h"

using ozz::math::SimdInt4;

OZZ_STATIC_ASSERT(sizeof(SimdInt4) == 4 * sizeof(int32_t));

TEST(LoadInt, ozz_simd_math) {
  const SimdInt4 iX = ozz::math::simd_int4::LoadX(15);
  EXPECT_SIMDINT_EQ(iX, 15, 0, 0, 0);

  const SimdInt4 i1 = ozz::math::simd_int4::Load1(15);
  EXPECT_SIMDINT_EQ(i1, 15, 15, 15, 15);

  const SimdInt4 i4 = ozz::math::simd_int4::Load(1, -1, 2, -3);
  EXPECT_SIMDINT_EQ(i4, 1, -1, 2, -3);

  const SimdInt4 itX = ozz::math::simd_int4::LoadX(true);
  EXPECT_SIMDINT_EQ(itX, 0xffffffff, 0, 0, 0);

  const SimdInt4 ifX = ozz::math::simd_int4::LoadX(false);
  EXPECT_SIMDINT_EQ(ifX, 0, 0, 0, 0);

  const SimdInt4 it1 = ozz::math::simd_int4::Load1(true);
  EXPECT_SIMDINT_EQ(it1, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

  const SimdInt4 if1 = ozz::math::simd_int4::Load1(false);
  EXPECT_SIMDINT_EQ(if1, 0, 0, 0, 0);

  const SimdInt4 ibttff = ozz::math::simd_int4::Load(true, true, false, false);
  EXPECT_SIMDINT_EQ(ibttff, 0xffffffff, 0xffffffff, 0, 0);

  const SimdInt4 ibftft = ozz::math::simd_int4::Load(false, true, false, true);
  EXPECT_SIMDINT_EQ(ibftft, 0, 0xffffffff, 0, 0xffffffff);
}

TEST(LoadIntPtr, ozz_simd_math) {
  union Data {
    int i[4 + 5];
    char c[(4 + 5) * sizeof(int)];  // The 2nd char isn't aligned to an integer.
  };
  const Data d_in = {{-1, 1, 2, 3, 4, 5, 6, 7, 8}};
  const int aligned_offset =
    (16 - (reinterpret_cast<intptr_t>(d_in.i) & 0xf)) / sizeof(float);
  assert(aligned_offset > 0 && aligned_offset <= 4);

  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::LoadPtrU(d_in.i + aligned_offset + 1), d_in.i[aligned_offset + 1], d_in.i[aligned_offset + 2], d_in.i[aligned_offset + 3], d_in.i[aligned_offset + 4]);
  EXPECT_ASSERTION(ozz::math::simd_int4::LoadPtrU(reinterpret_cast<const int*>(d_in.c + 1)), "alignment");
  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::LoadPtr(d_in.i + aligned_offset), d_in.i[aligned_offset], d_in.i[aligned_offset + 1], d_in.i[aligned_offset + 2], d_in.i[aligned_offset + 3]);
  EXPECT_ASSERTION(ozz::math::simd_int4::LoadPtr(d_in.i + aligned_offset + 1), "alignment");

  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::LoadXPtrU(d_in.i + aligned_offset), d_in.i[aligned_offset], 0, 0, 0);
  EXPECT_ASSERTION(ozz::math::simd_int4::LoadXPtrU(reinterpret_cast<const int*>(d_in.c + 1)), "alignment");
  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::Load1PtrU(d_in.i + aligned_offset), d_in.i[aligned_offset], d_in.i[aligned_offset], d_in.i[aligned_offset], d_in.i[aligned_offset]);
  EXPECT_ASSERTION(ozz::math::simd_int4::Load1PtrU(reinterpret_cast<const int*>(d_in.c + 1)), "alignment");

  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::LoadXPtr(d_in.i + aligned_offset), d_in.i[aligned_offset], 0, 0, 0);
  EXPECT_ASSERTION(ozz::math::simd_int4::LoadXPtr(d_in.i + aligned_offset + 1), "alignment");
  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::Load1Ptr(d_in.i + aligned_offset), d_in.i[aligned_offset], d_in.i[aligned_offset], d_in.i[aligned_offset], d_in.i[aligned_offset]);
  EXPECT_ASSERTION(ozz::math::simd_int4::Load1Ptr(d_in.i + aligned_offset + 1), "alignment");

  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::Load2Ptr(d_in.i + aligned_offset), d_in.i[aligned_offset], d_in.i[aligned_offset + 1], 0, 0);
  EXPECT_ASSERTION(ozz::math::simd_int4::Load2Ptr(d_in.i + aligned_offset + 1), "alignment");
  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::Load2PtrU(d_in.i + aligned_offset + 1), d_in.i[aligned_offset + 1], d_in.i[aligned_offset + 2], 0, 0);
  EXPECT_ASSERTION(ozz::math::simd_int4::Load2PtrU(reinterpret_cast<const int*>(d_in.c + 1)), "alignment");

  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::Load3Ptr(d_in.i + aligned_offset), d_in.i[aligned_offset], d_in.i[aligned_offset + 1], d_in.i[aligned_offset + 2], 0);
  EXPECT_ASSERTION(ozz::math::simd_int4::Load3Ptr(d_in.i + aligned_offset + 1), "alignment");
  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::Load3PtrU(d_in.i + aligned_offset + 1), d_in.i[aligned_offset + 1], d_in.i[aligned_offset + 2], d_in.i[aligned_offset + 3], 0);
  EXPECT_ASSERTION(ozz::math::simd_int4::Load3PtrU(reinterpret_cast<const int*>(d_in.c + 1)), "alignment");
}

TEST(GetInt, ozz_simd_math) {
  const SimdInt4 i = ozz::math::simd_int4::Load(1, -1, 2, -3);

  EXPECT_EQ(ozz::math::GetX(i), 1);
  EXPECT_EQ(ozz::math::GetY(i), -1);
  EXPECT_EQ(ozz::math::GetZ(i), 2);
  EXPECT_EQ(ozz::math::GetW(i), -3);
}

TEST(SetInt, ozz_simd_math) {
  const SimdInt4 i = ozz::math::simd_int4::Load(1, -1, 2, -3);

  const SimdInt4 j = ozz::math::SetX(i, 11);
  EXPECT_SIMDINT_EQ(j, 11, -1, 2, -3);

  const SimdInt4 k = ozz::math::SetY(i, -11);
  EXPECT_SIMDINT_EQ(k, 1, -11, 2, -3);

  const SimdInt4 l = ozz::math::SetZ(i, 21);
  EXPECT_SIMDINT_EQ(l, 1, -1, 21, -3);

  const SimdInt4 m = ozz::math::SetW(i, -31);
  EXPECT_SIMDINT_EQ(m, 1, -1, 2, -31);

  EXPECT_ASSERTION(ozz::math::SetI(i, 7, 46), "range");
  const SimdInt4 i3 = ozz::math::SetI(i, 2, 46);
  EXPECT_SIMDINT_EQ(i3, 1, -1, 46, -3);
}

TEST(StoreIntPtr, ozz_simd_math) {
  const SimdInt4 i4 = ozz::math::simd_int4::Load(-1, 1, 2, 3);

  union Data {
    int i[4 + 4];  // The 2nd float isn't aligned to a SimdInt4.
    SimdInt4 i4[2];  // Forces alignment.
    char c[(4 + 4) * sizeof(int)];  // The 2nd char isn't aligned to an integer.
  };

  {
    Data d_out = {};
    ozz::math::StorePtrU(i4, d_out.i + 1);
    EXPECT_EQ(d_out.i[0], 0);
    EXPECT_EQ(d_out.i[1], -1);
    EXPECT_EQ(d_out.i[2], 1);
    EXPECT_EQ(d_out.i[3], 2);
    EXPECT_EQ(d_out.i[4], 3);
    EXPECT_ASSERTION(ozz::math::StorePtrU(i4, reinterpret_cast<int*>(d_out.c + 1)), "alignment");
  }
  {
    Data d_out = {};
    ozz::math::Store1PtrU(i4, d_out.i + 1);
    EXPECT_EQ(d_out.i[0], 0);
    EXPECT_EQ(d_out.i[1], -1);
    EXPECT_EQ(d_out.i[2], 0);
    EXPECT_EQ(d_out.i[3], 0);
    EXPECT_EQ(d_out.i[4], 0);
    EXPECT_ASSERTION(ozz::math::Store1PtrU(i4, reinterpret_cast<int*>(d_out.c + 1)), "alignment");
  }
  {
    Data d_out = {};
    ozz::math::Store2PtrU(i4, d_out.i + 1);
    EXPECT_EQ(d_out.i[0], 0);
    EXPECT_EQ(d_out.i[1], -1);
    EXPECT_EQ(d_out.i[2], 1);
    EXPECT_EQ(d_out.i[3], 0);
    EXPECT_EQ(d_out.i[4], 0);
    EXPECT_ASSERTION(ozz::math::Store2Ptr(i4, reinterpret_cast<int*>(d_out.c + 1)), "alignment");
  }
  {
    Data d_out = {};
    ozz::math::Store3PtrU(i4, d_out.i + 1);
    EXPECT_EQ(d_out.i[0], 0);
    EXPECT_EQ(d_out.i[1], -1);
    EXPECT_EQ(d_out.i[2], 1);
    EXPECT_EQ(d_out.i[3], 2);
    EXPECT_EQ(d_out.i[4], 0);
    EXPECT_ASSERTION(ozz::math::Store3Ptr(i4, reinterpret_cast<int*>(d_out.c + 1)), "alignment");
  }
  {
    Data d_out = {};
    ozz::math::StorePtr(i4, d_out.i);
    EXPECT_EQ(d_out.i[0], -1);
    EXPECT_EQ(d_out.i[1], 1);
    EXPECT_EQ(d_out.i[2], 2);
    EXPECT_EQ(d_out.i[3], 3);
    EXPECT_EQ(d_out.i[4], 0);
    EXPECT_ASSERTION(ozz::math::StorePtr(i4, d_out.i + 1), "alignment");
  }
  {
    Data d_out = {};
    ozz::math::Store1Ptr(i4, d_out.i);
    EXPECT_EQ(d_out.i[0], -1);
    EXPECT_EQ(d_out.i[1], 0);
    EXPECT_EQ(d_out.i[2], 0);
    EXPECT_EQ(d_out.i[3], 0);
    EXPECT_EQ(d_out.i[4], 0);
    EXPECT_ASSERTION(ozz::math::Store1Ptr(i4, d_out.i + 1), "alignment");
  }
  {
    Data d_out = {};
    ozz::math::Store2Ptr(i4, d_out.i);
    EXPECT_EQ(d_out.i[0], -1);
    EXPECT_EQ(d_out.i[1], 1);
    EXPECT_EQ(d_out.i[2], 0);
    EXPECT_EQ(d_out.i[3], 0);
    EXPECT_EQ(d_out.i[4], 0);
    EXPECT_ASSERTION(ozz::math::Store2Ptr(i4, d_out.i + 1), "alignment");
  }
  {
    Data d_out = {};
    ozz::math::Store3Ptr(i4, d_out.i);
    EXPECT_EQ(d_out.i[0], -1);
    EXPECT_EQ(d_out.i[1], 1);
    EXPECT_EQ(d_out.i[2], 2);
    EXPECT_EQ(d_out.i[3], 0);
    EXPECT_EQ(d_out.i[4], 0);
    EXPECT_ASSERTION(ozz::math::Store3Ptr(i4, d_out.i + 1), "alignment");
  }
}

TEST(ConstantInt, ozz_simd_math) {
  const SimdInt4 zero = ozz::math::simd_int4::zero();
  EXPECT_SIMDINT_EQ(zero, 0, 0, 0, 0);

  const SimdInt4 one = ozz::math::simd_int4::one();
  EXPECT_SIMDINT_EQ(one, 1, 1, 1, 1);

  const SimdInt4 x_axis = ozz::math::simd_int4::x_axis();
  EXPECT_SIMDINT_EQ(x_axis, 1, 0, 0, 0);

  const SimdInt4 y_axis = ozz::math::simd_int4::y_axis();
  EXPECT_SIMDINT_EQ(y_axis, 0, 1, 0, 0);

  const SimdInt4 z_axis = ozz::math::simd_int4::z_axis();
  EXPECT_SIMDINT_EQ(z_axis, 0, 0, 1, 0);

  const SimdInt4 w_axis = ozz::math::simd_int4::w_axis();
  EXPECT_SIMDINT_EQ(w_axis, 0, 0, 0, 1);

  const SimdInt4 all_true = ozz::math::simd_int4::all_true();
  EXPECT_SIMDINT_EQ(all_true, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

  const SimdInt4 all_false = ozz::math::simd_int4::all_false();
  EXPECT_SIMDINT_EQ(all_false, 0, 0, 0, 0);

  const SimdInt4 mask_sign = ozz::math::simd_int4::mask_sign();
  EXPECT_SIMDINT_EQ(mask_sign, 0x80000000, 0x80000000, 0x80000000, 0x80000000);

  const SimdInt4 mask_not_sign = ozz::math::simd_int4::mask_not_sign();
  EXPECT_SIMDINT_EQ(mask_not_sign, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff);

  const SimdInt4 mask_ffff = ozz::math::simd_int4::mask_ffff();
  EXPECT_SIMDINT_EQ(mask_ffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

  const SimdInt4 mask_0000 = ozz::math::simd_int4::mask_0000();
  EXPECT_SIMDINT_EQ(mask_0000, 0, 0, 0, 0);

  const SimdInt4 mask_fff0 = ozz::math::simd_int4::mask_fff0();
  EXPECT_SIMDINT_EQ(mask_fff0, 0xffffffff, 0xffffffff, 0xffffffff, 0);

  const SimdInt4 mask_f000 = ozz::math::simd_int4::mask_f000();
  EXPECT_SIMDINT_EQ(mask_f000, 0xffffffff, 0, 0, 0);

  const SimdInt4 mask_0f00 = ozz::math::simd_int4::mask_0f00();
  EXPECT_SIMDINT_EQ(mask_0f00, 0, 0xffffffff, 0, 0);

  const SimdInt4 mask_00f0 = ozz::math::simd_int4::mask_00f0();
  EXPECT_SIMDINT_EQ(mask_00f0, 0, 0, 0xffffffff, 0);

  const SimdInt4 mask_000f = ozz::math::simd_int4::mask_000f();
  EXPECT_SIMDINT_EQ(mask_000f, 0, 0, 0, 0xffffffff);
}

TEST(SplatInt, ozz_simd_math) {
  const SimdInt4 i = ozz::math::simd_int4::Load(1, -1, 2, -3);

  const SimdInt4 x = ozz::math::SplatX(i);
  EXPECT_SIMDINT_EQ(x, 1, 1, 1, 1);

  const SimdInt4 y = ozz::math::SplatY(i);
  EXPECT_SIMDINT_EQ(y, -1, -1, -1, -1);

  const SimdInt4 z = ozz::math::SplatZ(i);
  EXPECT_SIMDINT_EQ(z, 2, 2, 2, 2);

  const SimdInt4 w = ozz::math::SplatW(i);
  EXPECT_SIMDINT_EQ(w, -3, -3, -3, -3);
}

TEST(FromFloat, ozz_simd_math) {
  const ozz::math::SimdFloat4 f =
    ozz::math::simd_float4::Load(0.f, 46.93f, 46.26f, -93.99f);
  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::FromFloatRound(f),
                    0, 47, 46, -94);
  EXPECT_SIMDINT_EQ(ozz::math::simd_int4::FromFloatTrunc(f),
                    0, 46, 46, -93);
}

TEST(ArithmeticInt, ozz_simd_math) {
  const SimdInt4 a = ozz::math::simd_int4::Load(0, 1, 2, 3);
  const SimdInt4 b = ozz::math::simd_int4::Load(4, 5, -6, 7);
  
  const SimdInt4 hadd2 = ozz::math::HAdd2(a);
  EXPECT_SIMDINT_EQ(hadd2, 1, 1, 2, 3);

  const SimdInt4 hadd3 = ozz::math::HAdd3(a);
  EXPECT_SIMDINT_EQ(hadd3, 3, 1, 2, 3);

  const SimdInt4 hadd4 = ozz::math::HAdd4(a);
  EXPECT_SIMDINT_EQ(hadd4, 6, 1, 2, 3);

  const SimdInt4 abs = ozz::math::Abs(b);
  EXPECT_SIMDINT_EQ(abs, 4, 5, 6, 7);

  const SimdInt4 sign = ozz::math::Sign(b);
  EXPECT_SIMDINT_EQ(sign, 0, 0, 0x80000000, 0);
}

TEST(CompareInt, ozz_simd_math) {
  const SimdInt4 a = ozz::math::simd_int4::Load(0, 1, 2, 3);
  const SimdInt4 b = ozz::math::simd_int4::Load(4, 1, -6, 7);
  const SimdInt4 c = ozz::math::simd_int4::Load(4, 5, 6, 7);

  const SimdInt4 min = ozz::math::Min(a, b);
  EXPECT_SIMDINT_EQ(min, 0, 1, -6, 3);

  const SimdInt4 max = ozz::math::Max(a, b);
  EXPECT_SIMDINT_EQ(max, 4, 1, 2, 7);

  const SimdInt4 min0 = ozz::math::Min0(b);
  EXPECT_SIMDINT_EQ(min0, 0, 0, -6, 0);

  const SimdInt4 max0 = ozz::math::Max0(b);
  EXPECT_SIMDINT_EQ(max0, 4, 1, 0, 7);

  EXPECT_SIMDINT_EQ(ozz::math::Clamp(a, ozz::math::simd_int4::Load(-12, 2, 9, 3), c), 0, 2, 6, 3);

  const SimdInt4 eq1 = ozz::math::CmpEq(a, b);
  EXPECT_SIMDINT_EQ(eq1, 0, 0xffffffff, 0, 0);

  const SimdInt4 eq2 = ozz::math::CmpEq(a, a);
  EXPECT_SIMDINT_EQ(eq2, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

  const SimdInt4 neq1 = ozz::math::CmpNe(a, b);
  EXPECT_SIMDINT_EQ(neq1, 0xffffffff, 0, 0xffffffff, 0xffffffff);

  const SimdInt4 neq2 = ozz::math::CmpNe(a, a);
  EXPECT_SIMDINT_EQ(neq2, 0, 0, 0, 0);

  const SimdInt4 lt = ozz::math::CmpLt(a, b);
  EXPECT_SIMDINT_EQ(lt, 0xffffffff, 0, 0, 0xffffffff);

  const SimdInt4 le = ozz::math::CmpLe(a, b);
  EXPECT_SIMDINT_EQ(le, 0xffffffff, 0xffffffff, 0, 0xffffffff);

  const SimdInt4 gt = ozz::math::CmpGt(a, b);
  EXPECT_SIMDINT_EQ(gt, 0, 0, 0xffffffff, 0);

  const SimdInt4 ge = ozz::math::CmpGe(a, b);
  EXPECT_SIMDINT_EQ(ge, 0, 0xffffffff, 0xffffffff, 0);
}

TEST(MaskInt, ozz_simd_math) {
  EXPECT_EQ(ozz::math::MoveMask(ozz::math::simd_int4::all_false()), 0x00000000);
  EXPECT_EQ(ozz::math::MoveMask(ozz::math::simd_int4::all_true()), 0x0000000f);
  EXPECT_EQ(ozz::math::MoveMask(ozz::math::simd_int4::mask_f000()), 0x00000001);
  EXPECT_EQ(ozz::math::MoveMask(ozz::math::simd_int4::mask_0f00()), 0x00000002);
  EXPECT_EQ(ozz::math::MoveMask(ozz::math::simd_int4::mask_00f0()), 0x00000004);
  EXPECT_EQ(ozz::math::MoveMask(ozz::math::simd_int4::mask_000f()), 0x00000008);
  EXPECT_EQ(ozz::math::MoveMask(ozz::math::simd_int4::Load(0xffffffff,
                                                0x00000000,
                                                0x80000001,
                                                0x7fffffff)),
            0x00000005);
  EXPECT_EQ(ozz::math::MoveMask(ozz::math::simd_int4::Load(0xffffffff,
                                                0x1000000f,
                                                0x80000001,
                                                0x8ffffffe)),
            0x0000000d);
  EXPECT_TRUE(ozz::math::AreAllFalse(ozz::math::simd_int4::all_false()));
  EXPECT_FALSE(ozz::math::AreAllFalse(ozz::math::simd_int4::all_true()));
  EXPECT_FALSE(ozz::math::AreAllFalse(ozz::math::simd_int4::mask_000f()));

  EXPECT_TRUE(ozz::math::AreAllTrue(ozz::math::simd_int4::all_true()));
  EXPECT_FALSE(ozz::math::AreAllTrue(ozz::math::simd_int4::all_false()));
  EXPECT_FALSE(ozz::math::AreAllTrue(ozz::math::simd_int4::mask_000f()));

  EXPECT_TRUE(ozz::math::AreAllFalse3(ozz::math::simd_int4::all_false()));
  EXPECT_TRUE(ozz::math::AreAllFalse3(ozz::math::simd_int4::mask_000f()));
  EXPECT_FALSE(ozz::math::AreAllFalse3(ozz::math::simd_int4::all_true()));
  EXPECT_FALSE(ozz::math::AreAllFalse3(ozz::math::simd_int4::mask_f000()));

  EXPECT_TRUE(ozz::math::AreAllTrue3(ozz::math::simd_int4::all_true()));
  EXPECT_FALSE(ozz::math::AreAllTrue3(ozz::math::simd_int4::all_false()));
  EXPECT_FALSE(ozz::math::AreAllTrue3(ozz::math::simd_int4::mask_f000()));

  EXPECT_TRUE(ozz::math::AreAllFalse2(ozz::math::simd_int4::all_false()));
  EXPECT_TRUE(ozz::math::AreAllFalse2(ozz::math::simd_int4::mask_000f()));
  EXPECT_FALSE(ozz::math::AreAllFalse2(ozz::math::simd_int4::all_true()));
  EXPECT_FALSE(ozz::math::AreAllFalse2(ozz::math::simd_int4::mask_f000()));

  EXPECT_TRUE(ozz::math::AreAllTrue2(ozz::math::simd_int4::all_true()));
  EXPECT_FALSE(ozz::math::AreAllTrue2(ozz::math::simd_int4::all_false()));
  EXPECT_FALSE(ozz::math::AreAllTrue2(ozz::math::simd_int4::mask_f000()));

  EXPECT_TRUE(ozz::math::AreAllFalse1(ozz::math::simd_int4::all_false()));
  EXPECT_TRUE(ozz::math::AreAllFalse1(ozz::math::simd_int4::mask_000f()));
  EXPECT_FALSE(ozz::math::AreAllFalse1(ozz::math::simd_int4::all_true()));
  EXPECT_FALSE(ozz::math::AreAllFalse1(ozz::math::simd_int4::mask_f000()));

  EXPECT_TRUE(ozz::math::AreAllTrue1(ozz::math::simd_int4::all_true()));
  EXPECT_FALSE(ozz::math::AreAllTrue1(ozz::math::simd_int4::all_false()));
  EXPECT_TRUE(ozz::math::AreAllTrue1(ozz::math::simd_int4::mask_f000()));
}

TEST(LogicalInt, ozz_simd_math) {
  const SimdInt4 a = ozz::math::simd_int4::Load(0xffffffff, 0x00000000, 0x80000001, 0x7fffffff);
  const SimdInt4 b = ozz::math::simd_int4::Load(0x80000001, 0xffffffff, 0x7fffffff, 0x00000000);
  const SimdInt4 c = ozz::math::simd_int4::Load(0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef);

  const SimdInt4 andm = ozz::math::And(a, b);
  EXPECT_SIMDINT_EQ(andm, 0x80000001, 0x00000000, 0x00000001, 0x00000000);

  const SimdInt4 orm = ozz::math::Or(a, b);
  EXPECT_SIMDINT_EQ(orm, 0xffffffff, 0xffffffff, 0xffffffff, 0x7fffffff);

  const SimdInt4 xorm = ozz::math::Xor(a, b);
  EXPECT_SIMDINT_EQ(xorm, 0x7ffffffe, 0xffffffff, 0xfffffffe, 0x7fffffff);

  const SimdInt4 select = ozz::math::Select(a, b, c);
  EXPECT_SIMDINT_EQ(select, 0x80000001, 0x89abcdef, 0x01234567, 0x80000000);
}

TEST(ShiftInt, ozz_simd_math) {
  const SimdInt4 a = ozz::math::simd_int4::Load(0xffffffff, 0x00000000, 0x80000001, 0x7fffffff);

  const SimdInt4 shift_l = ozz::math::ShiftL(a, 3);
  EXPECT_SIMDINT_EQ(shift_l, 0xfffffff8, 0x00000000, 0x00000008, 0xfffffff8);

  const SimdInt4 shift_r = ozz::math::ShiftR(a, 3);
  EXPECT_SIMDINT_EQ(shift_r, 0xffffffff, 0x00000000, 0xf0000000, 0x0fffffff);

  const SimdInt4 shift_ru = ozz::math::ShiftRu(a, 3);
  EXPECT_SIMDINT_EQ(shift_ru, 0x1fffffff, 0x00000000, 0x10000000, 0x0fffffff);
}
