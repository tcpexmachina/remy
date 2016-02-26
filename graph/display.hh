#ifndef DISPLAY_HH
#define DISPLAY_HH

#include <deque>
#include <string>

#include "gl_objects.hh"

class Display
{
  static const std::string shader_source_scale_from_pixel_coordinates;
  static const std::string shader_source_passthrough_texture;
  static const std::string shader_source_solid_color;

  struct CurrentContextWindow
  {
    GLFWContext glfw_context_ = {};
    Window window_;

    CurrentContextWindow( const unsigned int width, const unsigned int height,
			  const std::string & title );
  } current_context_window_;

  VertexShader scale_from_pixel_coordinates_ = { shader_source_scale_from_pixel_coordinates };
  FragmentShader passthrough_texture_ = { shader_source_passthrough_texture };
  FragmentShader solid_color_ = { shader_source_solid_color };

  Program texture_shader_program_ = {};
  Program solid_color_shader_program_ = {};

  Texture texture_;

  VertexArrayObject texture_shader_array_object_ = {};
  VertexArrayObject solid_color_array_object_ = {};

  VertexBufferObject screen_corners_ = {};
  VertexBufferObject other_vertices_ = {};

public:
  Display( const unsigned int width, const unsigned int height,
	   const std::string & title );

  void draw( const Image & image );
  void draw( const float red, const float green, const float blue, const float alpha,
	     const float width,
	     const float cutoff,
	     const std::deque<std::pair<float, float>> & vertices,
	     const std::function<std::pair<float, float>(const std::pair<float, float> &)> & transform );
  void clear( void );

  void repaint( void );

  void swap( void );

  const Window & window( void ) const { return current_context_window_.window_; }

  void resize( const std::pair<unsigned int, unsigned int> & target_size );
};

#endif /* DISPLAY_HH */
