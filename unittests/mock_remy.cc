#include <map>
#include <string>
#include <utility>
#include <fstream>
#include <streambuf>
#include <curl/curl.h>
#include <fcntl.h>
#include "http_transmitter.hh"
#include "answer.pb.h"
#include "problem.pb.h"

using namespace std;

int main()
{
  curl_global_init(CURL_GLOBAL_ALL);

  // headers
  map<string, string> headers;
  headers[ "Host" ] = string( getenv( "HTTP_HOST" ) );

  // POST problem
  HttpTransmitter tx( "http://localhost:80/problem" );
  std::ifstream pb_file( "test.problem", ios::binary );
  std::string str((std::istreambuf_iterator<char>(pb_file)),
                   std::istreambuf_iterator<char>());
  auto response = tx.make_post_request( str, headers );
  string response_body = get<0>( response );
  printf( "POST respone was %s\n", response_body.c_str() );

  // GET answer
  headers["problemid"] = response_body;
  response = tx.make_get_request( headers );
  response_body = get<0>( response );
  AnswerBuffers::Outcome outcome;
  outcome.ParseFromString( response_body );
  printf( "GET response from the server is %s\n", outcome.DebugString().c_str() );

  curl_global_cleanup();
  return 0;
}
