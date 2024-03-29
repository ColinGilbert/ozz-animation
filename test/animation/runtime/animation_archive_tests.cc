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

#include "ozz/animation/runtime/animation.h"

#include "gtest/gtest.h"
#include "ozz/base/maths/gtest_math_helper.h"

#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"
#include "ozz/base/memory/allocator.h"

#include "ozz/base/maths/soa_transform.h"

#include "ozz/animation/runtime/sampling_job.h"

#include "ozz/animation/offline/raw_animation.h"
#include "ozz/animation/offline/animation_builder.h"

using ozz::animation::Animation;
using ozz::animation::TranslationKey;
using ozz::animation::RotationKey;
using ozz::animation::ScaleKey;
using ozz::animation::offline::RawAnimation;
using ozz::animation::offline::AnimationBuilder;

TEST(Empty, AnimationSerialize) {
  ozz::io::MemoryStream stream;

  // Streams out.
  ozz::io::OArchive o(&stream, ozz::GetNativeEndianness());

  Animation o_animation;
  o << o_animation;

  // Streams in.
  stream.Seek(0, ozz::io::Stream::kSet);
  ozz::io::IArchive i(&stream);

  Animation i_animation;
  i >> i_animation;

  EXPECT_EQ(o_animation.num_tracks(), i_animation.num_tracks());
}

TEST(Filled, AnimationSerialize) {
  // Builds a valid animation.
  Animation* o_animation = NULL;
  {
    RawAnimation raw_animation;
    raw_animation.duration = 1.f;
    raw_animation.tracks.resize(1);

    RawAnimation::TranslationKey t_key0 = {
      0.f, ozz::math::Float3(93.f, 58.f, 46.f)};
    raw_animation.tracks[0].translations.push_back(t_key0);
    RawAnimation::TranslationKey t_key1 = {
      .9f, ozz::math::Float3(46.f, 58.f, 93.f)};
    raw_animation.tracks[0].translations.push_back(t_key1);

    RawAnimation::RotationKey r_key = {
      0.7f, ozz::math::Quaternion(0.f, 1.f, 0.f, 0.f)};
    raw_animation.tracks[0].rotations.push_back(r_key);

    RawAnimation::ScaleKey s_key = {
      0.1f, ozz::math::Float3(99.f, 26.f, 14.f)};
    raw_animation.tracks[0].scales.push_back(s_key);

    AnimationBuilder builder;
    o_animation = builder(raw_animation);
    ASSERT_TRUE(o_animation != NULL);
  }

  for (int e = 0; e < 2; ++e) {
    ozz::Endianness endianess = e == 0 ? ozz::kBigEndian : ozz::kLittleEndian;
    ozz::io::MemoryStream stream;

    // Streams out.
    ozz::io::OArchive o(&stream, endianess);
    o << *o_animation;

    // Streams in.
    stream.Seek(0, ozz::io::Stream::kSet);
    ozz::io::IArchive i(&stream);

    Animation i_animation;
    i >> i_animation;

    ASSERT_FLOAT_EQ(o_animation->duration(), i_animation.duration());
    ASSERT_EQ(o_animation->num_tracks(), i_animation.num_tracks());
    EXPECT_EQ(o_animation->size(), i_animation.size());

    // Needs to sample to test the animation.
    ozz::animation::SamplingJob job;
    ozz::animation::SamplingCache cache(1);
    ozz::math::SoaTransform output[1];
    job.animation = o_animation;
    job.cache = &cache;
    job.output.begin = output;
    job.output.end = output + 1;

    // Samples and compares the two animations
    { // Samples at t = 0
      job.time = 0.f;
      job.Run();
      EXPECT_SOAFLOAT3_EQ_EST(output[0].translation, 93.f, 0.f, 0.f, 0.f,
                                                     58.f, 0.f, 0.f, 0.f,
                                                     46.f, 0.f, 0.f, 0.f);
      EXPECT_SOAQUATERNION_EQ_EST(output[0].rotation, 0.f, 0.f, 0.f, 0.f,
                                                      1.f, 0.f, 0.f, 0.f,
                                                      0.f, 0.f, 0.f, 0.f,
                                                      0.f, 1.f, 1.f, 1.f);
      EXPECT_SOAFLOAT3_EQ_EST(output[0].scale, 99.f, 1.f, 1.f, 1.f,
                                               26.f, 1.f, 1.f, 1.f,
                                               14.f, 1.f, 1.f, 1.f);
    }
    { // Samples at t = 1
      job.time = 1.f;
      job.Run();
      EXPECT_SOAFLOAT3_EQ_EST(output[0].translation, 46.f, 0.f, 0.f, 0.f,
                                                     58.f, 0.f, 0.f, 0.f,
                                                     93.f, 0.f, 0.f, 0.f);
      EXPECT_SOAQUATERNION_EQ_EST(output[0].rotation, 0.f, 0.f, 0.f, 0.f,
                                                      1.f, 0.f, 0.f, 0.f,
                                                      0.f, 0.f, 0.f, 0.f,
                                                      0.f, 1.f, 1.f, 1.f);
      EXPECT_SOAFLOAT3_EQ_EST(output[0].scale, 99.f, 1.f, 1.f, 1.f,
                                               26.f, 1.f, 1.f, 1.f,
                                               14.f, 1.f, 1.f, 1.f);
    }
  }
  ozz::memory::default_allocator()->Delete(o_animation);
}

TEST(AlreadyInitialized, AnimationSerialize) {

  ozz::io::MemoryStream stream;

  {
    ozz::io::OArchive o(&stream);

    RawAnimation raw_animation;
    raw_animation.duration = 1.f;
    raw_animation.tracks.resize(1);

    AnimationBuilder builder;
    Animation* o_animation = builder(raw_animation);
    ASSERT_TRUE(o_animation != NULL);
    o << *o_animation;
    ozz::memory::default_allocator()->Delete(o_animation);

    raw_animation.duration = 2.f;
    raw_animation.tracks.resize(2);
    o_animation = builder(raw_animation);
    ASSERT_TRUE(o_animation != NULL);
    o << *o_animation;
    ozz::memory::default_allocator()->Delete(o_animation);
  }

  {
    // Streams in.
    stream.Seek(0, ozz::io::Stream::kSet);
    ozz::io::IArchive i(&stream);

    // Reads and check the first animation.
    Animation i_animation;
    i >> i_animation;
    EXPECT_FLOAT_EQ(i_animation.duration(), 1.f);
    EXPECT_EQ(i_animation.num_tracks(), 1);

    // Reuse the animation a second time.
    i >> i_animation;
    EXPECT_FLOAT_EQ(i_animation.duration(), 2.f);
    ASSERT_EQ(i_animation.num_tracks(), 2);
  }
}
