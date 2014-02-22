#ifndef GRAPH_HH
#define GRAPH_HH

#include "display.hh"
#include "cairo_objects.hh"

class Graph
{
  Display display_;
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
  std::deque<std::pair<float, float>> data_points_;

  Pango::Text x_label_;
  Pango::Text y_label_;

  float bottom_adjustment_, top_adjustment_;
  float bottom_, top_;

  float project_height( const float x ) const { return ( x - bottom_ ) / ( top_ - bottom_ ); }
  float chart_height( const float x, const unsigned int window_height ) const
  {
    return window_height * (.825*(1-project_height( x ))+.025);
  }

  Cairo::Pattern horizontal_fadeout_;

public:
  Graph( const unsigned int initial_width, const unsigned int initial_height, const std::string & title );

  void set_window( const float t, const float logical_width );
  void add_data_point( const float t, const float y ) { data_points_.emplace_back( t, y ); }
  bool blocking_draw( const float t, const float logical_width );
};

#endif /* GRAPH_HH */
