#include "cycle-finder.hh"
#include "exception.hh"

#include <iomanip>

using namespace std;

const double END_TIME = 5000000.0;

typedef int64_t quantized_t;
const double quantizer = 100000000;

template <class SenderType1, class SenderType2>
static void print_state_variables( Network<SenderType1, SenderType2> network )
{
  cout << setw(8) << network.tickno();
  auto network_state = network.get_state();
  for ( unsigned int i = 0; i < network_state.size() - 1; i++ ) {
    cout << " " <<  setw(10) << network_state.at( i );
  }
  cout << " " << setw(20) << network_state.at( network_state.size() - 1 ) << endl;
}

static vector<quantized_t> all_down( const vector<double> & state ) {
  vector<quantized_t> ret;
  ret.reserve( state.size() );
  for ( const auto & x : state ) {
    ret.push_back( x * quantizer );
  }
  return ret;
}

static vector<vector<quantized_t>> fuzz_state( const vector<quantized_t> & state_all_down ) {
  vector<vector<quantized_t>> ret { state_all_down };

  /* bisect in each axis */
  for ( unsigned int i = 0; i < state_all_down.size(); i++ ) {
    continue;
    //if ( i == 4 or i == 9 or i == 10 or i == 11 ) continue;
    //if ( not ( i == 2 or i == 5 ) ) continue; 
    //if ( not ( i == state_all_down.size() - 1 or i == state_all_down.size() - 2 ) ) continue;

    vector<vector<quantized_t>> new_ret;

    for ( const auto & x : ret ) {
      /* make up version */
      auto x_upped = x;
      auto x_downed = x;
      x_upped.at( i )++;
      x_downed.at( i )--;
      new_ret.push_back( x );
      new_ret.push_back( x_upped );
      new_ret.push_back( x_downed );
    }

    ret = new_ret;
  }

  return ret;
}

static bool quantized_states_equal( std::vector<double> state1,
                             std::vector<double> state2 ) 
{
  const vector<quantized_t> state1_quantized = all_down( state1 );
  const vector<quantized_t> state2_quantized = all_down( state2 );

  return state1_quantized == state2_quantized;

  const vector<vector<quantized_t>> fuzzy_states_1 { fuzz_state( state1_quantized ) };

  for ( unsigned int i = 0; i < fuzzy_states_1.size(); i++ ) {
    if ( fuzzy_states_1.at( i ) == state2_quantized ) return true;
  }
  return false;
}

template <class SenderType1, class SenderType2>
CycleFinder<SenderType1, SenderType2>::CycleFinder( const Network< SenderType1, SenderType2 > & network, const double offset_value )
  : _network( network ),
    _offset_value( offset_value ),
    _cycle_start( network )
{
}

/*
  Brent's algorithm: run the fast network one event at a time
  and jump the slow network forward every power-of-two events.
 */
template < class SenderType1, class SenderType2 >
void CycleFinder<SenderType1, SenderType2>::run_until_cycle_found( bool verbose ) {
  if ( _cycle_found or _exception ) return;
  
  Network<SenderType1, SenderType2> network_fast( _network );
  
  bool found_cycle = false;
  unsigned int step_limit = 1;
  unsigned int steps_taken = 0;
  /* Phase 1: find a cycle */
  while ( network_fast.tickno() < END_TIME ) {
    Network<SenderType1, SenderType2> network_slow( network_fast );

    while ( steps_taken < step_limit ) {
      network_fast.run_until_event();
      
      steps_taken++;
      
      auto slow_network_state = network_slow.get_state();
      auto fast_network_state = network_fast.get_state();
      
      if ( quantized_states_equal( slow_network_state, fast_network_state ) ) {
        found_cycle = true;
        break;
      }
    } 

    if ( found_cycle ) break;

    steps_taken = 0;
    step_limit *= 2;
  }

  if ( not found_cycle ) {
    _exception = true;
    char end_str[256];
    sprintf(end_str, "No cycle found after %f ms", END_TIME);
    throw Exception( "find_cycles", end_str);
  }

  /* Phase 2: find the beginning of the cycle and calculate utility */
  Network<SenderType1, SenderType2> utility_network( _network );

  for ( unsigned int i = 0; i < steps_taken; i++ ) {
    utility_network.run_until_event();
  }

  while ( not ( quantized_states_equal( _cycle_start.get_state(),
                                        utility_network.get_state() ) ) ) {
    _cycle_start.run_until_event();
    utility_network.run_until_event();
  }
  _convergence_time = _cycle_start.tickno();
  _cycle_len = utility_network.tickno() - _cycle_start.tickno();

  auto start_tp_del = utility_network.senders().throughputs_delays();

  for ( unsigned int i = 0; i < steps_taken; i++ ) {
    utility_network.run_until_event();
    if ( verbose ) {
      /* output the cycle */
      print_state_variables( utility_network );
    }
  }
  auto end_tp_del = utility_network.senders().throughputs_delays();

  for( size_t i = 0; i < start_tp_del.size(); i++ ) {
    _deltas.emplace_back( end_tp_del.at( i ).first -
                          start_tp_del.at( i ).first,
                          end_tp_del.at( i ).second -
                          start_tp_del.at( i ).second );
  }

  _cycle_found = true;
}

template <class SenderType1, class SenderType2>
void CycleFinder<SenderType1, SenderType2>::print_all_statistics( void ) const
{
  if ( not _cycle_found ) {
    cout << "Error: no cycle recorded\n" << endl;
    return;
  }

  for ( size_t i = 0; i < _deltas.size(); i++ ) {
    auto packets_received = _deltas.at( i ).first;
    auto total_delay = _deltas.at( i ).second;
    auto norm_avg_delay = ( total_delay / packets_received ) / _network.delay();
    auto norm_avg_throughput = ( packets_received / _cycle_len ) / _network.link_ppt();
    
    cout << _offset_value << " " << 
      _convergence_time << " " << 
      _cycle_len << " " << 
      norm_avg_delay << " " <<
      norm_avg_throughput << " " <<
      log2( norm_avg_throughput ) - log2( norm_avg_delay ) << " "  << endl;
  }
}
