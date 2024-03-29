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

#include "ozz/geometry/runtime/skinning_job.h"

#include "gtest/gtest.h"

#include "ozz/base/log.h"
#include "ozz/base/memory/allocator.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/gtest_math_helper.h"

using ozz::geometry::SkinningJob;

TEST(JobValidity, SkinningJob) {

  ozz::math::Float4x4 matrices[2];
  ozz::math::Float4x4 it_matrices[2];
  uint16_t joint_indices[8];
  float joint_weights[6];
  float in_positions[6];
  float in_normals[6];
  float in_tangents[6];
  float out_positions[6];
  float out_normals[6];
  float out_tangents[6];

  { // Default is invalid.
    SkinningJob job;
    EXPECT_FALSE(job.Validate());
    EXPECT_FALSE(job.Run());
  }
  { // Valid job with 0 vertex.
    SkinningJob job;
    job.vertex_count = 0;
    job.influences_count = 1;
    job.joint_matrices = matrices;
    job.joint_indices.begin = joint_indices;
    job.joint_indices.end = joint_indices + 2;
    job.joint_indices_stride = sizeof(uint16_t) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Invalid job with 0 influence
    SkinningJob job;
    job.vertex_count = 0;
    job.influences_count = 0;
    job.joint_matrices = matrices;
    job.joint_indices.begin = joint_indices;
    job.joint_indices.end = joint_indices + 2;
    job.joint_indices_stride = sizeof(uint16_t) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Valid job with 1 influence
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 1;
    job.joint_matrices = matrices;
    job.joint_indices.begin = joint_indices;
    job.joint_indices.end = joint_indices + 2;
    job.joint_indices_stride = sizeof(uint16_t) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Valid job with 2 influences
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices.begin = joint_indices;
    job.joint_indices.end = joint_indices + 4;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights.begin = joint_weights;
    job.joint_weights.end = joint_weights + 2;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Valid job with 3 influences
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 3;
    job.joint_matrices = matrices;
    job.joint_indices.begin = joint_indices;
    job.joint_indices.end = joint_indices + 6;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights.begin = joint_weights;
    job.joint_weights.end = joint_weights + 4;
    job.joint_weights_stride = sizeof(float) * 2;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Valid job with 4 influences
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 4;
    job.joint_matrices = matrices;
    job.joint_indices.begin = joint_indices;
    job.joint_indices.end = joint_indices + 8;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights.begin = joint_weights;
    job.joint_weights.end = joint_weights + 6;
    job.joint_weights_stride = sizeof(float) * 2;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Valid job with 1 influence, unneeded weights.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 1;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Valid job with 1 influence, with inverse transposed matrices.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 1;
    job.joint_matrices = matrices;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Invalid job with 2 influences, missing indices.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 2 influences, missing weights.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 2 influences, not enough indices.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices.begin = joint_indices;
    job.joint_indices.end = joint_indices + 3;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 2 influences, not enough indices.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(joint_indices) + 1;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 2 influences, not enough weights.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights.begin = joint_weights;
    job.joint_weights.end = joint_weights + 1;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 2 influences, not enough weights.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(joint_weights) + 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Valid job with 2 influences
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Invalid job with 2 influences, mandatory positions.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 2 influences, mandatory positions input.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 2 influences, mandatory positions output.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Valid job with 2 influences, with normals.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Valid job with 2 influences, with normals but missing positions.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Valid job with 2 influences, with normals, mandatory output.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Valid job with 2 influences, with normals and tangents.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Validate());
  }
  { // Invalid job with 2 influences, with normals and tangents,
    // required output.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 2 influences, with tangents but missing normals.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 2;
    job.joint_matrices = matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.joint_weights = joint_weights;
    job.joint_weights_stride = sizeof(float) * 1;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
  { // Invalid job with 1 influence, bad inverse transposed matrices.
    SkinningJob job;
    job.vertex_count = 2;
    job.influences_count = 1;
    job.joint_matrices = matrices;
    job.joint_inverse_transpose_matrices.begin = it_matrices;
    job.joint_indices = joint_indices;
    job.joint_indices_stride = sizeof(uint16_t) * 2;
    job.in_positions = in_positions;
    job.in_positions_stride = sizeof(float) * 3;
    job.out_positions = out_positions;
    job.out_positions_stride = sizeof(float) * 3;
    EXPECT_FALSE(job.Validate());
  }
}

TEST(JobResult, SkinningJob) {

  ozz::math::Float4x4 matrices[4] = {
    {{ozz::math::simd_float4::Load(-1.f, 0.f, 0.f, 0.f),
      ozz::math::simd_float4::Load(0.f, 1.f, 0.f, 0.f),
      ozz::math::simd_float4::Load(0.f, 0.f, -1.f, 0.f),
      ozz::math::simd_float4::Load(0.f, 0.f, 0.f, 1.f)}},
    ozz::math::Float4x4::Translation(ozz::math::simd_float4::Load(1.f, 2.f, 3.f, 0.f)),
    ozz::math::Float4x4::Scaling(ozz::math::simd_float4::Load(1.f, 2.f, 3.f, 0.f)),
    ozz::math::Float4x4::Translation(ozz::math::simd_float4::Load(1.f, 2.f, 3.f, 0.f))
  };
  // This isn't the inverse transpose of matrices array, but no mind.
  ozz::math::Float4x4 it_matrices[4] = {
    {{ozz::math::simd_float4::Load(1.f, 0.f, 0.f, 0.f),
      ozz::math::simd_float4::Load(0.f, -1.f, 0.f, 0.f),
      ozz::math::simd_float4::Load(0.f, 0.f, 1.f, 0.f),
      ozz::math::simd_float4::Load(0.f, 0.f, 0.f, 1.f)}},
    ozz::math::Float4x4::identity(),
    {{ozz::math::simd_float4::Load(-1.f, 0.f, 0.f, 0.f),
      ozz::math::simd_float4::Load(0.f, -1.f, 0.f, 0.f),
      ozz::math::simd_float4::Load(0.f, 0.f, -1.f, 0.f),
      ozz::math::simd_float4::Load(0.f, 0.f, 0.f, 1.f)}},
    ozz::math::Float4x4::identity()
  }; 
  uint16_t joint_indices[10] = {0, 1, 2, 3, 0, 3, 2, 1, 0, 3};
  float joint_weights[8] = {.5f, .25f, .25f, .1f, .1f, .25f, .25f, .15f};
  float in_positions[6] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
  float in_normals[6] = {.1f, .2f, .3f, .4f, .5f, .6f};
  float in_tangents[6] = {.01f, .02f, .03f, .04f, .05f, .06f};
  float out_positions[6];
  float out_normals[6];
  float out_tangents[6];

  SkinningJob base_job;
  base_job.vertex_count = 2;
  base_job.influences_count = 1;
  base_job.joint_matrices = matrices;
  base_job.joint_indices = joint_indices;
  base_job.joint_indices_stride = sizeof(uint16_t) * 5;
  base_job.in_positions = in_positions;
  base_job.in_positions_stride = sizeof(float) * 3;
  base_job.out_positions = out_positions;
  base_job.out_positions_stride = sizeof(float) * 3;

  { // P1
    SkinningJob job = base_job;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(-1.f, out_positions[0]);
    EXPECT_FLOAT_EQ(2.f, out_positions[1]);
    EXPECT_FLOAT_EQ(-3.f, out_positions[2]);

    EXPECT_FLOAT_EQ(5.f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
  }
  { // PN1
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(-1.f, out_positions[0]);
    EXPECT_FLOAT_EQ(2.f, out_positions[1]);
    EXPECT_FLOAT_EQ(-3.f, out_positions[2]);
    EXPECT_FLOAT_EQ(-.1f, out_normals[0]);
    EXPECT_FLOAT_EQ(.2f, out_normals[1]);
    EXPECT_FLOAT_EQ(-.3f, out_normals[2]);

    EXPECT_FLOAT_EQ(5.f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
    EXPECT_FLOAT_EQ(.4f, out_normals[3]);
    EXPECT_FLOAT_EQ(.5f, out_normals[4]);
    EXPECT_FLOAT_EQ(.6f, out_normals[5]);
  }
  { // PitN1
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(-1.f, out_positions[0]);
    EXPECT_FLOAT_EQ(2.f, out_positions[1]);
    EXPECT_FLOAT_EQ(-3.f, out_positions[2]);
    EXPECT_FLOAT_EQ(.1f, out_normals[0]);
    EXPECT_FLOAT_EQ(-.2f, out_normals[1]);
    EXPECT_FLOAT_EQ(.3f, out_normals[2]);

    EXPECT_FLOAT_EQ(5.f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
    EXPECT_FLOAT_EQ(.4f, out_normals[3]);
    EXPECT_FLOAT_EQ(.5f, out_normals[4]);
    EXPECT_FLOAT_EQ(.6f, out_normals[5]);
  }
  { // PNT1
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(-1.f, out_positions[0]);
    EXPECT_FLOAT_EQ(2.f, out_positions[1]);
    EXPECT_FLOAT_EQ(-3.f, out_positions[2]);
    EXPECT_FLOAT_EQ(-.1f, out_normals[0]);
    EXPECT_FLOAT_EQ(.2f, out_normals[1]);
    EXPECT_FLOAT_EQ(-.3f, out_normals[2]);
    EXPECT_FLOAT_EQ(-.01f, out_tangents[0]);
    EXPECT_FLOAT_EQ(.02f, out_tangents[1]);
    EXPECT_FLOAT_EQ(-.03f, out_tangents[2]);

    EXPECT_FLOAT_EQ(5.f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
    EXPECT_FLOAT_EQ(.4f, out_normals[3]);
    EXPECT_FLOAT_EQ(.5f, out_normals[4]);
    EXPECT_FLOAT_EQ(.6f, out_normals[5]);
    EXPECT_FLOAT_EQ(.04f, out_tangents[3]);
    EXPECT_FLOAT_EQ(.05f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.06f, out_tangents[5]);
  }
  { // PitNT1
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(-1.f, out_positions[0]);
    EXPECT_FLOAT_EQ(2.f, out_positions[1]);
    EXPECT_FLOAT_EQ(-3.f, out_positions[2]);
    EXPECT_FLOAT_EQ(.1f, out_normals[0]);
    EXPECT_FLOAT_EQ(-.2f, out_normals[1]);
    EXPECT_FLOAT_EQ(.3f, out_normals[2]);
    EXPECT_FLOAT_EQ(.01f, out_tangents[0]);
    EXPECT_FLOAT_EQ(-.02f, out_tangents[1]);
    EXPECT_FLOAT_EQ(.03f, out_tangents[2]);

    EXPECT_FLOAT_EQ(5.f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
    EXPECT_FLOAT_EQ(.4f, out_normals[3]);
    EXPECT_FLOAT_EQ(.5f, out_normals[4]);
    EXPECT_FLOAT_EQ(.6f, out_normals[5]);
    EXPECT_FLOAT_EQ(.04f, out_tangents[3]);
    EXPECT_FLOAT_EQ(.05f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.06f, out_tangents[5]);
  }

  base_job.influences_count = 2;
  base_job.joint_weights = joint_weights;
  base_job.joint_weights_stride = sizeof(float) * 4;

  { // P2
    SkinningJob job = base_job;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.5f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(1.5f, out_positions[2]);

    EXPECT_FLOAT_EQ(4.1f, out_positions[3]);
    EXPECT_FLOAT_EQ(9.7f, out_positions[4]);
    EXPECT_FLOAT_EQ(17.1f, out_positions[5]);
  }
  { // PN2
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.5f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(1.5f, out_positions[2]);
    EXPECT_FLOAT_EQ(0.f, out_normals[0]);
    EXPECT_FLOAT_EQ(.2f, out_normals[1]);
    EXPECT_FLOAT_EQ(0.f, out_normals[2]);

    EXPECT_FLOAT_EQ(4.1f, out_positions[3]);
    EXPECT_FLOAT_EQ(9.7f, out_positions[4]);
    EXPECT_FLOAT_EQ(17.1f, out_positions[5]);
    EXPECT_FLOAT_EQ(.4f, out_normals[3]);
    EXPECT_FLOAT_EQ(.95f, out_normals[4]);
    EXPECT_FLOAT_EQ(1.68f, out_normals[5]);
  }
  { // PitN2
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.5f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(1.5f, out_positions[2]);
    EXPECT_FLOAT_EQ(.1f, out_normals[0]);
    EXPECT_FLOAT_EQ(0.f, out_normals[1]);
    EXPECT_FLOAT_EQ(.3f, out_normals[2]);

    EXPECT_FLOAT_EQ(4.1f, out_positions[3]);
    EXPECT_FLOAT_EQ(9.7f, out_positions[4]);
    EXPECT_FLOAT_EQ(17.1f, out_positions[5]);
    EXPECT_FLOAT_EQ(-.32f, out_normals[3]);
    EXPECT_FLOAT_EQ(-.4f, out_normals[4]);
    EXPECT_FLOAT_EQ(-.48f, out_normals[5]);
  }
  { // PNT2
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.5f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(1.5f, out_positions[2]);
    EXPECT_FLOAT_EQ(0.f, out_normals[0]);
    EXPECT_FLOAT_EQ(.2f, out_normals[1]);
    EXPECT_FLOAT_EQ(0.f, out_normals[2]);
    EXPECT_FLOAT_EQ(0.f, out_tangents[0]);
    EXPECT_FLOAT_EQ(.02f, out_tangents[1]);
    EXPECT_FLOAT_EQ(0.f, out_tangents[2]);

    EXPECT_FLOAT_EQ(4.1f, out_positions[3]);
    EXPECT_FLOAT_EQ(9.7f, out_positions[4]);
    EXPECT_FLOAT_EQ(17.1f, out_positions[5]);
    EXPECT_FLOAT_EQ(.4f, out_normals[3]);
    EXPECT_FLOAT_EQ(.95f, out_normals[4]);
    EXPECT_FLOAT_EQ(1.68f, out_normals[5]);
    EXPECT_FLOAT_EQ(.04f, out_tangents[3]);
    EXPECT_FLOAT_EQ(.095f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.168f, out_tangents[5]);
  }
  { // PitNT2
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.5f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(1.5f, out_positions[2]);
    EXPECT_FLOAT_EQ(.1f, out_normals[0]);
    EXPECT_FLOAT_EQ(0.f, out_normals[1]);
    EXPECT_FLOAT_EQ(.3f, out_normals[2]);
    EXPECT_FLOAT_EQ(0.01f, out_tangents[0]);
    EXPECT_FLOAT_EQ(0.f, out_tangents[1]);
    EXPECT_FLOAT_EQ(.03f, out_tangents[2]);

    EXPECT_FLOAT_EQ(4.1f, out_positions[3]);
    EXPECT_FLOAT_EQ(9.7f, out_positions[4]);
    EXPECT_FLOAT_EQ(17.1f, out_positions[5]);
    EXPECT_FLOAT_EQ(-.32f, out_normals[3]);
    EXPECT_FLOAT_EQ(-.4f, out_normals[4]);
    EXPECT_FLOAT_EQ(-.48f, out_normals[5]);
    EXPECT_FLOAT_EQ(-.032f, out_tangents[3]);
    EXPECT_FLOAT_EQ(-.04f, out_tangents[4]);
    EXPECT_FLOAT_EQ(-.048f, out_tangents[5]);
  }

  base_job.influences_count = 3;

  { // P3
    SkinningJob job = base_job;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);

    EXPECT_FLOAT_EQ(4.75f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.75f, out_positions[4]);
    EXPECT_FLOAT_EQ(11.25f, out_positions[5]);
  }
  { // PN3
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);
    EXPECT_FLOAT_EQ(0.f, out_normals[0]);
    EXPECT_FLOAT_EQ(.25f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);

    EXPECT_FLOAT_EQ(4.75f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.75f, out_positions[4]);
    EXPECT_FLOAT_EQ(11.25f, out_positions[5]);
    EXPECT_FLOAT_EQ(.4f, out_normals[3]);
    EXPECT_FLOAT_EQ(.625f, out_normals[4]);
    EXPECT_FLOAT_EQ(.9f, out_normals[5]);
  }
  { // PitN3
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);
    EXPECT_FLOAT_EQ(.05f, out_normals[0]);
    EXPECT_FLOAT_EQ(-.1f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);

    EXPECT_FLOAT_EQ(4.75f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.75f, out_positions[4]);
    EXPECT_FLOAT_EQ(11.25f, out_positions[5]);
    EXPECT_FLOAT_EQ(.2f, out_normals[3]);
    EXPECT_FLOAT_EQ(.25f, out_normals[4]);
    EXPECT_FLOAT_EQ(.3f, out_normals[5]);
  }
  { // PNT3
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);
    EXPECT_FLOAT_EQ(0.f, out_normals[0]);
    EXPECT_FLOAT_EQ(.25f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);
    EXPECT_FLOAT_EQ(0.f, out_tangents[0]);
    EXPECT_FLOAT_EQ(.025f, out_tangents[1]);
    EXPECT_FLOAT_EQ(.015f, out_tangents[2]);

    EXPECT_FLOAT_EQ(4.75f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.75f, out_positions[4]);
    EXPECT_FLOAT_EQ(11.25f, out_positions[5]);
    EXPECT_FLOAT_EQ(.4f, out_normals[3]);
    EXPECT_FLOAT_EQ(.625f, out_normals[4]);
    EXPECT_FLOAT_EQ(.9f, out_normals[5]);
    EXPECT_FLOAT_EQ(.04f, out_tangents[3]);
    EXPECT_FLOAT_EQ(.0625f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.09f, out_tangents[5]);
  }
  { // PitN3
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);
    EXPECT_FLOAT_EQ(.05f, out_normals[0]);
    EXPECT_FLOAT_EQ(-.1f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);
    EXPECT_FLOAT_EQ(.005f, out_tangents[0]);
    EXPECT_FLOAT_EQ(-.01f, out_tangents[1]);
    EXPECT_FLOAT_EQ(.015f, out_tangents[2]);

    EXPECT_FLOAT_EQ(4.75f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.75f, out_positions[4]);
    EXPECT_FLOAT_EQ(11.25f, out_positions[5]);
    EXPECT_FLOAT_EQ(.2f, out_normals[3]);
    EXPECT_FLOAT_EQ(.25f, out_normals[4]);
    EXPECT_FLOAT_EQ(.3f, out_normals[5]);
    EXPECT_FLOAT_EQ(.02f, out_tangents[3]);
    EXPECT_FLOAT_EQ(.025f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.03f, out_tangents[5]);
  }

  base_job.influences_count = 4;

  { // P4
    SkinningJob job = base_job;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(0.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);

    EXPECT_FLOAT_EQ(1.15f, out_positions[3]);
    EXPECT_FLOAT_EQ(6.95f, out_positions[4]);
    EXPECT_FLOAT_EQ(5.25f, out_positions[5]);
  }
  { // PN4
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(0.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);
    EXPECT_FLOAT_EQ(0.f, out_normals[0]);
    EXPECT_FLOAT_EQ(.25f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);

    EXPECT_FLOAT_EQ(1.15f, out_positions[3]);
    EXPECT_FLOAT_EQ(6.95f, out_positions[4]);
    EXPECT_FLOAT_EQ(5.25f, out_positions[5]);
    EXPECT_FLOAT_EQ(.08f, out_normals[3]);
    EXPECT_FLOAT_EQ(.625f, out_normals[4]);
    EXPECT_FLOAT_EQ(.42f, out_normals[5]);
  }
  { // PitN4
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(0.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);
    EXPECT_FLOAT_EQ(.05f, out_normals[0]);
    EXPECT_FLOAT_EQ(-.1f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);

    EXPECT_FLOAT_EQ(1.15f, out_positions[3]);
    EXPECT_FLOAT_EQ(6.95f, out_positions[4]);
    EXPECT_FLOAT_EQ(5.25f, out_positions[5]);
    EXPECT_FLOAT_EQ(.2f, out_normals[3]);
    EXPECT_FLOAT_EQ(-.15f, out_normals[4]);
    EXPECT_FLOAT_EQ(.3f, out_normals[5]);
  }
  { // PNT4
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(0.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);
    EXPECT_FLOAT_EQ(0.f, out_normals[0]);
    EXPECT_FLOAT_EQ(.25f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);
    EXPECT_FLOAT_EQ(0.f, out_tangents[0]);
    EXPECT_FLOAT_EQ(.025f, out_tangents[1]);
    EXPECT_FLOAT_EQ(.015f, out_tangents[2]);

    EXPECT_FLOAT_EQ(1.15f, out_positions[3]);
    EXPECT_FLOAT_EQ(6.95f, out_positions[4]);
    EXPECT_FLOAT_EQ(5.25f, out_positions[5]);
    EXPECT_FLOAT_EQ(.08f, out_normals[3]);
    EXPECT_FLOAT_EQ(.625f, out_normals[4]);
    EXPECT_FLOAT_EQ(.42f, out_normals[5]);
    EXPECT_FLOAT_EQ(.008f, out_tangents[3]);
    EXPECT_FLOAT_EQ(.0625f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.042f, out_tangents[5]);
  }
  { // PitNT4
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(0.25f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.f, out_positions[1]);
    EXPECT_FLOAT_EQ(2.25f, out_positions[2]);
    EXPECT_FLOAT_EQ(.05f, out_normals[0]);
    EXPECT_FLOAT_EQ(-.1f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);
    EXPECT_FLOAT_EQ(.005f, out_tangents[0]);
    EXPECT_FLOAT_EQ(-.01f, out_tangents[1]);
    EXPECT_FLOAT_EQ(.015f, out_tangents[2]);

    EXPECT_FLOAT_EQ(1.15f, out_positions[3]);
    EXPECT_FLOAT_EQ(6.95f, out_positions[4]);
    EXPECT_FLOAT_EQ(5.25f, out_positions[5]);
    EXPECT_FLOAT_EQ(.2f, out_normals[3]);
    EXPECT_FLOAT_EQ(-.15f, out_normals[4]);
    EXPECT_FLOAT_EQ(.3f, out_normals[5]);
    EXPECT_FLOAT_EQ(.02f, out_tangents[3]);
    EXPECT_FLOAT_EQ(-.015f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.03f, out_tangents[5]);
  }

  base_job.influences_count = 5;

  { // P5
    SkinningJob job = base_job;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.55f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.2f, out_positions[1]);
    EXPECT_FLOAT_EQ(3.15f, out_positions[2]);

    EXPECT_FLOAT_EQ(3.4f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.45f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
  }
  { // PN5
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.55f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.2f, out_positions[1]);
    EXPECT_FLOAT_EQ(3.15f, out_positions[2]);
    EXPECT_FLOAT_EQ(.02f, out_normals[0]);
    EXPECT_FLOAT_EQ(.25f, out_normals[1]);
    EXPECT_FLOAT_EQ(.21f, out_normals[2]);

    EXPECT_FLOAT_EQ(3.4f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.45f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
    EXPECT_FLOAT_EQ(.28f, out_normals[3]);
    EXPECT_FLOAT_EQ(.625f, out_normals[4]);
    EXPECT_FLOAT_EQ(.72f, out_normals[5]);
  }
  { // PitN5
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.55f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.2f, out_positions[1]);
    EXPECT_FLOAT_EQ(3.15f, out_positions[2]);
    EXPECT_FLOAT_EQ(.05f, out_normals[0]);
    EXPECT_FLOAT_EQ(-.06f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);

    EXPECT_FLOAT_EQ(3.4f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.45f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
    EXPECT_FLOAT_EQ(.2f, out_normals[3]);
    EXPECT_FLOAT_EQ(.1f, out_normals[4]);
    EXPECT_FLOAT_EQ(.3f, out_normals[5]);
  }
  { // PNT5
    SkinningJob job = base_job;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.55f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.2f, out_positions[1]);
    EXPECT_FLOAT_EQ(3.15f, out_positions[2]);
    EXPECT_FLOAT_EQ(.02f, out_normals[0]);
    EXPECT_FLOAT_EQ(.25f, out_normals[1]);
    EXPECT_FLOAT_EQ(.21f, out_normals[2]);
    EXPECT_FLOAT_EQ(.002f, out_tangents[0]);
    EXPECT_FLOAT_EQ(.025f, out_tangents[1]);
    EXPECT_FLOAT_EQ(.021f, out_tangents[2]);

    EXPECT_FLOAT_EQ(3.4f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.45f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
    EXPECT_FLOAT_EQ(.28f, out_normals[3]);
    EXPECT_FLOAT_EQ(.625f, out_normals[4]);
    EXPECT_FLOAT_EQ(.72f, out_normals[5]);
    EXPECT_FLOAT_EQ(.028f, out_tangents[3]);
    EXPECT_FLOAT_EQ(.0625f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.072f, out_tangents[5]);
  }
  { // PitNT5
    SkinningJob job = base_job;
    job.joint_inverse_transpose_matrices = it_matrices;
    job.in_normals = in_normals;
    job.in_normals_stride = sizeof(float) * 3;
    job.out_normals = out_normals;
    job.out_normals_stride = sizeof(float) * 3;
    job.in_tangents = in_tangents;
    job.in_tangents_stride = sizeof(float) * 3;
    job.out_tangents = out_tangents;
    job.out_tangents_stride = sizeof(float) * 3;
    EXPECT_TRUE(job.Run());

    EXPECT_FLOAT_EQ(.55f, out_positions[0]);
    EXPECT_FLOAT_EQ(3.2f, out_positions[1]);
    EXPECT_FLOAT_EQ(3.15f, out_positions[2]);
    EXPECT_FLOAT_EQ(.05f, out_normals[0]);
    EXPECT_FLOAT_EQ(-.06f, out_normals[1]);
    EXPECT_FLOAT_EQ(.15f, out_normals[2]);
    EXPECT_FLOAT_EQ(.005f, out_tangents[0]);
    EXPECT_FLOAT_EQ(-.006f, out_tangents[1]);
    EXPECT_FLOAT_EQ(.015f, out_tangents[2]);

    EXPECT_FLOAT_EQ(3.4f, out_positions[3]);
    EXPECT_FLOAT_EQ(7.45f, out_positions[4]);
    EXPECT_FLOAT_EQ(9.f, out_positions[5]);
    EXPECT_FLOAT_EQ(.2f, out_normals[3]);
    EXPECT_FLOAT_EQ(.1f, out_normals[4]);
    EXPECT_FLOAT_EQ(.3f, out_normals[5]);
    EXPECT_FLOAT_EQ(.02f, out_tangents[3]);
    EXPECT_FLOAT_EQ(.01f, out_tangents[4]);
    EXPECT_FLOAT_EQ(.03f, out_tangents[5]);
  }
}

struct BenchVertexIn {
  float pos[3];
  float normals[3];
  float tangents[3];
  uint16_t indices[8];
  float weights[7];
};

struct BenchVertexOut {
  float pos[3];
  float normals[3];
  float tangents[3];
};

TEST(Benchmark, SkinningJob) {

  const int vertex_count = 10000;
  const int joint_count = 100;

  // Prepares matrices.
  ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
  ozz::Range<ozz::math::Float4x4> matrices =
    allocator->AllocateRange<ozz::math::Float4x4>(joint_count);
  for (int i = 0; i < joint_count; ++i) {
    matrices[i] = ozz::math::Float4x4::identity();
  }

  // Prepares vertices.
  ozz::Range<BenchVertexIn> in_vertices =
    allocator->AllocateRange<BenchVertexIn>(vertex_count + 1);
  for (int i = 0; i < vertex_count; ++i) {
    BenchVertexIn& vertex = in_vertices[i];
    for (size_t j = 0; j < OZZ_ARRAY_SIZE(vertex.indices); ++j) {
      vertex.indices[j] = rand() % joint_count;
    }
    for (size_t j = 0; j < OZZ_ARRAY_SIZE(vertex.weights); ++j) {
      vertex.weights[j] = static_cast<float>(rand()) / RAND_MAX;
    }
    for (int j = 0; j < 3; ++j) {
      vertex.pos[j] = static_cast<float>(rand()) / RAND_MAX;
      vertex.normals[j] = static_cast<float>(rand()) / RAND_MAX;
      vertex.tangents[j] = static_cast<float>(rand()) / RAND_MAX;
    }
  }
  ozz::Range<BenchVertexOut> out_vertices =
    allocator->AllocateRange<BenchVertexOut>(vertex_count + 1);

  SkinningJob base_job;
  base_job.vertex_count = vertex_count;
  base_job.joint_matrices = matrices;
  base_job.joint_indices.begin = in_vertices.begin->indices;
  base_job.joint_indices.end = reinterpret_cast<const uint16_t*>(in_vertices.end);
  base_job.joint_indices_stride = sizeof(BenchVertexIn);
  base_job.joint_weights.begin = in_vertices.begin->weights;
  base_job.joint_weights.end = reinterpret_cast<const float*>(in_vertices.end);
  base_job.joint_weights_stride = sizeof(BenchVertexIn);
  base_job.in_positions.begin = in_vertices.begin->pos;
  base_job.in_positions.end = reinterpret_cast<const float*>(in_vertices.end);
  base_job.in_positions_stride = sizeof(BenchVertexIn);
  base_job.out_positions.begin = out_vertices.begin->pos;
  base_job.out_positions.end = reinterpret_cast<const float*>(out_vertices.end);
  base_job.out_positions_stride = sizeof(BenchVertexOut);

  for (int i = 1; i <= 8; ++i) {
    SkinningJob job = base_job;
    job.influences_count = i;

    // Pi
    {
      EXPECT_TRUE(job.Run());
    }
 
    // PNi
    {
      job.in_normals.begin = in_vertices.begin->normals;
      job.in_normals.end = reinterpret_cast<const float*>(in_vertices.end);
      job.in_normals_stride = sizeof(BenchVertexIn);
      job.out_normals.begin = out_vertices.begin->normals;
      job.out_normals.end = reinterpret_cast<const float*>(out_vertices.end);
      job.out_normals_stride = sizeof(BenchVertexOut);

      EXPECT_TRUE(job.Run());
    }
 
    // PitNi
    {
      job.joint_inverse_transpose_matrices = matrices;
      job.in_normals.begin = in_vertices.begin->normals;
      job.in_normals.end = reinterpret_cast<const float*>(in_vertices.end);
      job.in_normals_stride = sizeof(BenchVertexIn);
      job.out_normals.begin = out_vertices.begin->normals;
      job.out_normals.end = reinterpret_cast<const float*>(out_vertices.end);
      job.out_normals_stride = sizeof(BenchVertexOut);

      EXPECT_TRUE(job.Run());
    }

    // PNTi
    {
      job.in_tangents.begin = in_vertices.begin->tangents;
      job.in_tangents.end = reinterpret_cast<const float*>(in_vertices.end);
      job.in_tangents_stride = sizeof(BenchVertexIn);
      job.out_tangents.begin = out_vertices.begin->tangents;
      job.out_tangents.end = reinterpret_cast<const float*>(out_vertices.end);
      job.out_tangents_stride = sizeof(BenchVertexOut);

      EXPECT_TRUE(job.Run());
    }

    // PitNTi
    {
      job.joint_inverse_transpose_matrices = matrices;
      job.in_tangents.begin = in_vertices.begin->tangents;
      job.in_tangents.end = reinterpret_cast<const float*>(in_vertices.end);
      job.in_tangents_stride = sizeof(BenchVertexIn);
      job.out_tangents.begin = out_vertices.begin->tangents;
      job.out_tangents.end = reinterpret_cast<const float*>(out_vertices.end);
      job.out_tangents_stride = sizeof(BenchVertexOut);

      EXPECT_TRUE(job.Run());
    }
  }

  allocator->Deallocate(matrices);
  allocator->Deallocate(in_vertices);
  allocator->Deallocate(out_vertices);
}
