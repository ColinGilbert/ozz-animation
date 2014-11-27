//============================================================================//
//                                                                            //
// ozz-animation, 3d skeletal animation libraries and tools.                  //
// https://code.google.com/p/ozz-animation/                                   //
//                                                                            //
//----------------------------------------------------------------------------//
//                                                                            //
// Copyright (c) 2012-2014 Guillaume Blanc                                    //
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

#include "ozz/animation/offline/animation_delta_builder.h"

#include <cstddef>
#include <cassert>

#include "ozz/animation/offline/raw_animation.h"

namespace ozz {
namespace animation {
namespace offline {

// Setup default values (favoring quality).
AnimationDeltaBuilder::AnimationDeltaBuilder() {
}

bool AnimationDeltaBuilder::operator()(const RawAnimation& _input,
                                       RawAnimation* _output) const {
  if (!_output) {
    return false;
  }
  // Reset output animation to default.
  *_output = RawAnimation();

  // Validate animation.
  if (!_input.Validate()) {
    return false;
  }

  // Rebuilds output animation.
  _output->duration = _input.duration;
  int num_tracks = _input.num_tracks();
  _output->tracks.resize(num_tracks);
  /*
  for (int i = 0; i < num_tracks; ++i) {
    Filter(_input.tracks[i].translations,
           CompareTranslation, LerpTranslation, translation_tolerance,
           &_output->tracks[i].translations);
    Filter(_input.tracks[i].rotations,
           CompareRotation, LerpRotation, rotation_tolerance,
           &_output->tracks[i].rotations);
    Filter(_input.tracks[i].scales,
           CompareScale, LerpScale, scale_tolerance,
           &_output->tracks[i].scales);
  }
  */

  // Output animation is always valid though.
  return _output->Validate();
}
}  // offline
}  // animation
}  // ozz