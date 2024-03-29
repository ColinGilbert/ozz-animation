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

#ifndef OZZ_SAMPLES_FRAMEWORK_INTERNAL_SHADER_H_
#define OZZ_SAMPLES_FRAMEWORK_INTERNAL_SHADER_H_

#include "renderer_impl.h"

namespace ozz {
namespace math { struct Float4x4; }
namespace sample {
namespace internal {

// Declares a shader program.
class Shader {
 public:

  // Construct a fixed function pipeline shader. Use Shader::Build to specify
  // shader sources.
  Shader();

  // Destruct a shader.
  virtual ~Shader();

  // Constructs a shader from _vertex and _fragment glsl sources.
  // Mutliple source files can be specified using the *count argument.
  bool BuildFromSource(int _vertex_count, const char** _vertex,
                       int _fragment_count, const char** _fragment);

  // Returns the shader program that can be bound to the OpenGL context.
  GLuint program() const {
    return program_;
  }

  // Request an uniform location and pushes it to the uniform stack.
  // The uniform location is then accessible thought uniform().
  bool BindUniform(const char* _semantic);

  // Get an uniform location from the stack at index _index.
  GLint uniform(int _index) const {
    return uniforms_[_index];
  }

  // Request an attribute location and pushes it to the uniform stack.
  // The varying location is then accessible thought attrib().
  bool FindAttrib(const char* _semantic);

  // Get an varying location from the stack at index _index.
  GLint attrib(int _index) const {
    return attribs_[_index];
  }

  // Unbind all attribs from GL.
  void UnbindAttribs();

 private:

  // Shader program
  GLuint program_;

  // Vertex and fragment shaders
  GLuint vertex_;
  GLuint fragment_;

  // Uniform locations, in the order they were requested.
  ozz::Vector<GLint>::Std uniforms_;

  // Varying locations, in the order they were requested.
  ozz::Vector<GLint>::Std attribs_;
};

class ImmediatePCShader : public Shader{
public:
  ImmediatePCShader() {}
  virtual ~ImmediatePCShader() {}

  // Constructs the shader.
  // Returns NULL if shader compilation failed or a valid Shader pointer on
  // success. The shader must then be deleted using default allocator Delete
  // function.
  static ImmediatePCShader* Build();

  // Binds the shader.
  void Bind(const math::Float4x4& _model,
            const math::Float4x4& _view_proj,
            GLsizei _pos_stride, GLsizei _pos_offset,
            GLsizei _color_stride, GLsizei _color_offset);

  void Unbind();
};

class ImmediatePTCShader : public Shader{
public:
  ImmediatePTCShader() {}
  virtual ~ImmediatePTCShader() {}

  // Constructs the shader.
  // Returns NULL if shader compilation failed or a valid Shader pointer on
  // success. The shader must then be deleted using default allocator Delete
  // function.
  static ImmediatePTCShader* Build();

  // Binds the shader.
  void Bind(const math::Float4x4& _model,
            const math::Float4x4& _view_proj,
            GLsizei _pos_stride, GLsizei _pos_offset,
            GLsizei _tex_stride, GLsizei _tex_offset,
            GLsizei _color_stride, GLsizei _color_offset);

  void Unbind();
};

class SkeletonShader : public Shader {
 public:
  SkeletonShader() {}
  virtual ~SkeletonShader() {}

  // Binds the shader.
  void Bind(const math::Float4x4& _model,
            const math::Float4x4& _view_proj,
            GLsizei _pos_stride, GLsizei _pos_offset,
            GLsizei _normal_stride, GLsizei _normal_offset,
            GLsizei _color_stride, GLsizei _color_offset);

  void Unbind();

  // Get an attribute location for the join, in cased of instanced rendering.
  GLint joint_instanced_attrib() const {
    return attrib(3);
  }

  // Get an uniform location for the join, in cased of non-instanced rendering.
  GLint joint_uniform() const {
    return uniform(1);
  }
};

class JointShader : public SkeletonShader{
 public:
  JointShader() {}
  virtual ~JointShader() {}

   // Constructs the shader.
   // Returns NULL if shader compilation failed or a valid Shader pointer on
   // success. The shader must then be deleted using default allocator Delete
   // function.
   static JointShader* Build();
};

class BoneShader : public SkeletonShader{
public:
  BoneShader() {}
  virtual ~BoneShader() {}

  // Constructs the shader.
  // Returns NULL if shader compilation failed or a valid Shader pointer on
  // success. The shader must then be deleted using default allocator Delete
  // function.
  static BoneShader* Build();
};

class AmbientShader : public Shader{
public:
  AmbientShader() {}
  virtual ~AmbientShader() {}

  // Constructs the shader.
  // Returns NULL if shader compilation failed or a valid Shader pointer on
  // success. The shader must then be deleted using default allocator Delete
  // function.
  static AmbientShader* Build();

  // Binds the shader.
  void Bind(const math::Float4x4& _model,
            const math::Float4x4& _view_proj,
            GLsizei _pos_stride, GLsizei _pos_offset,
            GLsizei _normal_stride, GLsizei _normal_offset,
            GLsizei _color_stride, GLsizei _color_offset);

  void Unbind();
};
}  // internal
}  // sample
}  // ozz
#endif  // OZZ_SAMPLES_FRAMEWORK_INTERNAL_SHADER_H_
