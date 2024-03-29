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

#include "ozz/animation/runtime/skeleton_utils.h"

#include "ozz/base/maths/soa_transform.h"

#include <assert.h>

namespace ozz {
namespace animation {

// Unpacks skeleton bind pose stored in soa format by the skeleton.
ozz::math::Transform GetJointBindPose(const Skeleton& _skeleton, int _joint) {
  assert(_joint >= 0 && _joint < _skeleton.num_joints() &&
         "Joint index out of range.");

  const ozz::math::SoaTransform& soa_transform =
    _skeleton.bind_pose()[_joint / 4];

  // Transpose SoA data to AoS.
  ozz::math::SimdFloat4 translations[4];
  ozz::math::Transpose3x4(&soa_transform.translation.x, translations);
  ozz::math::SimdFloat4 rotations[4];
  ozz::math::Transpose4x4(&soa_transform.rotation.x, rotations);
  ozz::math::SimdFloat4 scales[4];
  ozz::math::Transpose3x4(&soa_transform.scale.x, scales);

  // Stores to the Transform object.
  math::Transform bind_pose;
  const int offset = _joint % 4;
  ozz::math::Store3PtrU(translations[offset], &bind_pose.translation.x);
  ozz::math::StorePtrU(rotations[offset], &bind_pose.rotation.x);
  ozz::math::Store3PtrU(scales[offset], &bind_pose.scale.x);

  return bind_pose;
}

// Helper macro used to detect if a joint has a brother.
#define _HAS_SIBLING(_i, _num_joints, _properties) \
  ((_i + 1 < _num_joints) &&\
   (_properties[_i].parent == _properties[_i + 1].parent))

// Implement joint hierarchy depth-first traversal.
// Uses a non-recursive implementation to control stack usage (ie: making
// algorithm behavior (stack consumption) independent off the data being
// processed).
void IterateJointsDF(const Skeleton& _skeleton,
                     int _from,
                     JointsIterator* _iterator) {
  assert(_iterator);
  const int num_joints = _skeleton.num_joints();
  Range<const Skeleton::JointProperties> properties =
    _skeleton.joint_properties();

  // Initialize iterator.
  _iterator->num_joints = 0;

  // Validates input range first.
  if (num_joints == 0) {
    return;
  }
  if ((_from < 0 || _from >= num_joints) &&
      _from != Skeleton::kNoParentIndex) {
    return;
  }

  // Simulate a stack to unroll usual recursive implementation.
  struct Context {
    uint16_t joint:15;
    uint16_t has_brother:1;
  };
  Context stack[Skeleton::kMaxJoints];
  int stack_size = 0;

  // Initializes iteration start.
  Context start;
  if (_from != Skeleton::kNoParentIndex) {
    start.joint = _from;
    start.has_brother = false;  // Disallow brother processing.
  } else {  // num_joints > 0, which was tested as pre-conditions.
    start.joint = 0;
    start.has_brother = _HAS_SIBLING(0, num_joints, properties.begin);
  }
  stack[stack_size++] = start;

  for (; stack_size != 0;) {
    // Process next joint on the stack.
    const Context& top = stack[stack_size - 1];

    // Push that joint to the list and then process its child.
    _iterator->joints[_iterator->num_joints++] = top.joint;

    // Skip all the joints until the first child is found.
    if (!properties.begin[top.joint].is_leaf) {  // A leaf has no child anyway.
      uint16_t next_joint = top.joint + 1;
      for (;
           next_joint < num_joints &&
           top.joint != properties.begin[next_joint].parent;
           ++next_joint) {
      }
      if (next_joint < num_joints) {
        const Context next = {
          next_joint,
          _HAS_SIBLING(next_joint, num_joints, properties.begin)};
        stack[stack_size++] = next;  // Push child and process it.
        continue;
      }
    }

    // Rewind the stack while there's no brother to process.
    for (;stack_size != 0 && !stack[stack_size - 1].has_brother; --stack_size) {
    }

    // Replace top joint by its brother.
    if (stack_size != 0) {
      Context& next = stack[stack_size - 1];
      assert(next.has_brother && next.joint + 1 < num_joints);

      ++next.joint;  // The brother is the next joint in breadth-first order.
      next.has_brother = _HAS_SIBLING(next.joint, num_joints, properties.begin);
    }
  }
}
#undef _HAS_SIBLING
}  // animation
}  // ozz
