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

  std::deque<std::pair<int, Pango::Text>> x_tick_labels_;
  std::vector<YLabel> y_tick_labels_;
  std::vector<std::tuple<float, float, float, float>> colors_;
  std::vector<std::deque<std::pair<float, float>>> data_points_;

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

  unsigned int data_memory_size_; // number of data points to keep, -1 to never forget

public:
  Graph( const unsigned int num_lines,
	 const unsigned int initial_width, const unsigned int initial_height,
         const std::string & xlabel, const std::string & ylabel,
	 const float min_y, const float max_y, const float data_memory );

  /*  void set_window( const float t, const float logical_width ); */
  void add_data_point( const unsigned int num, const float t, const float y ) {
    data_points_.at( num ).emplace_back( t, y );

    if( data_memory_size_ > 0 ) {
      // forget old data
      for ( auto & line : data_points_ ) {
        while ( line.size() > data_memory_size_ ) {
          line.pop_front();
        }
      }
    }
  }

  void set_color( const unsigned int num, const float red, const float green,
                  const float blue, const float alpha );

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
