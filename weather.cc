#include "http.h"
#include <cstdlib>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <iostream>

using namespace std;

static auto URL = "http://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20weather.forecast%20where%20woeid%20in%20(select%20woeid%20from%20geo.places(1)%20where%20text%3D%22sanfrancisco%2C%20ca%22)&format=xml&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys";

static CURL *conn;

static int parseWeather(string buffer) {
    const auto doc = xmlParseMemory (buffer.c_str(), buffer.size());
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
    auto weather = string(weatherPtr);
    xmlFree(weatherPtr);
    return atoi(weather.c_str());
}

void weatherInit(){
    if (!curlInit(conn, URL))
    {
        fprintf(stderr, "Weather Connection initializion failed\n");
        exit(1);
    }
}

string weatherRun(){
    static std::string buffer;
    /*
    const int TORNADO = 0;
    const int TROPICAL_STORM = 1;
    const int HURRICANE = 2;
    const int SEVERE_THUNDERSTORMS = 3;
    const int THUNDERSTORMS = 4;
    const int MIXED_RAIN_AND_SNOW = 5;
    const int MIXED_RAIN_AND_SLEET = 6;
    const int MIXED_SNOW_AND_SLEET = 7;
    const int FREEZING_DRIZZLE = 8;
    const int DRIZZLE = 9;
    const int FREEZING_RAIN = 10;
    const int SHOWERS = 11;
    const int SHOWERS_2 = 12;
    const int SNOW_FLURRIES = 13;
    const int LIGHT_SNOW_SHOWERS = 14;
    const int BLOWING_SNOW = 15;
    const int SNOW = 16;
    const int HAIL = 17;
    const int SLEET = 18;
    const int DUST = 19;
    const int FOGGY = 20;
    const int HAZE = 21;
    const int SMOKY = 22;
    const int BLUSTERY = 23;
    const int WINDY = 24;
    const int COLD = 25;
    const int CLOUDY = 26;
    const int MOSTLY_CLOUDY_NIGHT = 27;
    const int MOSTLY_CLOUDY_DAY = 28;
    const int PARTLY_CLOUDY_NIGHT = 29;
    const int PARTLY_CLOUDY_DAY = 30;
    const int CLEAR_NIGHT = 31;
    const int SUNNY = 32;
    const int FAIR_NIGHT = 33;
    const int FAIR_DAY = 34;
    const int MIXED_RAIN_AND_HAIL = 35;
    const int HOT = 36;
    const int ISOLATED_THUNDERSTORMS = 37;
    const int SCATTERED_THUNDERSTORMS = 38;
    const int SCATTERED_THUNDERSTORMS_2 = 39;
    const int SCATTERED_SHOWERS = 40;
    const int HEAVY_SNOW = 41;
    const int SCATTERED_SNOW_SHOWERS = 42;
    const int HEAVY_SNOW_2 = 43;
    const int PARTLY_CLOUDY = 44;
    const int THUNDERSHOWERS = 45;
    const int SNOW_SHOWERS = 46;
    const int ISOLATED_THUNDERSHOWERS = 47;
    const int NOT_AVAILABLE = 3200;
    */
    if (!curlRun(conn, &buffer))
    {
        fprintf(stderr, "Failed to get '%s' [%s]\n", URL, httpErrorBuffer);
    }
    const int code = parseWeather(buffer);
    if (code <= 18 || (code >= 37 && code <= 43) || (code >= 45 && code <= 47)) { return "R"; }
    else if ((code >= 26 && code <= 30) || code == 44 ) { return "C"; }
    else if (code == 20) { return "F"; }
    else if (code == 23 || code == 24) { return "W"; }
    else if (code == 32) { return "S"; }
    else if (code == 33 || code == 34) { return "OK"; }
    else if (code == 36) { return "H"; }
    else { return to_string(code); } 
}
