#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <curl/curl.h>

extern char httpErrorBuffer[CURL_ERROR_SIZE];

bool curlInit(CURL *&conn, const char *url);

bool curlRun(CURL *conn, std::string *buffer);

void curlCleanup(CURL *conn);

#endif