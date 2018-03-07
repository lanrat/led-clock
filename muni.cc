
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

/*
static auto URL_N = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=5200";
static auto URL_NX = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=NX&s=5200";
static auto URL_N_OWL = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=N_OWL&s=5200";
*/

/* 38R
 * 38AX
 * 38BX
 */
static auto URL1 = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=38&s=4279";
static auto URL2 = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=38R&s=4279";
static auto URL3 = "http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=38AX&s=4279";
static CURL *conn1;
static CURL *conn2;
static CURL *conn3;


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

    if (!curlInit(conn1, URL1))
    {
        fprintf(stderr, "Connection initializion failed\n");
        exit(1);
    }
    if (!curlInit(conn2, URL2))
    {
        fprintf(stderr, "Connection initializion failed\n");
        exit(1);
    }
    if (!curlInit(conn3, URL3))
    {
        fprintf(stderr, "Connection initializion failed\n");
        exit(1);
    }
}

muniETA muniRun()
{
    // Retrieve content for the URL
    static std::string buffer1;
    if (!curlRun(conn1, &buffer1))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL1, httpErrorBuffer);
    }

    static std::string buffer2;
    if (!curlRun(conn2, &buffer2))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL2, httpErrorBuffer);
    }

    static std::string buffer3;
    if (!curlRun(conn3, &buffer3))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL3, httpErrorBuffer);
    }

    auto eta1 = parseMuniXML(buffer1);
    auto eta2 = parseMuniXML(buffer2);
    auto eta3 = parseMuniXML(buffer3);
    muniETA eta;

    for(auto it = eta1.begin(); it != eta1.end(); ++it) {
        eta.push_back((arrivalETA){*it,1});
    }
    for(auto it = eta2.begin(); it != eta2.end(); ++it) {
        eta.push_back((arrivalETA){*it,2});
    }
    for(auto it = eta3.begin(); it != eta3.end(); ++it) {
        eta.push_back((arrivalETA){*it,3});
    }

    std::sort(eta.begin(), eta.end(), eta_cmp);

    return eta;
}

void muniCleanup()
{
    xmlCleanupParser();
    curlCleanup(conn1);
    curlCleanup(conn2);
    curlCleanup(conn3);
}
