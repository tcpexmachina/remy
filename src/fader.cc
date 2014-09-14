#include <gtkmm.h>

#include <thread>

#include "fader.hh"

using namespace std;
using namespace Glib;
using namespace Gtk;

GTKFader::GTKFader()
{
  thread newthread( [&] () {
      int argc = 1;
      char argv0[] = "Controller";
      char *argv[] = { argv0 };
      char **argv_ptr = argv;

      RefPtr<Application> app = Application::create( argc, argv_ptr );

      Window window;
      window.set_default_size( 200, 600 );

      VScale scaler( 0.3, 300.1, .1 );
      scaler.set_inverted();
      scaler.set_value( link_rate_ );
      scaler.set_value_pos( POS_LEFT );
      window.add( scaler );

      window.show_all();
      app->run( window );
    } );

  newthread.detach();
}
