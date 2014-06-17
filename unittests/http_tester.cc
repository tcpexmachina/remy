#include <map>
#include <string>
#include <utility>
#include <curl/curl.h>
#include "http_transmitter.hh"

using namespace std;

int main()
{
  curl_global_init( CURL_GLOBAL_ALL );
  HttpTransmitter tx( "http://www.google.com" );
  auto response = tx.make_get_request( map<string, string>() );
  printf( "GET response from the server is %s\n", get<0>( response ).c_str() );
  response = tx.make_post_request( "loremepsum", map<string, string>() );
  printf( "POST response from the server is %s\n", get<0>( response ).c_str() );
  curl_global_cleanup();
  return 0;
}
