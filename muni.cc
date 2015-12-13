
#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include "http.h"
#include "muni.h"


static auto URL_N = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=5200";
static auto URL_NX = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=NX&s=5200";
static auto URL_N_OWL = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=N_OWL&s=5200";
static CURL *connN;
static CURL *connNX;
static CURL *connN_OWL;

std::vector<int> parseMuniXML(std::string &buffer) {
    xmlDocPtr doc;
    xmlNodePtr cur;
    std::vector<int> eta;

    doc = xmlParseMemory (buffer.c_str(), buffer.size());
    if (doc == NULL) {
        fprintf(stderr,"Document not parsed successfully. \n");
        return eta;
    }
    
    cur = xmlDocGetRootElement(doc);
    if (cur == NULL) {
        fprintf(stderr,"empty document\n");
        xmlFreeDoc(doc);
        return eta;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "body")) {
        fprintf(stderr,"document of the wrong type, root node != body");
        xmlFreeDoc(doc);
        return eta;
    }

    cur = cur->children;
    xmlChar *key;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"predictions"))){

            xmlNodePtr pred = cur->children;
            while (pred != NULL) {
                if ((!xmlStrcmp(pred->name, (const xmlChar *)"direction"))){

                    xmlNodePtr dir = pred->children;
                    while (dir != NULL) {
                        if ((!xmlStrcmp(dir->name, (const xmlChar *)"prediction"))){

                            key = xmlGetProp(dir, (xmlChar*)"seconds");
                            eta.push_back(time(NULL) + atoi((const char *)key));
                            xmlFree(key);

                        }
                        dir = dir->next;

                    }
                }
                pred = pred->next;

            }
        }
        cur = cur->next;

    }

    xmlFreeDoc(doc);

    sort(eta.begin(), eta.end());

    return eta;
}


void muniInit()
{
    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    if (!curlInit(connN, URL_N))
    {
        fprintf(stderr, "Connection initializion failed\n");
        exit(1);
    }
    if (!curlInit(connNX, URL_NX))
    {
        fprintf(stderr, "Connection initializion failed\n");
        exit(1);
    }
    if (!curlInit(connN_OWL, URL_N_OWL))
    {
        fprintf(stderr, "Connection initializion failed\n");
        exit(1);
    }
}

muniETA muniRun()
{
    // Retrieve content for the URL
    static std::string bufferN;
    if (!curlRun(connN, &bufferN))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL_N, httpErrorBuffer);
    }

    static std::string bufferNX;
    if (!curlRun(connNX, &bufferNX))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL_NX, httpErrorBuffer);
    }

    static std::string bufferN_OWL;
    if (!curlRun(connNX, &bufferN_OWL))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL_N_OWL, httpErrorBuffer);
    }

    muniETA eta;

    auto N_OWL = parseMuniXML(bufferN_OWL);
    auto N = parseMuniXML(bufferN);
    eta.NX = parseMuniXML(bufferNX);

    // merge N and N_OWL
    eta.N.resize(N.size() + N_OWL.size());
    merge(N.begin(), N.end(), N_OWL.begin(), N_OWL.end(), eta.N.begin());

    return eta;
}

void muniCleanup()
{
    xmlCleanupParser();
    curlCleanup(connN);
    curlCleanup(connNX);
    curlCleanup(connN_OWL);
}

/*
int main(int argc, char *argv[])
{
    muniInit();

    // loop this
    muniRun();

    muniCleanup();
    return 0;
}*/