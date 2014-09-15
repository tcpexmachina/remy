#include <gtkmm.h>

#include <thread>

#include "fader.hh"

using namespace std;
using namespace Glib;
using namespace Gtk;

GTKFader::GTKFader()
{
  thread newthread( [&] () {
      RefPtr<Application> app = Application::create();

      Window window;
      window.set_default_size( 200, 600 );

      VBox link_speed_control;

      Label link_speed_label;
      link_speed_label.set_markup( "<b>Link speed (Mbps)</b>" );
      link_speed_label.set_padding( 10, 10 );

      VScale link_speed_scaler( .3, 300.1, .1 );
      link_speed_scaler.set_inverted();
      link_speed_scaler.set_value( link_rate_ * 10 );
      link_speed_scaler.signal_change_value().connect_notify( [&] ( const ScrollType &, const double & val ) {
	  link_rate_ = max( val, link_speed_scaler.get_adjustment()->get_lower() ) / 10.0;
	} );

      link_speed_control.pack_start( link_speed_label, PACK_SHRINK );
      link_speed_control.pack_start( link_speed_scaler );

      window.add( link_speed_control );

      window.show_all();

      return app->run( window, 0, nullptr );
    } );

  newthread.detach();
}
