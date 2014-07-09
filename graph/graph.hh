#ifndef GRAPH_HH
#define GRAPH_HH

#include "display.hh"
#include "cairo_objects.hh"

class Graph
{
  Cairo cairo_;
  Pango pango_;

  Pango::Font tick_font_;
  Pango::Font label_font_;

  struct YLabel
  {
    int height;
    Pango::Text text;
    float intensity;
  };

  struct Color
  {
    float r;
    float g;
    float b;
    float a;

    /* black */
    Color( void ) 
    : r( 0.0 ), g( 0.0 ), b( 0.0 ), a( 1.0 )
    {
    }
    
    Color( float red, float green, float blue, float alpha )
    : r( red ), g( green ), b( blue ), a( alpha )
    {
    }
  };

  struct Point
  {
    float x;
    float y;

    Point( void )
    : x( 0.0 ), y( 0.0 )
    {
    }

    Point( float p_x, float p_y )
      : x( p_x ), y( p_y )
    {
    }
  };

  struct Line
  {
    Color color;
    std::deque< Point > data_points;
    unsigned int data_memory; // number of data points to keep, -1 to never forget
    
    Line( void ) 
    : color(),
      data_points(),
      data_memory()
    {
    }
  };

  std::vector< Line > lines_;

  std::deque<std::pair<int, Pango::Text>> x_tick_labels_;
  std::vector<YLabel> y_tick_labels_;
  Pango::Text x_label_;
  Pango::Text y_label_;

  std::string info_string_;
  Pango::Text info_;

  float min_y_, max_y_;
  float bottom_, top_; // for smoothing

  float project_height( const float x ) const { return ( x - bottom_ ) / ( top_ - bottom_ ); }

  float chart_height( const float x, const unsigned int window_height ) const
  {
    return (window_height - 40) * (.825*(1-project_height( x ))+.025) + (.825 * 40);
  }

  double xtick_label_y( std::pair<unsigned int, unsigned int>
                        window_size );
  void draw_xtick_labels( std::pair< float, float > xrange, 
                          const float tick_interval,
                          std::pair<unsigned int, unsigned int> graph_size );

  Cairo::Pattern horizontal_fadeout_;

public:
  Graph( const unsigned int num_lines,
	 const unsigned int initial_width, const unsigned int initial_height,
         const std::string & xlabel, const std::string & ylabel,
	 const float min_y, const float max_y );

  /*  void set_window( const float t, const float logical_width ); */
  void add_data_point( const unsigned int num, const float t, const float y ) {
    if( num >= lines_.size() ) {
      return;
    }

    auto & line = lines_.at( num );

    line.data_points.emplace_back( t, y );

    if( line.data_memory > 0 ) {
      // forget old data
      while ( line.data_points.size() > line.data_memory ) {
        line.data_points.pop_front();
      }
    }
  }

  void set_color( const unsigned int num, const float red, const float green,
                  const float blue, const float alpha );

  void set_memory( const unsigned int num, const float line_memory );

  void set_ylimits( const float min_y, const float max_y ) {
    min_y_ = min_y;
    max_y_ = max_y;
  }

  cairo_surface_t * generate_graph( const float width, const float height,
                                    const std::pair<float, float> xrange );
  
  void draw_lines( Display* display, const float width,
                   const float height, const unsigned int y_shift,
                   const float window_height,
                   std::pair< float, float> xrange ); 

  void set_info( const std::string & info );
};

#endif /* GRAPH_HH */
