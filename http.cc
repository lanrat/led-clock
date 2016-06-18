#include <cstdio>
#include "http.h"

char httpErrorBuffer[CURL_ERROR_SIZE];

//
//  libcurl write callback function
//
static int curlWriter(char *data, size_t size, size_t nmemb, std::string *writerData)
{
    if (writerData == NULL)
        return 0;

    writerData->append(data, size*nmemb);

    return size * nmemb;
}

//
//  libcurl connection initialization
//
bool curlInit(CURL *&conn, const char *url)
{
    CURLcode code;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    conn = curl_easy_init();

    if (conn == NULL)
    {
        fprintf(stderr, "Failed to create CURL connection\n");
        exit(EXIT_FAILURE);
    }

    code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, httpErrorBuffer);
    if (code != CURLE_OK)
    {
        fprintf(stderr, "Failed to set error buffer [%d]\n", code);

        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_URL, url);
    if (code != CURLE_OK)
    {
        fprintf(stderr, "Failed to set URL [%s]\n", httpErrorBuffer);
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
    if (code != CURLE_OK)
    {
        fprintf(stderr, "Failed to set redirect option [%s]\n", httpErrorBuffer);
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, curlWriter);
    if (code != CURLE_OK)
    {
        fprintf(stderr, "Failed to set writer [%s]\n", httpErrorBuffer);
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_TIMEOUT_MS, 60000L); // 1 minute
    if (code != CURLE_OK)
    {
        fprintf(stderr, "Failed to set timeout [%s]\n", httpErrorBuffer);
        return false;
    }

    return true;
}

bool curlRun(CURL *conn, std::string *buffer)
{
    CURLcode code;
    buffer->clear();

    code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, buffer);
    if (code != CURLE_OK)
    {
        fprintf(stderr, "Failed to set write data [%s]\n", httpErrorBuffer);
        return false;
    }

    code = curl_easy_perform(conn);
    return (code == CURLE_OK);
}

void curlCleanup(CURL *conn)
{
    curl_easy_cleanup(conn);
}

