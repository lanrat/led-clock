
#include <cstdlib>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <iostream>
#include "http.h"
#include "weather.h"

using namespace std;

static auto URL = "http://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20weather.forecast%20where%20woeid%20in%20(select%20woeid%20from%20geo.places(1)%20where%20text%3D%22sanfrancisco%2C%20ca%22)&format=xml&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys";

static CURL *conn;

static int parseWeather(string buffer) {
    const auto doc = xmlParseMemory (buffer.c_str(), buffer.size());
    int code = 255;
    if (doc == NULL) {
        fprintf(stderr,"Document not parsed successfully. \n");
        return -1;
    }

    auto cur = xmlDocGetRootElement(doc)->children->children->children;
    while (xmlStrcmp(cur->name, (const xmlChar*) "item")) {
        cur = cur->next;
    }
    cur = cur->children;
    while (cur != nullptr && xmlStrcmp(cur->name, (const xmlChar*)"condition")) {
        cur = cur->next;
    }
    auto weatherPtr = (char*)xmlGetProp(cur, (const xmlChar*)"code");
    if (weatherPtr != NULL) {
        auto weather = string(weatherPtr);
        code = atoi(weather.c_str());
    }
    
    xmlFree(weatherPtr);
    xmlFreeDoc(doc);
    return code;
}

void weatherInit(){
    if (!curlInit(conn, URL))
    {
        fprintf(stderr, "Weather Connection initializion failed\n");
        exit(1);
    }
}

void weatherCleanup() {
    xmlCleanupParser();
    curlCleanup(conn);
}

int weatherRun(){
    static std::string buffer;
    if (!curlRun(conn, &buffer))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL, httpErrorBuffer);
    }
    const int code = parseWeather(buffer);
    if (code == NOT_AVAILABLE) {
        return 255;
    }
    return code;
    /*
    switch (code) {
        case TORNADO:
        case HURRICANE:
            i = "H";
			break;
        case TROPICAL_STORM:
        case SEVERE_THUNDERSTORMS:
        case THUNDERSHOWERS:
        case ISOLATED_THUNDERSHOWERS:
        case THUNDERSTORMS:
            i = "L";
			break;
        case MIXED_RAIN_AND_SNOW:
        case MIXED_RAIN_AND_SLEET:
        case MIXED_SNOW_AND_SLEET:
        case MIXED_RAIN_AND_HAIL:
            i = "R";
			break;
        case FREEZING_DRIZZLE:
        case DRIZZLE:
            i = "D";
			break;
        case FREEZING_RAIN:
        case SHOWERS:
        case SHOWERS_2:
        case SCATTERED_SHOWERS:
        case SLEET:
            i = "r";
			break;
        case SNOW_FLURRIES:
        case LIGHT_SNOW_SHOWERS:
        case BLOWING_SNOW:
        case HEAVY_SNOW:
        case SCATTERED_SNOW_SHOWERS:
        case HEAVY_SNOW_2:
        case SNOW:
        case SNOW_SHOWERS:
            i = "S";
			break;
        case HAIL:
            i = "R";
			break;
        case DUST:
        case FOGGY:
        case HAZE:
        case SMOKY:
            i = "F";
			break;
        case BLUSTERY:
        case WINDY:
            i = "W";
			break;
        case COLD:
            i = "c";
			break;
        case CLOUDY:
        case PARTLY_CLOUDY:
            i = "C";
			break;
        case MOSTLY_CLOUDY_NIGHT:
        case PARTLY_CLOUDY_NIGHT:
            i = "C";
			break;
        case MOSTLY_CLOUDY_DAY:
        case PARTLY_CLOUDY_DAY:
            i = "C";
			break;
        case SUNNY:
            i = "S";
			break;
        case CLEAR_NIGHT:
        case FAIR_NIGHT:
            i = "f";
			break;
        case FAIR_DAY:
            i = "f";
			break;
        case HOT:
            i = "h";
			break;
        case ISOLATED_THUNDERSTORMS:
        case SCATTERED_THUNDERSTORMS:
        case SCATTERED_THUNDERSTORMS_2:
            i = "L";
			break;
        case NOT_AVAILABLE:
        default:
            i = "?";
			break;
    }*/
}
