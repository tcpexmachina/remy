#ifndef FINTREE_HH
#define FINTREE_HH

#include <array>

#include "configrange.hh"
#include "fin.hh"
#include "memoryrange.hh"
#include "dna.pb.h"

class FinTree {
private:
  MemoryRange _domain;

  std::vector< FinTree > _children;
  std::vector< Fin > _leaf;

  const Fin * fin( const Memory & _memory ) const;

public:
  FinTree();

  FinTree( const Fin & fin, const bool bisect );

  const Fin & use_fin( const Memory & _memory, const bool track ) const;

  bool replace( const Fin & w );
  bool replace( const Fin & src, const FinTree & dst );
  const Fin * most_used( const unsigned int max_generation ) const;

  void reset_counts( void );
  void promote( const unsigned int generation );
  void reset_generation( void );

  std::string str( void ) const;
  std::string str( const unsigned int total ) const;
  unsigned int total_fin_queries( void ) const;

  unsigned int num_children( void ) const;

  bool is_leaf( void ) const;

  RemyBuffers::FinTree DNA( void ) const;
  FinTree( const RemyBuffers::FinTree & dna );
};

#endif
