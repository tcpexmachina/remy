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

Graph::Graph( const unsigned int num_lines,
	      const unsigned int initial_width, 
              const unsigned int initial_height,
              const string & xlabel, const string & ylabel,
	      const float min_y, const float max_y,

              const float data_memory )
  : cairo_( std::pair< unsigned int, unsigned int > ( initial_width,
                                                      initial_height )),
    pango_( cairo_ ),
    tick_font_( "Nimbus Sans L, Normal 12" ),
    label_font_( "Nimbus Sans L, Normal 10" ),
    x_tick_labels_(),
    y_tick_labels_(),
    colors_( num_lines ),
    data_points_( num_lines ),
    x_label_( cairo_, pango_, label_font_, xlabel ),
    y_label_( cairo_, pango_, label_font_, ylabel ),
    info_string_(),
    info_( cairo_, pango_, label_font_, "" ),
    min_y_( min_y ),
    max_y_( max_y ),
    bottom_( min_y ),
    top_( max_y ),
    horizontal_fadeout_( cairo_pattern_create_linear( 0, 0, 190, 0 ) ),
    data_memory_size_( data_memory )
{
  cairo_pattern_add_color_stop_rgba( horizontal_fadeout_, 0.0, 1, 1, 1, 1 );
  cairo_pattern_add_color_stop_rgba( horizontal_fadeout_, 0.67, 1, 1, 1, 1 );
  cairo_pattern_add_color_stop_rgba( horizontal_fadeout_, 1.0, 1, 1, 1, 0 );
}

void Graph::set_info( const string & info )
{
  if ( info != info_string_ ) {
    info_string_ = info;
    info_ = Pango::Text( cairo_, pango_, tick_font_, info );
  }
}

/* void Graph::set_window( const float t, const float logical_width )
{
  for ( auto & line : data_points_ ) {
    while ( (line.size() >= 2) and (line.front().first < t - logical_width - 1)
	    and (line.at( 1 ).first < t - logical_width - 1) ) {
      line.pop_front();
    }
  }

  while ( (not x_tick_labels_.empty()) and (x_tick_labels_.front().first < t - logical_width - 1) ) {
    x_tick_labels_.pop_front();
  }
  } */

static int to_int( const float x )
{
  return static_cast<int>( lrintf( x ) );
}

double Graph::xtick_label_y( std::pair<unsigned int, unsigned int>
                                   graph_size ) 
{
  return graph_size.second * 9.0 / 10.0;
}

void Graph::draw_xtick_labels( std::pair< float, float > xrange, 
                               const float tick_interval,
                               std::pair<unsigned int, unsigned int> graph_size )
{
  assert( xrange.second >= xrange.first );

  std::deque<std::pair<float, Pango::Text>> new_xtick_labels;

  for( float i = floorf( xrange.first/10 )*10; i < xrange.second; i+= tick_interval ) {
    const float next_label = floorf(i * 10)/10;
    
    stringstream ss;
    ss.imbue( locale( "" ) );
    ss.precision( 2 );
    ss << fixed << next_label;
    
    new_xtick_labels.emplace_back( next_label, Pango::Text( cairo_, pango_, tick_font_, ss.str() ) );
  }

  /* draw the labels and vertical grid */
  for ( const auto & x : new_xtick_labels ) {
    /* position the text in the window */
    double x_position = ( (x.first - xrange.first) / abs( xrange.second - xrange.first )) * graph_size.first;
    if( abs( xrange.second - xrange.first ) < std::numeric_limits< float >::epsilon() ) {
      x_position = xrange.first;
    }

    x.second.draw_centered_at( cairo_,
			       x_position,
                               xtick_label_y( graph_size ),
			       0.85 * graph_size.first );

    cairo_set_source_rgba( cairo_, 0, 0, 0.4, 1 );
    cairo_fill( cairo_ );

    /* draw vertical grid line */
    cairo_identity_matrix( cairo_ );
    cairo_set_line_width( cairo_, 2 );
    cairo_move_to( cairo_, x_position, chart_height( bottom_, 
                                                     graph_size.second ) );
    cairo_line_to( cairo_, x_position, chart_height( top_, 
                                                     graph_size.second ) );
    cairo_set_source_rgba( cairo_, 0, 0, 0.4, 0.25 );
    cairo_stroke( cairo_ );
  }
}

cairo_surface_t * Graph::generate_graph( const float width, 
                                         const float height,
                                         const std::pair< float, float > xrange )
{
  /* set scale (with smoothing) */
  top_ = top_ * .95 + max_y_ * 0.05;
  bottom_ = bottom_ * 0.95 + min_y_ * 0.05;

  /* get the current window size */
  auto graph_size = std::pair< unsigned int, unsigned int >( width, height );

  /* do we need to resize? */
  if ( graph_size != cairo_.image().size() ) {
    cairo_ = Cairo( graph_size );
  }

  /* start a new image */
  cairo_.mutable_image().clear();

  /* draw the x-axis label */
  x_label_.draw_centered_at( cairo_, 35 + graph_size.first / 2, 
                             graph_size.second * 9.6 / 10.0 );
  cairo_set_source_rgba( cairo_, 0, 0, 0.4, 1 );
  cairo_fill( cairo_ );

  /* draw the updated x-tick labels */
  draw_xtick_labels( xrange, 0.5, graph_size );

  /* draw the y-axis labels */

  /* draw a box to hide other labels */
  cairo_new_path( cairo_ );
  cairo_identity_matrix( cairo_ );
  cairo_rectangle( cairo_, 0, 0, 190, graph_size.second );
  cairo_set_source( cairo_, horizontal_fadeout_ );
  cairo_fill( cairo_ );

  /* draw the info */
  info_.draw_centered_at( cairo_, graph_size.first / 2, 20, 
                          graph_size.first - 40 );
  cairo_set_source_rgba( cairo_, 0.4, 0, 0, 1 );
  cairo_fill( cairo_ );

  /* draw the y-axis label */
  y_label_.draw_centered_rotated_at( cairo_, 25, graph_size.second * .4375 );
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
    x.text.draw_centered_at( cairo_, 90, chart_height( x.height, graph_size.second ) );
    cairo_set_source_rgba( cairo_, 0, 0, 0.4, x.intensity );
    cairo_fill( cairo_ );

    /* draw horizontal grid line */
    cairo_identity_matrix( cairo_ );
    cairo_set_line_width( cairo_, 1 );
    cairo_move_to( cairo_, 140, chart_height( x.height, graph_size.second ) );
    cairo_line_to( cairo_, graph_size.first, chart_height( x.height, graph_size.second ) );
    cairo_set_source_rgba( cairo_, 0, 0, 0.4, 0.25 * x.intensity );
    cairo_stroke( cairo_ );
  }

  return cairo_.surface();
}

/* TODO subgraph should just pass line information to figure for drawing */
void Graph::draw_lines( Display* display, const float width,
                        const float height, const unsigned int y_shift,
                        const float window_height,
                        std::pair< float, float > xrange ) 
{
  auto graph_size = std::pair< unsigned int, unsigned int >( width, height );

  /* draw the data points, including an extension off the right edge */
  for ( unsigned int i = 0; i < data_points_.size(); i++ ) {
    auto & line = data_points_[ i ];

    if ( not line.empty() ) {
      float t = line.back().first;

      line.emplace_back( t + 0.001, line.back().second );

      display->draw( get<0>( colors_[ i ] ), get<1>( colors_[ i ] ),
                     get<2>( colors_[ i ] ), get<3>( colors_[ i ] ),
                     3.0, 220, 
                     std::pair< unsigned int, unsigned int >
                     ( 0, window_height - ( height + y_shift )),
                     width, height, line,
                     [&] ( const pair<float, float> & x ) {
                       return make_pair( (x.first - xrange.first) 
                                         / abs( xrange.second - xrange.first ) * 
                                         graph_size.first, // TODO
                                         chart_height( x.second, 
                                                       graph_size.second )
                                         + y_shift );
                     } );
      line.pop_back();
    }
  }
}

void Graph::set_color( const unsigned int num, const float red, 
                       const float green, const float blue,
		       const float alpha )
{
  if ( num < colors_.size() ) {
    colors_.at( num ) = make_tuple( red, green, blue, alpha );
  }
}
