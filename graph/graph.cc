#include <cmath>
#include <sstream>
#include <locale>
#include <numeric>
#include <limits>
#include <algorithm>
#include <cassert>

#include <iostream>

#include "graph.hh"

using namespace std;

Graph::Graph( const unsigned int initial_width, const unsigned int initial_height, const string & title )
  : display_( initial_width, initial_height, title ),
    cairo_( display_.window().size() ),
    pango_( cairo_ ),
    tick_font_( "ACaslon Regular, Normal 30" ),
    label_font_( "ACaslon Regular, Normal 20" ),
    x_tick_labels_(),
    y_tick_labels_(),
    data_points_(),
    x_label_( cairo_, pango_, label_font_, "time (s)" ),
    y_label_( cairo_, pango_, label_font_, "packets in flight" ),
    bottom_adjustment_( 1.0 ),
    top_adjustment_( 1.0 ),
    bottom_( 0 ),
    top_( 1 ),
    horizontal_fadeout_( cairo_pattern_create_linear( 0, 0, 190, 0 ) )
{
  cairo_pattern_add_color_stop_rgba( horizontal_fadeout_, 0.0, 1, 1, 1, 1 );
  cairo_pattern_add_color_stop_rgba( horizontal_fadeout_, 0.67, 1, 1, 1, 1 );
  cairo_pattern_add_color_stop_rgba( horizontal_fadeout_, 1.0, 1, 1, 1, 0 );
}

void Graph::set_window( const float t, const float logical_width )
{
  while ( (not data_points_.empty()) and (data_points_.front().first < t - logical_width - 1) ) {
    data_points_.pop_front();
  }

  while ( (not x_tick_labels_.empty()) and (x_tick_labels_.front().first < t - logical_width - 1) ) {
    x_tick_labels_.pop_front();
  }
}

static int to_int( const float x )
{
  return static_cast<int>( lrintf( x ) );
}

bool Graph::blocking_draw( const float t, const float logical_width )
{
  /* get the current window size */
  const auto window_size = display_.window().size();

  /* do we need to resize? */
  if ( window_size != cairo_.image().size() ) {
    display_.resize( window_size );
    cairo_ = Cairo( window_size );
  }

  /* start a new image */
  cairo_.mutable_image().clear();

  /* do we need to make a new label? */
  while ( x_tick_labels_.empty() or (x_tick_labels_.back().first < t + 1) ) { /* start when offscreen */
    const int next_label = x_tick_labels_.empty() ? to_int( t ) : x_tick_labels_.back().first + 1;

    /* add commas as appropriate */
    stringstream ss;
    ss.imbue( locale( "" ) );
    ss << fixed << next_label;

    x_tick_labels_.emplace_back( next_label, Pango::Text( cairo_, pango_, tick_font_, ss.str() ) );
  }

  /* draw the labels and vertical grid */
  for ( const auto & x : x_tick_labels_ ) {
    /* position the text in the window */
    const double x_position = window_size.first - (t - x.first) * window_size.first / logical_width;

    x.second.draw_centered_at( cairo_,
			       x_position,
			       window_size.second * 9.0 / 10.0 );

    cairo_set_source_rgba( cairo_, 0, 0, 0.4, 1 );
    cairo_fill( cairo_ );

    /* draw vertical grid line */
    cairo_identity_matrix( cairo_ );
    cairo_set_line_width( cairo_, 2 );
    cairo_move_to( cairo_, x_position, window_size.second * 0.25 / 10.0 );
    cairo_line_to( cairo_, x_position, window_size.second * 8.5 / 10.0 );
    cairo_set_source_rgba( cairo_, 0, 0, 0.4, 0.25 );
    cairo_stroke( cairo_ );
  }

  /* draw the x-axis label */
  x_label_.draw_centered_at( cairo_, 35 + window_size.first / 2, window_size.second * 9.6 / 10.0 );
  cairo_set_source_rgba( cairo_, 0, 0, 0.4, 1 );
  cairo_fill( cairo_ );

  /* autoscale vertically */
  if ( not data_points_.empty() ) {
    /* adjust bottom and top */
    float data_max = accumulate( data_points_.begin(), data_points_.end(), numeric_limits<float>::min(),
				 [] ( const float x, const pair<float, float> & y ) {
				   return max( x, y.second ); } );
    float data_min = accumulate( data_points_.begin(), data_points_.end(), numeric_limits<float>::max(),
				 [] ( const float x, const pair<float, float> & y ) {
				   return min( x, y.second ); } );

    /* stop adjusting if data are good enough */
    if ( project_height( data_max ) > 0.833 ) {
      top_adjustment_ *= 0.95;
    }

    if ( project_height( data_min ) < 0.167 ) {
      bottom_adjustment_ *= 0.95;
    }

    /* expand weakly if data stays too far inside the graph */
    if ( project_height( data_max ) < 0.667 ) {
      top_adjustment_ = min( 0.02, top_adjustment_ + 0.02 / 15.0 );
    }

    if ( project_height( data_min ) > 0.333 ) {
      bottom_adjustment_ = min( 0.02, bottom_adjustment_ + 0.02 / 15.0 );
    }

    /* adjust strongly if data goes outside the graph */
    if ( project_height( data_max ) > 1.0 ) {
      top_adjustment_ = min( 0.05, top_adjustment_ + 0.05 / 15.0 );
    }

    if ( project_height( data_min ) < 0.0 ) {
      bottom_adjustment_ = min( 0.05, bottom_adjustment_ + 0.05 / 15.0 );
    }

    top_ = top_ * (1 - top_adjustment_) + (data_max + 0.15 * (data_max - data_min)) * top_adjustment_;
    bottom_ = bottom_ * (1 - bottom_adjustment_) + (data_min - 0.15 * (data_max - data_min)) * bottom_adjustment_;
  }

  /* draw the y-axis labels */

  /* draw a box to hide other labels */
  cairo_new_path( cairo_ );
  cairo_identity_matrix( cairo_ );
  cairo_rectangle( cairo_, 0, 0, 190, window_size.second );
  cairo_set_source( cairo_, horizontal_fadeout_ );
  cairo_fill( cairo_ );

  /* draw the y-axis label */
  y_label_.draw_centered_rotated_at( cairo_, 25, window_size.second * .4375 );
  cairo_set_source_rgba( cairo_, 0, 0, 0.4, 1 );
  cairo_fill( cairo_ );

  int label_bottom = to_int( floor( bottom_ ) );
  int label_top = to_int( ceil( top_ ) );
  int label_spacing = 1;

  while ( label_spacing < (label_top - label_bottom) / 4 ) {
    label_spacing *= 2;
  }

  label_bottom = (label_bottom / label_spacing) * label_spacing;
  label_top = (label_top / label_spacing) * label_spacing;

  /* cull old labels */
  {
    auto it = y_tick_labels_.begin();
    while ( it < y_tick_labels_.end() ) {
      if ( it->intensity < 0.01 ) {
	/* delete it */
	auto it_next = it + 1;
	y_tick_labels_.erase( it );
	it = it_next;
      } else {
	it++;
      }
    }
  }

  /* find the labels we actually want on this frame */
  vector<pair<int, bool>> labels_that_belong;

  for ( int val = label_bottom; val <= label_top; val += label_spacing ) {
    if ( project_height( val ) < 0 or project_height( val ) > 1 ) {
      continue;
    }

    labels_that_belong.emplace_back( val, false );
  }

  /* adjust current labels as necessary */
  for ( auto it = y_tick_labels_.begin(); it != y_tick_labels_.end(); it++ ) {
    bool belongs = false;
    for ( auto & y : labels_that_belong ) {
      if ( it->height == y.first ) {
	assert( y.second == false ); /* don't want duplicates */
	y.second = true;
	belongs = true;
	break;
      }
    }

    if ( belongs ) {
      it->intensity = 0.95 * it->intensity + 0.05;
    } else {
      it->intensity = 0.95 * it->intensity;
      /*
      if ( it->intensity < 0.05 ) {
	y_tick_labels_.erase( it );
      }
      */
    }
  }

  /* add new labels if necessary */
  for ( const auto & x : labels_that_belong ) {
    if ( x.second ) {
      /* already found */
      continue;
    }

    stringstream ss;
    ss.imbue( locale( "" ) );
    ss << fixed << x.first;

    y_tick_labels_.emplace_back( YLabel( { x.first, Pango::Text( cairo_, pango_, label_font_, ss.str() ), 0.05 } ) );
  }

  /* go through and paint all the labels */
  for ( const auto & x : y_tick_labels_ ) {
    x.text.draw_centered_at( cairo_, 90, chart_height( x.height, window_size.second ) );
    cairo_set_source_rgba( cairo_, 0, 0, 0.4, x.intensity );
    cairo_fill( cairo_ );

    /* draw horizontal grid line */
    cairo_identity_matrix( cairo_ );
    cairo_set_line_width( cairo_, 1 );
    cairo_move_to( cairo_, 140, chart_height( x.height, window_size.second ) );
    cairo_line_to( cairo_, window_size.first, chart_height( x.height, window_size.second ) );
    cairo_set_source_rgba( cairo_, 0, 0, 0.4, 0.25 * x.intensity );
    cairo_stroke( cairo_ );
  }

  /* draw the cairo surface on the OpenGL display */
  display_.draw( cairo_.image() );

  /* draw the data points, including an extension off the right edge */
  if ( not data_points_.empty() ) {
    data_points_.emplace_back( t + 20, data_points_.back().second );
    display_.draw( 1.0, 0.38, 0.0, 0.75, 5.0, 220, data_points_,
		   [&] ( const pair<float, float> & x ) {
		     return make_pair( window_size.first - (t - x.first) * window_size.first / logical_width,
				       chart_height( x.second, window_size.second ) );
		   } );
    data_points_.pop_back();
  }  

  /* swap buffers to reveal what has been drawn */
  display_.swap();

  /* should we quit? */
  glfwPollEvents();

  if ( display_.window().key_pressed( GLFW_KEY_ESCAPE ) or display_.window().should_close() ) {
    return true;
  }

  return false;
}
