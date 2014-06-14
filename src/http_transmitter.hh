#ifndef HTTP_TRANSMITTER_HH
#define HTTP_TRANSMITTER_HH

#include <map>
#include <string>
#include <curl/curl.h>

class HttpTransmitter
{
private:
  static size_t callback_helper( void *contents, size_t size, size_t nmemb, void * userp );
  size_t write_memory_callback( void *contents, size_t size, size_t nmemb );
  CURL *curl_;
  CURLcode res_;
  std::string recvbuffer_;

public:
  HttpTransmitter( const std::string & url );
  ~HttpTransmitter();

  /* TODO: Band aid fix, need to dig deeper */
  HttpTransmitter( const HttpTransmitter & other );
  HttpTransmitter & operator=( const HttpTransmitter & other );

  std::string make_get_request( const std::map<std::string, std::string> & headers );
  std::string make_post_request( const std::string & body );

};

#endif // HTTP_TRANSMITTER_HH
