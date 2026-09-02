#ifndef TZ_H
#define TZ_H

typedef struct {
    //const char* iana_id;
   const char* city;
   const char* posix_str;
   
} Timezones;

//array of structures 
const Timezones tz_list[] = {
    {"cairo", "EET-2EEST,M4.5.5/0,M10.5.5/0"},
    {"casablanca", "WET0WEST,M3.5.0/0,M10.5.0/0"},
    {"johannesburg", "SAST-2"},
    {"anchorage", "AKST9AKDT,M3.2.0,M11.1.0"},
    {"buenos_aires", "ART3"},
    {"chicago", "CST6CDT,M3.2.0,M11.1.0"},
    {"denver", "MST7MDT,M3.2.0,M11.1.0"},
    {"guatemala", "CST6"},
    {"halifax", "AST4ADT,M3.2.0,M11.1.0"},
    {"los_angeles", "PST8PDT,M3.2.0,M11.1.0"},
    {"mexico_city", "CST6"},
    {"new_york", "EST5EDT,M3.2.0,M11.1.0"},
    {"phoenix", "MST7"},
    {"santiago", "CLT4CLST,M9.1.6/24,M4.1.6/24"},
    {"sao_paulo", "BRT3"},
    {"st_johns", "NST3:30NDT,M3.2.0,M11.1.0"},
    {"toronto", "EST5EDT,M3.2.0,M11.1.0"},
    {"vancouver", "PST8PDT,M3.2.0,M11.1.0"},
    {"almaty", "ALMT-6"},
    {"amman", "EET-3"},
    {"baghdad", "AST-3"},
    {"bangkok", "ICT-7"},
    {"beirut", "EET-2EEST,M3.5.0/0,M10.5.0/0"},
    {"dhaka", "BDT-6"},
    {"dubai", "GST-4"},
    {"ho_chi_minh", "ICT-7"},
    {"hong_kong", "HKT-8"},
    {"jakarta", "WIB-7"},
    {"jerusalem", "IST-2IDT,M3.4.4/26,M10.5.0"},
    {"karachi", "PKT-5"},
    {"kathmandu", "NPT-5:45"},
    {"kolkata", "IST-5:30"},
    {"kuala_lumpur", "MYT-8"},
    {"manila", "PST-8"},
    {"seoul", "KST-9"},
    {"shanghai", "CST-8"},
    {"singapore", "SGT-8"},
    {"taipei", "CST-8"},
    {"tashkent", "UZT-5"},
    {"tehran", "IRST-3:30"},
    {"tokyo", "JST-9"},
    {"ulaanbaatar", "ULAT-8"},
    {"yekaterinburg", "YEKT-5"},
    {"azores", "AZOT1AZOST,M3.5.0/0,M10.5.0/0"},
    {"canary", "WET0WEST,M3.5.0/1,M10.5.0"},
    {"reykjavik", "GMT0"},
    {"adelaide", "ACST-9:30ACDT,M10.1.0,M4.1.0"},
    {"brisbane", "AEST-10"},
    {"darwin", "ACST-9:30"},
    {"hobart", "AEST-10AEDT,M10.1.0,M4.1.0"},
    {"melbourne", "AEST-10AEDT,M10.1.0,M4.1.0"},
    {"perth", "AWST-8"},
    {"sydney", "AEST-10AEDT,M10.1.0,M4.1.0"},
    {"amsterdam", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"athens", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    {"belgrade", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"berlin", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"brussels", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"bucharest", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    {"copenhagen", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"dublin", "GMT0IST,M3.5.0/1,M10.5.0"},
    {"helsinki", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    {"istanbul", "TRT-3"},
    {"kiev", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    {"lisbon", "WET0WEST,M3.5.0/0,M10.5.0/0"},
    {"london", "GMT0BST,M3.5.0/1,M10.5.0"},
    {"madrid", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"moscow", "MSK-3"},
    {"oslo", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"paris", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"prague", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"rome", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"stockholm", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"warsaw", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"auckland", "NZST-12NZDT,M9.5.0,M4.1.0"},
    {"chatham", "CHAST-12:45CHADT,M9.5.0,M4.1.0/3"},
    {"fiji", "FJT-12"},
    {"guam", "ChST-10"},
    {"honolulu", "HST10"},
    {"port_moresby", "PGT-10"},
    {"tahiti", "TAHT10"},
    {"utc", "UTC0"}
};

#define TZCOUNT sizeof(tz_list)/sizeof(tz_list[0])

//LOOKUP funct - inline from c++ is safer than macro from C
inline String posixFinder(String cityName){
    //byte matchVar; //used for debug
    for(byte i=0; i < TZCOUNT; i++){
        //matchVar = cityName==tz_list[i].city;     //used for debug

        if(cityName==tz_list[i].city){  
            //Serial.printf("%d @ %s\n" , matchVar, tz_list[i].city); //used for debug
            return tz_list[i].posix_str;
        }   
    }
    return "posix_fail";
}


#endif