#include <gtkmm.h>

#include <thread>
#include <string>

#include "fader.hh"

using namespace std;
using namespace Glib;
using namespace Gtk;



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

GTKFader::GTKFader()
{
  thread newthread( [&] () {
      RefPtr<Application> app = Application::create();

      Window window;
      window.set_default_size( 200, 400 );

      VBox stack;
      window.add( stack );

      /* numerical sliders */
      HBox numeric;
      stack.pack_start( numeric );

      LabeledScale speed( numeric, "<b>Speed</b> (%)", 0, 1000, 1, 60 * 100, time_increment_ );
      LabeledScale link_rate( numeric, "<b>Link rate</b> (Mbps)", 0.3, 300.1, 0.1, 10, link_rate_ );
      LabeledScale width( numeric, "<b>Width</b> (s)", 1, 100, 0.1, 1, horizontal_size_ );
      LabeledScale buffer( numeric, "<b>Buffer size</b> (pkts)", 0, 20000, 1, 1, buffer_size_ );

      /* scaling buttons */
      HBox buttons;
      stack.pack_start( buttons, PACK_SHRINK, 10 );

      HBox spacer1, spacer2;
      buttons.pack_start( spacer1 );
      buttons.pack_end( spacer2 );

      Button autoscaler( "Scale to senders" );
      autoscaler.signal_clicked().connect_notify( [&] () { autoscale_ = true; } );
      buttons.pack_start( autoscaler, PACK_SHRINK, 10 );

      Button superautoscaler( "Scale to buffer" );
      superautoscaler.signal_clicked().connect_notify( [&] () { autoscale_all_ = true; } );
      buttons.pack_start( superautoscaler, PACK_SHRINK, 10 );

      window.show_all();

      app->run( window, 0, nullptr );
      quit_ = true;
    } );

  newthread.detach();
}
