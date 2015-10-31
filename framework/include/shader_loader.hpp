#ifndef SHADER_LOADER_HPP
#define SHADER_LOADER_HPP

#include <glbinding/gl/gl.h>

using namespace gl;

namespace shader_loader {
  // compile shader
  GLuint shader(std::string const& file_path, GLenum shader_type);
  // create program from vertex and fragment shader
  GLuint program(std::string const& vertex_name, std::string const& fragment_name);
  // create program from vertex, geometry and fragment shader
  GLuint program(std::string const& vertex_path, std::string const& geometry_path, std::string const& fragment_path);
};

#endif
