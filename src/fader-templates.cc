#include "fader.hh"

using namespace std;

template <class NetworkType>
void GTKFader::update( NetworkType & network )
{
  unique_lock<mutex> ul( mutex_ );

  for ( unsigned int i = 0; i < network.senders().gang1().count_senders(); i++ ) {
    if ( remy_.get()[ i ] == network.senders().gang1().sender( i ).sending ) {
      continue;
    }

    if ( remy_.get()[ i ] ) {
      network.mutable_senders().mutable_gang1().mutable_sender( i ).switch_on( network.tickno() );
    } else {
      network.mutable_senders().mutable_gang1().mutable_sender( i ).switch_off( network.tickno(), 1 );
    }
  }

  for ( unsigned int i = 0; i < network.senders().gang2().count_senders(); i++ ) {
    if ( aimd_.get()[ i ] == network.senders().gang2().sender( i ).sending ) {
      continue;
    }

    if ( aimd_.get()[ i ] ) {
      network.mutable_senders().mutable_gang2().mutable_sender( i ).switch_on( network.tickno() );
    } else {
      network.mutable_senders().mutable_gang2().mutable_sender( i ).switch_off( network.tickno(), 1 );
    }
  }
}
