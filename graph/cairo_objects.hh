#ifndef CAIRO_OBJECTS_HH
#define CAIRO_OBJECTS_HH

#include <cairo.h>
#include <pango/pangocairo.h>
#include <memory>

#include "image.hh"

class Cairo
{
  Image image_;

  struct Surface
  {
    struct Deleter { void operator() ( cairo_surface_t * x ) { cairo_surface_destroy( x ); } };

    std::unique_ptr<cairo_surface_t, Deleter> surface;

    Surface( Image & image );

    void check_error( void );
  } surface_;

  struct Context
  {
    struct Deleter { void operator() ( cairo_t * x ) { cairo_destroy( x ); } };

    std::unique_ptr<cairo_t, Deleter> context;

    Context( Surface & surface );

    void check_error( void );
  } context_;

  static int stride_pixels_for_width( const unsigned int width );
  void check_error( void );

public:
  Cairo( const std::pair<unsigned int, unsigned int> size );

  operator cairo_t * () { return context_.context.get(); }

  Image & mutable_image( void ) { return image_; }
  const Image & image( void ) const { return image_; }

  template <bool device_coordinates>
  struct Extent
  {
    double x, y, width, height;

    Extent<false> to_user( Cairo & cairo ) const
    {
      static_assert( device_coordinates == true,
		     "Extent::to_user() called but coordinates already in user-space" );

      double x1 = x, x2 = x + width, y1 = y, y2 = y + height;

      cairo_device_to_user( cairo, &x1, &y1 );
      cairo_device_to_user( cairo, &x2, &y2 );

      return Extent<false>( { x1, y1, x2 - x1, y2 - y1 } );
    }

    Extent<true> to_device( Cairo & cairo ) const
    {
      static_assert( device_coordinates == false,
		     "Extent::to_device() called but coordinates already in device-space" );

      double x1 = x, x2 = x + width, y1 = y, y2 = y + height;

      cairo_user_to_device( cairo, &x1, &y1 );
      cairo_user_to_device( cairo, &x2, &y2 );

      return Extent<true>( { x1, y1, x2 - x1, y2 - y1 } );
    }
  };

  class Pattern
  {
    struct Deleter { void operator() ( cairo_pattern_t * x ) { cairo_pattern_destroy( x ); } };    

    std::unique_ptr<cairo_pattern_t, Deleter> pattern_;    

  public:
    Pattern( cairo_pattern_t * pattern );

    operator cairo_pattern_t * () { return pattern_.get(); }
  };
};

template <class T>
struct PangoDelete { void operator() ( T * x ) { g_object_unref( x ); } };

class Pango
{
  std::unique_ptr<PangoContext, PangoDelete<PangoContext>> context_;
  std::unique_ptr<PangoLayout, PangoDelete<PangoLayout>> layout_;

public:
  Pango( Cairo & cairo );

  operator PangoContext * () { return context_.get(); }
  operator PangoLayout * () { return layout_.get(); }

  struct Font
  {
    struct Deleter { void operator() ( PangoFontDescription * x ) { pango_font_description_free( x ); } };

    std::unique_ptr<PangoFontDescription, Deleter> font;

    Font( const std::string & description );

    operator const PangoFontDescription * () const { return font.get(); }
  };

  class Text
  {
    struct Deleter { void operator() ( cairo_path_t * x ) { cairo_path_destroy( x ); } };

    std::unique_ptr<cairo_path_t, Deleter> path_;
    Cairo::Extent<false> extent_;

  public:
    Text( Cairo & cairo, Pango & pango, const Font & font, const std::string & text );

    const Cairo::Extent<false> & extent( void ) const { return extent_; }

    void draw_centered_at( Cairo & cairo, const double x, const double y ) const;
    void draw_centered_rotated_at( Cairo & cairo, const double x, const double y ) const;

    operator const cairo_path_t * () const { return path_.get(); }
  };

  void set_font( const Font & font );
};

#endif /* CAIRO_OBJECTS_HH */
