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
    label_and_control_.pack_start( control_ );

    parent.add( label_and_control_ );
  }
};

GTKFader::GTKFader()
{
  thread newthread( [&] () {
      RefPtr<Application> app = Application::create();

      Window window;
      window.set_default_size( 200, 600 );

      LabeledScale link_rate( window, "<b>Link rate</b> (Mbps)", 0.3, 300.1, 0.1,
			      10, link_rate_ );

      window.show_all();

      app->run( window, 0, nullptr );
      quit_ = true;
    } );

  newthread.detach();
}
