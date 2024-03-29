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

#ifndef OZZ_OZZ_ANIMATION_RUNTIME_BLENDING_JOB_H_
#define OZZ_OZZ_ANIMATION_RUNTIME_BLENDING_JOB_H_

#include "ozz/base/maths/simd_math.h"

namespace ozz {

// Forward declaration of math structures.
namespace math { struct SoaTransform; }

namespace animation {

// Blends multiple input layer/postures to a single output. The number of
// transforms/joints blended by the job is defined by the number of transforms
// of the bind pose (note that this is a SoA format). This means that all
// buffers must be at least as big as the bind pose buffer.
// Partial animation blending is supported through optional joint weights that
// can be specified with layers joint_weights buffer. Unspecified joint weights
// are considered as a unit weight of 1.f.
// The job does not owned any buffers (input/output) and will thus not delete
// them during job's destruction.
struct BlendingJob {
  // Default constructor, initializes default values.
  BlendingJob();

  // Validates job parameters.
  // Returns true for a valid job, false otherwise:
  // -if layer range is not valid.
  // -if any layer is not valid.
  // -if output range is not valid.
  // -if any buffer (including layers' content : transform, joint weights...) is
  // smaller than the bind pose buffer.
  // -if the threshold value is less than or equal to 0.f.
  bool Validate() const;

  // Runs job's blending task.
  // The job is validated before any operation is performed, see Validate() for
  // more details.
  // Returns false if *this job is not valid.
  bool Run() const;

  // Defines a layer of blending input data (local space transforms) and
  // parameters (weights).
  struct Layer {
    // Default constructor, initializes default values.
    Layer();

    // Blending weight of this layer. Negative values are considered as 0.
    // Normalization is performed during the blending stage so weight can be in
    // any range, even though range [0:1] is optimal.
    float weight;

    // The range [begin,end[ of input layer posture. This buffer expect to store
    // local space transforms, that are usually outputted from a sampling job.
    // This range must be at least as big as the bind pose buffer, even though
    // only the number of transforms defined by the bind pose buffer will be
    // processed.
    Range<const math::SoaTransform> transform;

    // Optional range [begin,end[ of blending weight for each joint in this
    // layer.
    // If both pointers are NULL (default case) then per joint weight blending
    // is disabled.
    // A valid range is defined as being at least as big as the bind pose
    // buffer, even though only the number of transforms defined by the
    // bind pose buffer will be processed.
    // When a layer doesn't specifies per joint weights, then it is implicitly
    // considered as being 1.f. This default value is a reference value for
    // the normalization process, which implies that the range of values for
    // joint weights should be [0,1].
    // Negative weight values are considered as 0, but positive ones aren't
    // clamped because they could exceed 1.f if all layers contains valid joint
    // weights.
    Range<const math::SimdFloat4> joint_weights;
  };

  // The job blends the bind pose to the output when the accumulated weight of
  // all layers is less than this threshold value.
  // Must be greater than 0.f.
  float threshold;

  // Job input layers.
  // The range of layers that must be blended.
  Range<const Layer> layers;

  // The skeleton bind pose. The size of this buffer defines the number of
  // transforms to blend. This is the reference because this buffer is defined
  // by the skeleton that all the animations belongs to.
  // It is used when the accumulated weight for a bone on all layers is
  // less than the threshold value, in order to fall back on valid transforms.
  Range<const ozz::math::SoaTransform> bind_pose;

  // Job output.
  // The range of output transforms to be filled with blended layer
  // transforms during job execution.
  // Must be at least as big as the bind pose buffer, but only the number of
  // transforms defined by the bind pose buffer size will be processed.
  Range<ozz::math::SoaTransform> output;
};
}  // animation
}  // ozz
#endif  // OZZ_OZZ_ANIMATION_RUNTIME_BLENDING_JOB_H_
