#ifndef FIGURE_HH
#define FIGURE_HH

#include "display.hh"
#include "cairo_objects.hh"
#include "graph.hh"

/*
  a figure to which subgraphs can be added
 */
class Figure 
{
  Display display_;
  Cairo cairo_;

  std::pair< unsigned int, unsigned int > size_;
  
  unsigned int next_graph_id_;
  std::vector< std::pair< unsigned int,
                          std::unique_ptr< Graph > > > subgraphs_;

  unsigned int subgraph_height( unsigned int n_graphs ) { return display_.window().size().second / n_graphs; }

public:
  Figure( const unsigned int initial_width, 
          const unsigned int initial_height,
          const std::string & title );

  const unsigned int & add_subgraph( const unsigned int num_lines,
                                     const std::string & xlabel,
                                     const std::string & ylabel,
                                     const float min_y, const float max_y );

  void remove_subgraph( const unsigned int subgraph_id );
  void refit_subgraphs( void );

  void add_data_point( const unsigned int subgraph_id,
                       const unsigned int line_idx,
                       const float t, const float y );

  void set_line_color( const unsigned int subgraph_id, 
                      const unsigned int line_idx,
                       const float red, const float green, const float blue,
                       const float alpha );

  void set_subgraph_ylimits( const unsigned int subgraph_id,
                             const float min_y, 
                             const float max_y );

  void set_subgraph_window( const unsigned int subgraph_id,
                            const float t,
                            const float logical_width );

  bool blocking_draw( const float t, const float logical_width );
};

#endif /* FIGURE_HH */

