#include <cstring>
#include <cstdlib>
#include <cassert>
#include "http_transmitter.hh"

using namespace std;

HttpTransmitter::HttpTransmitter( const std::string & url )
  : curl_( curl_easy_init() ),
    res_( CURLE_OK ),
    recvbuffer_( { nullptr, 0 } )
{
  curl_easy_setopt( curl_, CURLOPT_URL, url.c_str() );
  curl_easy_setopt( curl_, CURLOPT_WRITEFUNCTION, callback_helper );
  curl_easy_setopt( curl_, CURLOPT_WRITEDATA, this );
}

size_t HttpTransmitter::callback_helper( void *contents, size_t size, size_t nmemb, void * userp )
{
  return static_cast<HttpTransmitter *>( userp )->write_memory_callback( contents, size, nmemb );
}

/* Callback to write into memory, inspired by Daniel Stenberg:
   http://curl.haxx.se/libcurl/c/getinmemory.html */
size_t HttpTransmitter::write_memory_callback( void *contents, size_t size, size_t nmemb )
{
  size_t realsize = size * nmemb;

  /* append to recvbuffer_ string */
  recvbuffer_ = recvbuffer_ + string( static_cast<char*>( contents ), realsize );

  return realsize;
}

pair<string, long> HttpTransmitter::make_get_request( const map<string, string> & headers )
{
  /* Setup recvbuffer_ */
  recvbuffer_ = "";

  /* First, make sure it's a GET request */
  curl_easy_setopt( curl_, CURLOPT_HTTPGET, 1 );

  /* Populate chunk corresponding to header */
  struct curl_slist *chunk = nullptr;
  for ( auto &kv : headers ) {
    auto key = kv.first;
    auto value = kv.second;
    chunk = curl_slist_append( chunk, ( key + ":" +  value ).c_str() );
  }

  res_ = curl_easy_setopt( curl_, CURLOPT_HTTPHEADER, chunk );
  res_ = curl_easy_perform( curl_ );

  /* Check for errors */
  if(res_ != CURLE_OK) {
    fprintf( stderr, "curl_easy_perform() failed: %s\n",
                     curl_easy_strerror( res_ ) );
    exit( -1 );
    return make_pair( "", -1 );
  } else {
    long ret = 0;
    curl_easy_getinfo( curl_, CURLINFO_RESPONSE_CODE, &ret );
    return make_pair( recvbuffer_, ret );
  }

  /* Free up chunks */
  curl_slist_free_all(chunk);
}

pair<string, long> HttpTransmitter::make_post_request( const string & body, const map<string, string> & headers )
{
  /* Setup recvbuffer_ */
  recvbuffer_ = "";

  /* First, make sure it's a POST request */
  curl_easy_setopt( curl_, CURLOPT_POST, 1 );

  /* Set content type to octet stream */
  struct curl_slist *chunk = nullptr;
  chunk = curl_slist_append( chunk, ( "Content-Type:application/octet-stream" ) );
  for ( auto &kv : headers ) {
    auto key = kv.first;
    auto value = kv.second;
    chunk = curl_slist_append( chunk, ( key + ":" +  value ).c_str() );
  }
  res_ = curl_easy_setopt( curl_, CURLOPT_HTTPHEADER, chunk );

  /* POST the actual data */
  curl_easy_setopt( curl_, CURLOPT_POSTFIELDS, body.c_str() );
  curl_easy_setopt( curl_, CURLOPT_POSTFIELDSIZE, body.size() );

  /* Perform the request, res will get the return code */ 
  res_ = curl_easy_perform( curl_ );

  /* Check for errors */
  if(res_ != CURLE_OK) {
    fprintf( stderr, "curl_easy_perform() failed: %s\n",
                     curl_easy_strerror( res_ ) );
    exit( -1 );
    return make_pair( "", -1 );
  } else {
    long ret = 0;
    curl_easy_getinfo( curl_, CURLINFO_RESPONSE_CODE, &ret );
    return make_pair( recvbuffer_, ret );
  }
}

HttpTransmitter::~HttpTransmitter()
{
  curl_easy_cleanup( curl_ );
}
