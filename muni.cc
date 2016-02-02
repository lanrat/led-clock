
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


static bool eta_cmp(const arrivalETA& a, const arrivalETA& b)
{
    // smallest comes first
    return a.eta < b.eta;
}

static std::vector<time_t> parseMuniXML(std::string &buffer) {
    xmlDocPtr doc;
    xmlNodePtr cur;
    std::vector<time_t> eta;
    time_t now;
    time(&now);

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
                            eta.push_back(now + atoi((const char *)key));
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
    if (!curlRun(connN_OWL, &bufferN_OWL))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL_N_OWL, httpErrorBuffer);
    }

    auto N = parseMuniXML(bufferN);
    auto NX = parseMuniXML(bufferNX);
    auto N_OWL = parseMuniXML(bufferN_OWL);
    muniETA eta;

    for(auto it = N.begin(); it != N.end(); ++it) {
        eta.push_back((arrivalETA){*it,1});
    }
    for(auto it = NX.begin(); it != NX.end(); ++it) {
        eta.push_back((arrivalETA){*it,2});
    }
    for(auto it = N_OWL.begin(); it != N_OWL.end(); ++it) {
        eta.push_back((arrivalETA){*it,3});
    }

    std::sort(eta.begin(), eta.end(), eta_cmp);

    return eta;
}

void muniCleanup()
{
    xmlCleanupParser();
    curlCleanup(connN);
    curlCleanup(connNX);
    curlCleanup(connN_OWL);
}
