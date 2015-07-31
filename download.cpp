/*
 * Copyright (C) 2015 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.cactus
 */


#include <cstdlib>
#include <curl/curl.h>

#include "download.h"

using namespace std;

// this code is mostly derived from libucrl example
// http://curl.haxx.se/libcurl/c/getinmemory.html

static const int INIT_BUF_SIZE = 1024;

Download::Download()
{
  _buffer.memory = (char*)malloc(INIT_BUF_SIZE);
  _buffer.size = INIT_BUF_SIZE;
}

Download::~Download()
{
  free(_buffer.memory);
}

void Download::init()
{
  curl_global_init(CURL_GLOBAL_ALL);
}

void Download::cleanup()
{
  curl_global_cleanup();
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  Download::MemoryStruct* mem = (Download::MemoryStruct *)userp;
 
  mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
/* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

const char* Download::postRequest(const string& url,
                                  const vector<string>& headers,
                                  const string& postData)
{
  CURL *curl_handle;
  CURLcode res;
  curl_handle = curl_easy_init();

// check errors todo!!!
  
/* specify URL to get */ 
  curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());

/* send all data to this function  */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
/* we pass our 'chunk' struct to the callback function */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&_buffer);

  curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postData.c_str());

  res = curl_easy_perform(curl_handle);
/* Check for errors */ 
  if(res != CURLE_OK)
     fprintf(stderr, "curl_easy_perform() failed: %s\n",
             curl_easy_strerror(res));

  curl_easy_cleanup(curl_handle);

  return NULL;
}
