#include "figure.hh"
#include "cairo_objects.hh"

using namespace std;

Figure::Figure( const unsigned int initial_width,
                const unsigned int initial_height,
                const string & title )
  : display_( initial_width, initial_height, title ),
    cairo_( display_.window().size() ),
    size_( pair< unsigned int, unsigned int >( initial_width,
                                               initial_height )),
    next_graph_id_( 0 ),
    subgraphs_()
{
}

/* returns id of new subgraph so it can be referenced later */
const unsigned int & Figure::add_subgraph( const unsigned int num_lines,
                                           const string & xlabel,
                                           const string & ylabel,
                                           const float min_y, 
                                           const float max_y )
{
  auto graph_id = next_graph_id_;
  
  // instantiate a new graph with a unique id and add it to subgraphs.
  subgraphs_.emplace_back( Subgraph { 
      graph_id, 
        std::unique_ptr< Graph >( new Graph( num_lines, display_.window().size().first, subgraph_height( subgraphs_.size() + 1 ), xlabel, ylabel, min_y, max_y )),
        std::pair< float, float >( 0, 0 ) } );

  next_graph_id_++;
  return subgraphs_.back().id;
}

/* this one is silent if subgraph not found */
void Figure::remove_subgraph( const unsigned int subgraph_id ) {
  auto it = subgraphs_.begin();
  while( it < subgraphs_.end() ) {
    if( it->id == subgraph_id ) {
      subgraphs_.erase( it );
      break;
    }
  }
}

void Figure::add_data_point( const unsigned int subgraph_id,
                             const unsigned int line_idx,
                             const float t, const float y ) {
  bool found_subgraph = false;

  for( const auto & s : subgraphs_ ) {
    if( s.id == subgraph_id ) {
      found_subgraph = true;

      ( s.graph )->add_data_point( line_idx, t, y );

      break;
    }
  }
  
  if( !found_subgraph ) {
    throw runtime_error( "invalid subgraph ID provided" );
  }
}

void Figure::set_line_memory( const unsigned int subgraph_id,
                              const unsigned int line_idx,
                              const float line_memory ) 
{
  bool found_subgraph = false;

  for( auto & s : subgraphs_ ) {
    if( s.id == subgraph_id ) {
      found_subgraph = true;
      ( s.graph )->set_memory( line_idx, line_memory );
      
      break;
    }
  }
  
  if( !found_subgraph ) {
    throw runtime_error( "invalid subgraph ID provided" );
  }
}

void Figure::set_line_color( const unsigned int subgraph_id,
                             const unsigned int line_idx,
                             const float red, const float green, 
                             const float blue,
                             const float alpha )
{
  bool found_subgraph = false;

  for( auto & s : subgraphs_ ) {
    if( s.id == subgraph_id ) {
      found_subgraph = true;
      ( s.graph )->set_color( line_idx, red, green, blue, alpha );
      
      break;
    }
  }
  
  if( !found_subgraph ) {
    throw runtime_error( "invalid subgraph ID provided" );
  }
}

void Figure::set_subgraph_info( const unsigned int subgraph_id,
                                const string & info ) {
  bool found_subgraph = false;

  for( auto & s : subgraphs_ ) {
    if( s.id == subgraph_id ) {
      found_subgraph = true;
      ( s.graph )->set_info( info );
      
      break;
    }
  }
  
  if( !found_subgraph ) {
    throw runtime_error( "invalid subgraph ID provided" );
  }
}

void Figure::set_subgraph_ylimits( const unsigned int subgraph_id,
                                   const float min_y, 
                                   const float max_y )
{
  bool found_subgraph = false;

  for( auto & s : subgraphs_ ) {
    if( s.id == subgraph_id ) {
      found_subgraph = true;
      ( s.graph )->set_ylimits( min_y, max_y );
      
      break;
    }
  }
  
  if( !found_subgraph ) {
    throw runtime_error( "invalid subgraph ID provided" );
  }
}

void Figure::set_subgraph_xrange( const unsigned int subgraph_id,
                                  const std::pair< float, float > xrange ) {
  bool found_subgraph = false;

  for( auto & s : subgraphs_ ) {
    if( s.id == subgraph_id ) {
      found_subgraph = true;
      s.xrange = xrange;

      break;
    }
  }
  
  if( !found_subgraph ) {
    throw runtime_error( "invalid subgraph ID provided" );
  }
}

/*  construct each subgraph, then draw the figure. */
bool Figure::blocking_draw( void )
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

  /* get each subgraph image */
  auto height = subgraph_height( subgraphs_.size() );
  for( unsigned int i = 0; i < subgraphs_.size(); i++ ) {
    auto & s = subgraphs_.at( i );
    cairo_surface_t * surface = ( s.graph )->
      generate_graph( window_size.first, height, s.xrange );

    /* draw onto figure image */
    cairo_set_source_surface( cairo_, surface, 0, height * i );
    cairo_paint( cairo_ );
  }

  display_.draw( cairo_.image() );

  for( unsigned int i = 0; i < subgraphs_.size(); i++ ) {
    auto & s = subgraphs_.at( i );
    /* draw the lines */
    ( s.graph )->draw_lines( &display_, window_size.first,
                             height, height * i, window_size.second,
                             s.xrange);
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
