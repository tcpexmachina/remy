#include <gtkmm.h>

#include <thread>
#include <string>

#include "fader.hh"

using namespace std;
using namespace Glib;
using namespace Gtk;

class LabeledToggle
{
  CheckButton control_ {};

public:
  LabeledToggle( Container & parent, mutex & the_mutex, const string & text, std::atomic<bool> & variable )
    : control_( text )
  {
    control_.signal_toggled().connect_notify( [&] () {
	unique_lock<mutex> ul( the_mutex );
	variable = control_.get_active();
      } );
    parent.add( control_ );
  };
};

class LabeledScale
{
  VBox label_and_control_ {};
  Label label_ {};
  VScale control_;

public:
  LabeledScale( Container & parent,
		const string & text, const double minval, const double maxval, const double incr,
		const double multiplier, std::atomic<double> & variable )
    : control_( minval, maxval, incr )
  {
    label_.set_markup( text );
    label_.set_padding( 10, 10 );
    control_.set_inverted();
    control_.set_value( variable * multiplier );
    control_.signal_change_value().connect_notify( [&] ( const ScrollType &, const double & val ) {
	variable = min( maxval, max( minval, val ) ) / multiplier;
      } );

    label_and_control_.pack_start( label_, PACK_SHRINK );
    label_and_control_.pack_start( control_, PACK_EXPAND_WIDGET );

    parent.add( label_and_control_ );
  }
};

GTKFader::GTKFader( const unsigned int & num_senders )
  : remy_( new atomic<bool>[ num_senders ] ),
    aimd_( new atomic<bool>[ num_senders ] )
{
  for ( unsigned int i = 0; i < num_senders; i++ ) {
    remy_.get()[ i ] = false;
    aimd_.get()[ i ] = false;
  }

  thread newthread( [&] () {
      RefPtr<Application> app = Application::create();

      Window window;
      window.set_default_size( 200, 400 );

      VBox stack;
      window.add( stack );

      /* AIMD and RemyCC controls */
      deque<LabeledToggle> sender_controls;

      HBox aimd_senders;
      stack.pack_start( aimd_senders, PACK_SHRINK );
      for ( unsigned int i = 0; i < num_senders; i++ ) {
        sender_controls.emplace_back( aimd_senders, mutex_, "AIMD" + to_string(i + 1), aimd_.get()[ i ] );
      }

      HBox remy_senders;
      stack.pack_start( remy_senders, PACK_SHRINK );
      for ( unsigned int i = 0; i < num_senders; i++ ) {
        sender_controls.emplace_back( remy_senders, mutex_, "RemyCC" + to_string(i + 1), remy_.get()[ i ] );
      }

      /* numerical sliders */
      HBox numeric;
      stack.pack_start( numeric );

      LabeledScale link_rate( numeric, "<b>Link rate</b> (Mbps)", 0.3, 300.1, 0.1, 10, link_rate_ );
      LabeledScale rtt( numeric, "<b>RTT</b> (ms)", 5, 500, 5, 1, rtt_ );
      LabeledScale buffer( numeric, "<b>Buffer cap</b> (pkts)", 0, 20000, 1, 1, buffer_size_ );
      LabeledScale speed( numeric, "<b>Speed</b> (%)", 0, 5000, 1, 60 * 100, time_increment_ );
      LabeledScale width( numeric, "<b>Width</b> (s)", 1, 100, 0.1, 1, horizontal_size_ );

      /* scaling buttons */
      HBox buttons;
      stack.pack_start( buttons, PACK_SHRINK, 10 );

      HBox spacer1, spacer2;
      buttons.pack_start( spacer1 );
      buttons.pack_end( spacer2 );

      Button autoscaler( "Scale to packets in flight" );
      autoscaler.signal_clicked().connect_notify( [&] () { autoscale_ = true; } );
      buttons.pack_start( autoscaler, PACK_SHRINK, 10 );

      Button superautoscaler( "Scale to full buffer + BDP" );
      superautoscaler.signal_clicked().connect_notify( [&] () { autoscale_all_ = true; } );
      buttons.pack_start( superautoscaler, PACK_SHRINK, 10 );

      window.show_all();

      app->run( window, 0, nullptr );
      quit_ = true;
    } );

  newthread.detach();
}
