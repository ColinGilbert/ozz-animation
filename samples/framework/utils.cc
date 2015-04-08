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

#include "framework/utils.h"

#include <limits>
#include <cassert>

#include "ozz/base/maths/box.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/memory/allocator.h"

#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/animation/runtime/local_to_model_job.h"

#include "ozz/geometry/runtime/skinning_job.h"

#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"
#include "ozz/base/log.h"

#include "framework/imgui.h"
#include "framework/mesh.h"

namespace ozz {
namespace sample {

PlaybackController::PlaybackController()
  : time_(0.f),
    playback_speed_(1.f),
    play_(true) {
}

void PlaybackController::Update(const animation::Animation& _animation,
                                float _dt) {
  if (!play_) {
    return;
  }
  const float new_time = time_ + _dt * playback_speed_;
  const float loops = new_time / _animation.duration();
  time_ = new_time - floorf(loops) * _animation.duration();
}

void PlaybackController::Reset() {
  time_ = 0.f;
  playback_speed_ = 1.f;
  play_ = true;
}

void PlaybackController::OnGui(const animation::Animation& _animation,
                               ImGui* _im_gui,
                               bool _enabled) {
  if (_im_gui->DoButton(play_ ? "Pause" : "Play", _enabled)) {
    play_ = !play_;
  }
  char szLabel[64];
  std::sprintf(szLabel, "Animation time: %.2f", time_);
  if (_im_gui->DoSlider(
    szLabel, 0.f, _animation.duration(), &time_, 1.f, _enabled)) {
    // Pause the time if slider as moved.
    play_ = false;
  }
  std::sprintf(szLabel, "Playback speed: %.2f", playback_speed_);
  _im_gui->DoSlider(szLabel, -5.f, 5.f, &playback_speed_, 1.f, _enabled);

  // Allow to reset speed if it is not the default value.
  if (_im_gui->DoButton(
    "Reset playback speed", playback_speed_ != 1.f && _enabled)) {
    playback_speed_ = 1.f;
  }
}

// Uses LocalToModelJob to compute skeleton model space posture, then forwards
// to ComputePostureBounds
void ComputeSkeletonBounds(const animation::Skeleton& _skeleton,
                           math::Box* _bound) {
  using ozz::math::Float4x4;

  assert(_bound);

  // Set a default box.
  *_bound = ozz::math::Box();

  const int num_joints = _skeleton.num_joints();
  if (!num_joints) {
    return;
  }

  // Allocate matrix array, out of memory is handled by the LocalToModelJob.
  memory::Allocator* allocator = memory::default_allocator();
  ozz::Range<ozz::math::Float4x4> models =
    allocator->AllocateRange<ozz::math::Float4x4>(num_joints);
  if (!models.begin) {
    return;
  }

  // Compute model space bind pose.
  ozz::animation::LocalToModelJob job;
  job.input = _skeleton.bind_pose();
  job.output = models;
  job.skeleton = &_skeleton;
  if (job.Run()) {
    // Forwards to posture function.
    ComputePostureBounds(models, _bound);
  }

  allocator->Deallocate(models);
}

// Loop through matrices and collect min and max bounds.
void ComputePostureBounds(ozz::Range<const ozz::math::Float4x4> _matrices,
                          math::Box* _bound) {
  assert(_bound);

  // Set a default box.
  *_bound = ozz::math::Box();

  if (!_matrices.begin || !_matrices.end) {
    return;
  }
  if (_matrices.begin > _matrices.end) {
    return;
  }

  math::SimdFloat4 min =
    math::simd_float4::Load1(std::numeric_limits<float>::max());
  math::SimdFloat4 max = -min;
  const ozz::math::Float4x4* current = _matrices.begin;
  while (current < _matrices.end) {
    min = math::Min(min, current->cols[3]);
    max = math::Max(max, current->cols[3]);
    ++current;
  }

  math::Store3PtrU(min, &_bound->min.x);
  math::Store3PtrU(max, &_bound->max.x);

  return;
}

bool LoadSkeleton(const char* _filename,
                  ozz::animation::Skeleton* _skeleton) {
  assert(_filename && _skeleton);
  ozz::log::Out() << "Loading skeleton archive " << _filename <<
    "." << std::endl;
  ozz::io::File file(_filename, "rb");
  if (!file.opened()) {
    ozz::log::Err() << "Failed to open skeleton file " << _filename << "."
      << std::endl;
    return false;
  }
  ozz::io::IArchive archive(&file);
  if (!archive.TestTag<ozz::animation::Skeleton>()) {
    ozz::log::Err() << "Failed to load skeleton instance from file " <<
      _filename << "." << std::endl;
    return false;
  }

  // Once the tag is validated, reading cannot fail.
  archive >> *_skeleton;

  return true;
}

bool LoadAnimation(const char* _filename,
                   ozz::animation::Animation* _animation) {
  assert(_filename && _animation);
  ozz::log::Out() << "Loading animation archive: " << _filename <<
    "." << std::endl;
  ozz::io::File file(_filename, "rb");
  if (!file.opened()) {
    ozz::log::Err() << "Failed to open animation file " << _filename <<
      "." << std::endl;
    return false;
  }
  ozz::io::IArchive archive(&file);
  if (!archive.TestTag<ozz::animation::Animation>()) {
    ozz::log::Err() << "Failed to load animation instance from file " <<
      _filename << "." << std::endl;
    return false;
  }

  // Once the tag is validated, reading cannot fail.
  archive >> *_animation;

  return true;
}

bool LoadMesh(const char* _filename,
              ozz::sample::Mesh* _mesh) {
  assert(_filename && _mesh);
  ozz::log::Out() << "Loading mesh archive: " << _filename <<
    "." << std::endl;
  ozz::io::File file(_filename, "rb");
  if (!file.opened()) {
    ozz::log::Err() << "Failed to open mesh file " << _filename <<
      "." << std::endl;
    return false;
  }
  ozz::io::IArchive archive(&file);
  if (!archive.TestTag<ozz::sample::Mesh>()) {
    ozz::log::Err() << "Failed to load mesh instance from file " <<
      _filename << "." << std::endl;
    return false;
  }

  // Once the tag is validated, reading cannot fail.
  archive >> *_mesh;

  return true;
}

SkinningMatricesUpdater::SkinningMatricesUpdater() {
}

SkinningMatricesUpdater::~SkinningMatricesUpdater() {
  memory::Allocator* allocator = memory::default_allocator();
  allocator->Deallocate(inverse_bind_pose_);
  allocator->Deallocate(skinning_matrices_);
}

bool SkinningMatricesUpdater::Initialize(const animation::Skeleton& _skeleton) {
  memory::Allocator* allocator = memory::default_allocator();

  // Setup inverse bind pose matrices.
  const int num_joints = _skeleton.num_joints();

  // Allocates skinning matrices.
  allocator->Reallocate<math::Float4x4>(skinning_matrices_, num_joints);

  // Build inverse bind-pose matrices, based on the input skeleton.
  allocator->Reallocate<math::Float4x4>(inverse_bind_pose_, num_joints);

  // Convert skeleton bind-pose in local space to model-space matrices using
  // the LocalToModelJob. Output is stored directly inside inverse_bind_pose_
  // which will then be inverted in-place.
  ozz::animation::LocalToModelJob ltm_job;
  ltm_job.skeleton = &_skeleton;
  ltm_job.input = _skeleton.bind_pose();
  ltm_job.output = inverse_bind_pose_;
  if (!ltm_job.Run()) {
    return false;
  }

  // Invert matrices in-place.
  for (int i = 0; i < num_joints; ++i) {
    inverse_bind_pose_[i] = Invert(inverse_bind_pose_[i]);
  }

  return true;
}

bool SkinningMatricesUpdater::Update(const Range<math::Float4x4> _model_space) {
  assert(skinning_matrices_.Count() == inverse_bind_pose_.Count());

  // Ensures input matrices buffer has the correct size.
  const size_t joints_count = _model_space.Count();
  if (joints_count != skinning_matrices_.Count()) {
    return false;
  }

  // Builds skinning matrices, based on the output of the animation stage.
  for (size_t i = 0; i < joints_count; ++i) {
    skinning_matrices_[i] = _model_space[i] * inverse_bind_pose_[i];
  }

  return true;
}
}  // sample
}  // ozz
