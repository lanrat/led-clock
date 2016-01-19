
/*
 * Yahoo Weather Codes
 * https://www.igorkromin.net/fp-content/images/photoframe/conditions.png
 */
#define TORNADO 0
#define TROPICAL_STORM 1
#define HURRICANE 2
#define SEVERE_THUNDERSTORMS 3
#define THUNDERSTORMS 4
#define MIXED_RAIN_AND_SNOW 5
#define MIXED_RAIN_AND_SLEET 6
#define MIXED_SNOW_AND_SLEET 7
#define FREEZING_DRIZZLE 8
#define DRIZZLE 9
#define FREEZING_RAIN 10
#define SHOWERS 11
#define SHOWERS_2 12
#define SNOW_FLURRIES 13
#define LIGHT_SNOW_SHOWERS 14
#define BLOWING_SNOW 15
#define SNOW 16
#define HAIL 17
#define SLEET 18
#define DUST 19
#define FOGGY 20
#define HAZE 21
#define SMOKY 22
#define BLUSTERY 23
#define WINDY 24
#define COLD 25
#define CLOUDY 26
#define MOSTLY_CLOUDY_NIGHT 27
#define MOSTLY_CLOUDY_DAY 28
#define PARTLY_CLOUDY_NIGHT 29
#define PARTLY_CLOUDY_DAY 30
#define CLEAR_NIGHT 31
#define SUNNY 32
#define FAIR_NIGHT 33
#define FAIR_DAY 34
#define MIXED_RAIN_AND_HAIL 35
#define HOT 36
#define ISOLATED_THUNDERSTORMS 37
#define SCATTERED_THUNDERSTORMS 38
#define SCATTERED_THUNDERSTORMS_2 39
#define SCATTERED_SHOWERS 40
#define HEAVY_SNOW 41
#define SCATTERED_SNOW_SHOWERS 42
#define HEAVY_SNOW_2 43
#define PARTLY_CLOUDY 44
#define THUNDERSHOWERS 45
#define SNOW_SHOWERS 46
#define ISOLATED_THUNDERSHOWERS 47
#define NOT_AVAILABLE 3200

typedef char weather_icon[8];


int weatherRun();

void weatherInit();


void weatherCleanup();
