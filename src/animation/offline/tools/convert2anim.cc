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

#include "ozz/animation/offline/tools/convert2anim.h"

#include <cstdlib>
#include <cstring>

#include "ozz/animation/offline/animation_builder.h"
#include "ozz/animation/offline/animation_optimizer.h"
#include "ozz/animation/offline/skeleton_builder.h"
#include "ozz/animation/offline/raw_animation.h"
#include "ozz/animation/offline/raw_skeleton.h"

#include "ozz/animation/runtime/skeleton.h"
#include "ozz/animation/runtime/animation.h"

#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"

#include "ozz/base/log.h"

#include "ozz/options/options.h"

// Declares command line options.
OZZ_OPTIONS_DECLARE_STRING(file, "Specifies input file", "", true)
OZZ_OPTIONS_DECLARE_STRING(skeleton, "Specifies ozz skeleton (raw or runtime) input file", "", true)
OZZ_OPTIONS_DECLARE_STRING(animation, "Specifies ozz animation output file", "", true)

OZZ_OPTIONS_DECLARE_BOOL(
  optimize,
  "Activate keyframes optimization stage.", true, false)

OZZ_OPTIONS_DECLARE_FLOAT(
  rotation,
  "Optimizer rotation tolerance in degrees",
  ozz::animation::offline::AnimationOptimizer().rotation_tolerance, false)
OZZ_OPTIONS_DECLARE_FLOAT(
  translation,
  "Optimizer translation tolerance in meters",
  ozz::animation::offline::AnimationOptimizer().translation_tolerance, false)
OZZ_OPTIONS_DECLARE_FLOAT(
  scale,
  "Optimizer scale tolerance in percents",
  ozz::animation::offline::AnimationOptimizer().scale_tolerance, false)

static bool ValidateEndianness(const ozz::options::Option& _option,
                               int /*_argc*/) {
  const ozz::options::StringOption& option =
    static_cast<const ozz::options::StringOption&>(_option);
  bool valid = std::strcmp(option.value(), "native") == 0 ||
               std::strcmp(option.value(), "little") == 0 ||
               std::strcmp(option.value(), "big") == 0;
  if (!valid) {
    ozz::log::Err() << "Invalid endianess option." << std::endl;
  }
  return valid;
}

OZZ_OPTIONS_DECLARE_STRING_FN(
  endian,
  "Selects output endianness mode. Can be \"native\" (same as current "\
  "platform), \"little\" or \"big\".",
  "native",
  false,
  &ValidateEndianness)

static bool ValidateLogLevel(const ozz::options::Option& _option,
                             int /*_argc*/) {
  const ozz::options::StringOption& option =
    static_cast<const ozz::options::StringOption&>(_option);
  bool valid = std::strcmp(option.value(), "verbose") == 0 ||
               std::strcmp(option.value(), "standard") == 0 ||
               std::strcmp(option.value(), "silent") == 0;
  if (!valid) {
    ozz::log::Err() << "Invalid log level option." << std::endl;
  }
  return valid;
}

OZZ_OPTIONS_DECLARE_STRING_FN(
  log_level,
  "Selects log level. Can be \"silent\", \"standard\" or \"verbose\".",
  "standard",
  false,
  &ValidateLogLevel)

static bool ValidateSamplingRate(const ozz::options::Option& _option,
                                 int /*_argc*/) {
    const ozz::options::FloatOption& option =
      static_cast<const ozz::options::FloatOption&>(_option);
    bool valid = option.value() > 0.f;
    if (!valid) {
      ozz::log::Err() << "Invalid sampling rate option." << std::endl;
    }
    return valid;
}

OZZ_OPTIONS_DECLARE_FLOAT_FN(
  sampling_rate,
  "Selects animation sampling rate in hertz",
  30.f,
  false,
  &ValidateSamplingRate)

OZZ_OPTIONS_DECLARE_BOOL(
  raw,
  "Outputs raw animation, instead of runtime animation.",
  false,
  false)

namespace ozz {
namespace animation {
namespace offline {

namespace {
void DisplaysOptimizationstatistics(const RawAnimation& _non_optimized,
                                    const RawAnimation& _optimized) {
  size_t opt_translations = 0, opt_rotations = 0, opt_scales = 0;
  for (size_t i =0; i < _optimized.tracks.size(); i++) {
    const RawAnimation::JointTrack& track = _optimized.tracks[i];
    opt_translations += track.translations.size();
    opt_rotations += track.rotations.size();
    opt_scales += track.scales.size();
  }
  size_t non_opt_translations = 0, non_opt_rotations = 0, non_opt_scales = 0;
  for (size_t i =0; i < _non_optimized.tracks.size(); i++) {
    const RawAnimation::JointTrack& track = _non_optimized.tracks[i];
    non_opt_translations += track.translations.size();
    non_opt_rotations += track.rotations.size();
    non_opt_scales += track.scales.size();
  }

  // Computes optimization ratios.
  float translation_ratio = non_opt_translations != 0 ?
    100.f * (non_opt_translations - opt_translations) / non_opt_translations : 0;
  float rotation_ratio = non_opt_rotations != 0 ?
    100.f * (non_opt_rotations - opt_rotations) / non_opt_rotations : 0;
  float scale_ratio = non_opt_scales != 0 ?
    100.f * (non_opt_scales - opt_scales) / non_opt_scales : 0;

  ozz::log::Log() << "Optimization stage results:" << std::endl;
  ozz::log::Log() << " - Translations key frames optimization: " <<
    translation_ratio << "%" << std::endl;
  ozz::log::Log() << " - Rotations key frames optimization: " <<
    rotation_ratio << "%" << std::endl;
  ozz::log::Log() << " - Scaling key frames optimization: " <<
    scale_ratio << "%" << std::endl;
}
}

int AnimationConverter::operator()(int _argc, const char** _argv) {
  // Parses arguments.
  ozz::options::ParseResult parse_result = ozz::options::ParseCommandLine(
    _argc, _argv,
    "1.1",
    "Imports a animation from a file and converts it to ozz binary raw or "
    "runtime animation format");
  if (parse_result != ozz::options::kSuccess) {
    return parse_result == ozz::options::kExitSuccess ?
      EXIT_SUCCESS : EXIT_FAILURE;
  }

  // Initializes log level from options.
  ozz::log::Level log_level = ozz::log::GetLevel();
  if (std::strcmp(OPTIONS_log_level, "silent") == 0) {
    log_level = ozz::log::Silent;
  } else if (std::strcmp(OPTIONS_log_level, "standard") == 0) {
    log_level = ozz::log::Standard;
  } else if (std::strcmp(OPTIONS_log_level, "verbose") == 0) {
    log_level = ozz::log::Verbose;
  }
  ozz::log::SetLevel(log_level);

  // Reads the skeleton from the binary ozz stream.
  ozz::animation::Skeleton* skeleton = NULL;
  {
    ozz::log::Log() << "Opens input skeleton ozz binary file: " <<
      OPTIONS_skeleton << std::endl;
    ozz::io::File file(OPTIONS_skeleton, "rb");
    if (!file.opened()) {
      ozz::log::Err() << "Failed to open input skeleton ozz binary file: " <<
        OPTIONS_skeleton << std::endl;
      return EXIT_FAILURE;
    }
    ozz::io::IArchive archive(&file);

    // File could contain a RawSkeleton or a Skeleton.
    if (archive.TestTag<ozz::animation::offline::RawSkeleton>()) {
      ozz::log::Log() << "Reading RawSkeleton from file." << std::endl;

      // Reading the skeleton cannot file.
      ozz::animation::offline::RawSkeleton raw_skeleton;
      archive >> raw_skeleton;

      // Builds runtime skeleton.
      ozz::log::Log() << "Builds runtime skeleton." << std::endl;
      ozz::animation::offline::SkeletonBuilder builder;
      skeleton = builder(raw_skeleton);
      if (!skeleton) {
        ozz::log::Err() << "Failed to build runtime skeleton." << std::endl;
        return EXIT_FAILURE;
      }
    } else if (archive.TestTag<ozz::animation::Skeleton>()) {
      // Reads input archive to the runtime skeleton.
      // This operation cannot fail.
      skeleton =
        ozz::memory::default_allocator()->New<ozz::animation::Skeleton>();
      archive >> *skeleton;
    } else {
      ozz::log::Err() << "Failed to read input skeleton from binary file: " <<
        OPTIONS_skeleton << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Imports animation from the document.
  ozz::log::Log() << "Importing file \"" << OPTIONS_file << "\"" <<
    std::endl;
  ozz::animation::offline::RawAnimation raw_animation;
  bool imported =
    Import(OPTIONS_file, *skeleton, OPTIONS_sampling_rate, &raw_animation);

  if (!imported) {
    ozz::log::Err() << "Failed to import file \"" << OPTIONS_file << "\"" <<
      std::endl;
    // No need for the skeleton anymore.
    ozz::memory::default_allocator()->Delete(skeleton);
    return EXIT_FAILURE;
  }

  // Optimizes animation if option is enabled.
  if (OPTIONS_optimize) {
    ozz::log::Log() << "Optimizing animation." << std::endl;
    ozz::animation::offline::AnimationOptimizer optimizer;
    optimizer.rotation_tolerance = OPTIONS_rotation;
    optimizer.translation_tolerance = OPTIONS_translation;
    optimizer.scale_tolerance = OPTIONS_scale;
    ozz::animation::offline::RawAnimation raw_optimized_animation;
    if (!optimizer(raw_animation, *skeleton, &raw_optimized_animation)) {
      ozz::log::Err() << "Failed to optimize animation." << std::endl;
      return EXIT_FAILURE;
    }

    // Displays optimization statistics.
    DisplaysOptimizationstatistics(raw_animation, raw_optimized_animation);

    // Brings data back to the raw animation.
    raw_animation = raw_optimized_animation;
  }

  // No need for the skeleton anymore.
  ozz::memory::default_allocator()->Delete(skeleton);

  // Builds runtime animation.
  ozz::animation::Animation* animation = NULL;
  if (!OPTIONS_raw) {
    ozz::log::Log() << "Builds runtime animation." << std::endl;
    ozz::animation::offline::AnimationBuilder builder;
    animation = builder(raw_animation);
    if (!animation) {
      ozz::log::Err() << "Failed to build runtime animation." << std::endl;
      return EXIT_FAILURE;
    }
  }

  {
    // Prepares output stream. File is a RAII so it will close automatically at
    // the end of this scope.
    // Once the file is opened, nothing should fail as it would leave an invalid
    // file on the disk.
    ozz::log::Log() << "Opens output file: " <<
      OPTIONS_animation.value() << std::endl;
    ozz::io::File file(OPTIONS_animation, "wb");
    if (!file.opened()) {
      ozz::log::Err() << "Failed to open output file: " <<
        OPTIONS_animation.value() << std::endl;
      ozz::memory::default_allocator()->Delete(animation);
      return EXIT_FAILURE;
    }

    // Initializes output endianness from options.
    ozz::Endianness endianness = ozz::GetNativeEndianness();
    if (std::strcmp(OPTIONS_endian, "little")) {
      endianness = ozz::kLittleEndian;
    } else if (std::strcmp(OPTIONS_endian, "big")) {
      endianness = ozz::kBigEndian;
    }
    ozz::log::Log() << (endianness == ozz::kLittleEndian ? "Little" : "Big") <<
      " Endian output binary format selected." << std::endl;

    // Initializes output archive.
    ozz::io::OArchive archive(&file, endianness);

    // Fills output archive with the animation.
    if (OPTIONS_raw) {
      ozz::log::Log() << "Outputs RawAnimation to binary archive." << std::endl;
      archive << raw_animation;
    } else {
      ozz::log::Log() << "Outputs Animation to binary archive." << std::endl;
      archive << *animation;      
    }
  }

  ozz::log::Log() << "Animation binary archive successfully outputted." <<
    std::endl;

  // Delete local objects.
  ozz::memory::default_allocator()->Delete(animation);

  return EXIT_SUCCESS;
}
}  // offline
}  // animation
}  // ozz
