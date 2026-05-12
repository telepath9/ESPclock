#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>
#include <FS.h>            //for file handling
#include <LittleFS.h>      //to access to filesystem
#include <TM1637Display.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "esp_sntp.h"

//JSON optimizations
#define ARDUINOJSON_SLOT_ID_SIZE 1
#define ARDUINOJSON_STRING_LENGTH_SIZE 1
#define ARDUINOJSON_USE_DOUBLE 0
#define ARDUINOJSON_USE_LONG_LONG 0

//NON-blocking timer function (delay() is EVIL). only accepts milliseconds
unsigned long myTimer(unsigned long everywhen){ //millis overflow-safe!
        static unsigned long t1, diff_time;
        bool ret=0;
        diff_time= millis() - t1;
          
        if(diff_time >= everywhen){
            t1= millis();
            ret=1;
        }
        return ret;
}
//4294967295 ms == 49d 17h 5m 

const char* ssid;
const char* password;
bool creds_available=false;
bool connected=false;   //wifi connection state

const char *esp_ssid = "ESPclock32";
const char *esp_password =  "waltwhite64"; //AP pw must be at least 8 chars, otherwise AP won't be customized 

bool newScan = false; //if true, ESP scans for networks again and overrides the previous networks on net_list
uint8_t attempts = 0; //connection attempts --> when it's set to 0 again, it means pw is wrong

AsyncWebServer server(80);

#define BUZZER_PIN 5

//TM1637 DISPLAY SETUP
#define CLK 9  //previously gpio2
#define DIO 10 //previously gpio3

TM1637Display mydisplay(CLK, DIO);
bool colon=true;
bool blink=true;
bool br_auto=false;
bool twelve=false;
bool leadingzero=false;
uint8_t brightness=7;
uint8_t ms_ovfl=0;

bool alarm_status=false; 
String timehm = "";
bool alarm_stop=0;  //when 0, alarm is active.
uint8_t alarm_hour;
uint8_t alarm_min;

const uint8_t SEG_try[]={
  SEG_D | SEG_E | SEG_F | SEG_G,  //t
  SEG_E | SEG_G,                  //r
  SEG_B | SEG_C | SEG_D | SEG_F | SEG_G  //Y
};

const uint8_t SEG_Err[]={
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, //E
  SEG_E | SEG_G, SEG_E | SEG_G          //rr
};

uint8_t px=4;                   //used by displayAnim()
const uint8_t SEG_WAIT[] = {     //used by displayAnim()
	 SEG_G
};

bool forw = true;               //used by displayAnim()

void displayAnim(void){
   if(myTimer(500)){
        if(forw==true){ // 4 -> 0
            mydisplay.clear();
            mydisplay.setSegments(SEG_WAIT, 1, px); 
            --px;          

            if(px==0){
              forw=false;
            }
        }

        else if(forw==false){ //0 -> 4
            mydisplay.clear();
            mydisplay.setSegments(SEG_WAIT, 1, px); 
            ++px;          

            if(px==3){
              forw= true;
            }
        }
  }
  return;
}

//NTP SETUP
struct tm timeinfo;

/*void printLocalTime(){
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time 1");
      return;
    }
    Serial.println(&timeinfo, "%H:%M:%S zone %Z %z ");
    //Serial.println(timeinfo.tm_hour); //access to single time vars
}*/
const char *ntp_addr;
int gmt_offset;
bool start_NtpClient = false;

//ALARM setup
uint8_t hh, mm; //hour and minutes

//all entries are initialized to 0
bool days[7] = {0};   //in this case sun=days[0], mon=days[1], tue=days[2], ...
bool tone_var=0;
uint8_t snooze;
//uint8_t ringtone;

void wifiScan(){
    //---------------------------------------------x
    //start wifiSCAN
    WiFi.disconnect();

    byte n = WiFi.scanNetworks();
    Serial.print(n);
    Serial.println(" networks found");

    //---------------------------------------------x
    //SSIDs found are stored in json
    //arduinoJson7 doesn't use static/dynamicJsonDocument anymore, but it uses only JsonDocument
      
    //If json doesn't exists yet, it creates it
    if(!LittleFS.exists("/network_list.json")){
      JsonDocument net_list;
      //Serial.println("Network list doesn't exists. Creating it now...");  
      net_list["found"] = n;
      JsonArray network = net_list["network"].to<JsonArray>();

      for(byte j = 0; j < n+1; j++){
        JsonArray network_n_credentials = network[j]["credentials"].to<JsonArray>();
        network_n_credentials.add(WiFi.SSID(j));
        network_n_credentials.add("");
      }
      
      //---------------------------------------------x
      //After creating JSON file (jsondocument) it stores it in FS
      File fx = LittleFS.open("/network_list.json", "w");

      //serializes json and passes it to "fx" var
      serializeJsonPretty(net_list, fx);
      fx.close();
    }


    //---------EXISTING JSON---------------------
    //2. IF JSON ALREADY EXISTS: access to json, reset it, then add new networks to it
    else{
      //Serial.println("Network list already exists! Updating it..."); 
      JsonDocument net_listUp;
     
      //1. fetch and open json from FS, then deserializes it
      File fxup = LittleFS.open("/network_list.json", "w+");
      deserializeJson(net_listUp, fxup);

      net_listUp["found"] = n;
      JsonArray network = net_listUp["network"].to<JsonArray>();
            
      for(byte k = 0; k < n+1; k++){
        //dynamically adds, to each entry "k", a new array to the main array "network"
        JsonArray network_n_credentials = network[k]["credentials"].to<JsonArray>();

        //adds SSID name to the entry[k][0]
        network_n_credentials.add(WiFi.SSID(k));

        //adds pw field (initially empty) to entry[k][1] 
        network_n_credentials.add("");
      }
      
      //3. serializing
      serializeJsonPretty(net_listUp, fxup);
      fxup.close();
    }     
}

void checkConfig(void){

    if(LittleFS.exists("/config.json")){
      Serial.println("Restoring data");
      creds_available=true;

      File fld = LittleFS.open("/config.json", "r");
      JsonDocument load_cf;
      
      DeserializationError error = deserializeJson(load_cf, fld);

      if (error) {
        fld.close();
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return;
      }

      ssid = load_cf[F("ssid")];
      password = load_cf[F("pw")]; 
      
      WiFi.begin(ssid, password);

      //if it restores wifi connection, then it restore the other settings too
      //if it can't restore wifi, then user must go to webUI to make a new config
      while(WiFi.status() != WL_CONNECTED){
        delay(100);
        mydisplay.setSegments(SEG_try, 3, 0);
       
        if(myTimer(3000)){  
          ++attempts;
          mydisplay.showNumberDec(attempts, true, 1, 3);
        }

        else if(attempts==4){
          attempts=0;
          creds_available = false;
          fld.close(); 
          break;
        }
      }

      if(WiFi.status() == WL_CONNECTED){
        attempts=0;
        connected=true;
        Serial.println("WIFI RESTORED");

        start_NtpClient=true;
        ntp_addr= strdup(load_cf["ntp_ad"]); 
        gmt_offset = load_cf["offset"]; 
        configTime(gmt_offset*3600, 3600, ntp_addr);
        //Serial.println("NTP server: " + String(ntp_addr));
        //Serial.println("OFFSET: " + String(gmt_offset));  
  
        brightness = (uint8_t)load_cf["br"];
        mydisplay.setBrightness(brightness);
        blink=  load_cf[F("blink")];
        br_auto = load_cf[F("br_auto")];
        twelve= load_cf[F("twelve")];
        fld.close();
      }
  }
  return;
}

void checkAlarm(){
  if(LittleFS.exists("/alarm.json")){

    File fla = LittleFS.open("/alarm.json", "r");
    JsonDocument load_al;
    DeserializationError error = deserializeJson(load_al, fla);

    if (error) {
      fla.close();
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    alarm_status= load_al[F("alarm")];

    //if alarm is set, edit JSON in order to update UI
    if(alarm_status==true){

      timehm= strdup(load_al[F("timehm")]);
      Serial.println("timehm > " + timehm);

      alarm_hour= load_al[F("alarm_hour")];
      alarm_min= load_al[F("alarm_min")];
      snooze= load_al[F("snooze")];

      String week= strdup(load_al[F("week")]);
      fla.close();
   
      for(uint8_t n=0; n<7; n++){
        (week.charAt(n)=='1') ? (days[n]= 1) : (days[n] = 0);
      }
    }
  }
}

bool snoozeOn=0;
bool snoozeRing=0;
bool snoozeMsStart=0;
unsigned long snoozeTimer;

void alarm_ring(){
  if(tone_var){
    tone(BUZZER_PIN, 2000, 900);
    tone_var=0;
  }

  else{
    tone(BUZZER_PIN, 0, 500);
    //noTone(BUZZER_PIN);
    tone_var=1;
  }
}

//TTP223 DEBOUNCE
unsigned long prevMillis = 0;     // Previous millis
unsigned long elapsedMillis = 0;      // Elapesed millis since touch
unsigned long debounceTime = 800;              // Debounce time for the touch sensor

void alarm_off(){
  elapsedMillis = millis() - prevMillis;
  if(alarm_stop==0 && elapsedMillis > debounceTime){
    alarm_stop=1;
    snoozeOn=0;
    Serial.println("Alarm OFF");
  }
  prevMillis = millis();
  snoozeMsStart=0;
}

//this is called when user requests resources from esp webserver that don't exists
void notFound(AsyncWebServerRequest *request){
    request->send(404, "text/plain", "NOT FOUND");
}

void initMDNS(){
   MDNS.end();

  if (MDNS.begin("espclock")) {
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("mDNS fail");
  }
}

void setup() {
  Serial.begin(115200);
  sntp_set_sync_interval(21600000UL); //time syncs with ntp servers every 6 hours
  
  mydisplay.setBrightness(7); 
  mydisplay.clear();
  pinMode(BUZZER_PIN, OUTPUT); 

  pinMode(4, INPUT_PULLUP);   //TTP223 Touch button
  attachInterrupt(digitalPinToInterrupt(3), alarm_off, RISING);

 //LittleFS.format();

  //Begin LittleFS can throw Err0
  if(!LittleFS.begin()){
    mydisplay.setSegments(SEG_Err, 3, 0);
    mydisplay.showNumberDec(0, false, 1, 3);
    //Serial.println("An Error has occurred while mounting LittleFS");
    delay(10000);
    return;
  }
  
  //can throw Err1
  if(!LittleFS.exists("/index.html")){
    mydisplay.setSegments(SEG_Err, 3, 0);
    mydisplay.showNumberDec(1, false, 1, 3);
    //Serial.println("\nSetup Html page NOT FOUND!");
    delay(10000);
    return;
  }
  
  checkConfig();
  checkAlarm();   

  //PHASE1 - AP_STA_MODE + WIFI SCAN ---------------------------x
  //here scans for networks, and as already said, networks are then stored in json
  WiFi.mode(WIFI_AP_STA);   
  WiFi.setAutoReconnect(true);
  initMDNS();
  delay(100);

  if(WiFi.status() != WL_CONNECTED){
    wifiScan();
  }
  
  //PHASE 2: here user choose its ssid and enters pw ---------------------------x
  WiFi.softAP(esp_ssid, esp_password, false, 2);     //Starting AP on given credential

  //Route for root index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send(LittleFS, "/index.html", "text/html" ); 
    Serial.println("Device detected");
  });

  //this is triggered when entering to the webUI after the clock is set. It checks the status of all of the UI elements and updates it
  server.on("/uicheck", HTTP_GET, [](AsyncWebServerRequest *request){
       
    JsonDocument uc_json;

    uc_json["conn"] = connected;
    uc_json["bright"]= brightness; 
    uc_json["br_auto"] = br_auto;
    uc_json["blink"]= blink;
    uc_json["twelve"]= twelve;
    uc_json["config"]= (LittleFS.exists("/config.json")) ? 1 : 0;
    uc_json["millis"]= millis();
    uc_json["msovfl"]= ms_ovfl;
    uc_json["lz"]= leadingzero;
    String uc_str;
    serializeJson(uc_json, uc_str);

    request->send(200,  "application/json", uc_str);
  });

  //client requests list of ssids and server sends it to client
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {

    File f = LittleFS.open("/network_list.json", "r");

    //checks json integrity
    if(!f) {
      //Serial.println("Error opening /network_list.json");
      request->send(500, "application/json", "{\"error\":\"Can't open network_list.json\"}");
      f.close();
    }

    else{
      request->send(LittleFS,  "/network_list.json", "application/json");
      newScan = true;
      f.close();
    }
  });

  //client(JS) sends http POST req with wifi credentials (inside the body) to server
  server.on("/sendcreds", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
           
    //deserializes http POST req body (has creds inside) from client
    JsonDocument thebody;
    deserializeJson(thebody, data);

    const char* user_ssid_str = thebody["ssid"];  
    const char* user_pw = thebody["pw"];

    ssid = strdup(user_ssid_str);
    password = strdup(user_pw);
    creds_available = true;
    
    request->send(200, "application/json", "{\"creds\":\"OK\"}");
  }); 

  //refresh SSID list on frontend
  server.on("/refresh", HTTP_GET, [](AsyncWebServerRequest *request) {
           
    File f = LittleFS.open("/network_list.json", "r");

    //check json integrity
    if(!f) {
      Serial.println(F("Error opening /network_list.json"));
      request->send(500, "application/json", "{\"error\":\"Failed to open network_list.json\"}");
      f.close();
      return;
    }

    else{
      request->send(LittleFS,  "/network_list.json", "application/json");
      f.close();
      newScan =true;
    }
  });

  //HTTP GET req from client, in order to know if connection attempt was successful
  server.on("/wifi_status", HTTP_GET, [](AsyncWebServerRequest *request) {
          
    if(attempts == 4){
      creds_available = false;
      //Serial.println(password);
      Serial.println("handler: 5 attempts-WRONG PASSWORD- RESET attempts to 0");
      request->send(200, "application/json", "{\"stat\":\"fail\"}");
    }

    else{
      ++attempts;
      if(WiFi.status() == WL_CONNECTED){
        request->send(200, "application/json", "{\"stat\":\"ok\"}");
      }

      else{
        Serial.println("PLEASE WAIT");
        request->send(200, "application/json", "{\"stat\":\"wait\"}");
      }
    }
  });

  //"empty input" check: [OK]
  server.on("/updatetime", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

    JsonDocument ntp_json;
    deserializeJson(ntp_json, data);
    String ntp_str_test = strdup(ntp_json["ntp_addr"]);//used this cuz ntp_addr is const char* and i don't want to change it to String type
    ntp_str_test.trim();

    if(ntp_str_test=="" || ntp_json["offset"]==""){  //beware of multiple whitespaces though (e.g. "    ")
      Serial.println("NTP address or Offset==NULL");
      request->send(200, "application/json", "{\"ntp\":\"FAIL\"}");
      return;
    }

    //POST/GET values are never null. The best they can be is an empty string, which you can convert to null/'NULL'.
    else{
    ntp_addr = strdup(ntp_json["ntp_addr"]); 
    gmt_offset = (int)atoi(ntp_json["offset"]);
    configTime(gmt_offset*3600, 3600, ntp_addr); 
    //Serial.println("NTP server: " + String(ntp_addr));
    //Serial.println("OFFSET: " + String(gmt_offset));
    if(start_NtpClient == false){
      start_NtpClient=true;
    }
    
    request->send(200, "application/json", "{\"ntp\":\"OK\"}");
    }
  });


  server.on("/slider", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
          
          JsonDocument bgt_json;
          deserializeJson(bgt_json, data);

          brightness =(uint8_t)atoi(bgt_json["bgt"]); //extract brightness value
          mydisplay.setBrightness(brightness); 
          request->send(200, "application/json", "{\"status\":\"BGT OK\"}");
  });

  
  server.on("/br_auto", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

            JsonDocument br_auto_json;
            deserializeJson(br_auto_json, data);
            br_auto = br_auto_json["br"];
            /*to get single time vars 
            Serial.println("Time variables");
            char timeHour[3];
            strftime(timeHour,3, "%H", &timeinfo); https://cplusplus.com/reference/ctime/strftime/*/
            
            //optimization: should i replace it with a switch-case (?)
            if(timeinfo.tm_hour >= 0 && timeinfo.tm_hour < 9){
              brightness=0;
              mydisplay.setBrightness(0);
              request->send(200, "application/json", "{\"status\":\"0\"}");
            }

            else if(timeinfo.tm_hour >= 7 && timeinfo.tm_hour < 17){
              brightness=6;
              mydisplay.setBrightness(6);
              request->send(200, "application/json", "{\"status\":\"6\"}");
            }

            else if(timeinfo.tm_hour >= 17 && timeinfo.tm_hour < 20){
              brightness=3;
              mydisplay.setBrightness(3);
              request->send(200, "application/json", "{\"status\":\"3\"}");
            }

            else if(timeinfo.tm_hour >= 20){
              brightness=2;
              mydisplay.setBrightness(2);
              request->send(200, "application/json", "{\"status\":\"2\"}");
            }
  });

  server.on("/blink", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

    JsonDocument blink_json;
    deserializeJson(blink_json, data);
    blink = (uint8_t)blink_json["bl"];  //update blink var
    request->send(200, "application/json", "{\"status\":\"updated\"}");
  });

  server.on("/twelve", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

    JsonDocument twelve_json;
    deserializeJson(twelve_json, data);
    twelve = (uint8_t)twelve_json["tw"];  //update blink var
    request->send(200, "application/json", "{\"status\":\"updated\"}");
  });

  server.on("/lead0", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            JsonDocument leadz_json;
            deserializeJson(leadz_json, data);
            leadingzero = (uint8_t)leadz_json["lz"];  //update leading zero var
            request->send(200, "application/json", "{\"status\":\"updated\"}");
  });

  /*this saves alarm data on memory and in json too*/
  server.on("/alarm", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    
    JsonDocument doc;
    deserializeJson(doc, data);
    alarm_status= doc["set"]; //alarm status ==1 if there's an alarm saved
    
    //user wants to save alarm
    if(!LittleFS.exists("/alarm.json") && alarm_status==1){      
      if(doc["timehm"]==""){    
        Serial.println("Alarm Time value is NULL");
        request->send(200, "application/json", "{\"alarm\":\"FAIL\"}");
        return;
      }

      else{

        timehm= strdup(doc["timehm"]);
        alarm_hour= (uint8_t)timehm.substring(0, 2).toInt(); //e.g. time is 12:45 --> extracts "12"
        alarm_min= (uint8_t)timehm.substring(3, 5).toInt(); //--> extracts "45"
        
        days[0]= (uint8_t)doc["sun"];
        days[1]= (uint8_t)doc["mon"];
        days[2]= (uint8_t)doc["tue"];
        days[3]= (uint8_t)doc["wed"];
        days[4]= (uint8_t)doc["thu"];
        days[5]= (uint8_t)doc["fri"];
        days[6]= (uint8_t)doc["sat"];
        snooze= (uint8_t)doc["snooze"];

        JsonDocument alarmjson; 
        alarmjson[F("alarm")] = alarm_status;  
        alarmjson[F("timehm")]= timehm; //sends timehm with the format "hh:mm" 
        alarmjson[F("alarm_hour")] = alarm_hour; 
        alarmjson[F("alarm_min")] = alarm_min;
        alarmjson[F("snooze")] = snooze;

        String wwd= "";
        for(uint8_t z=0; z<7; z++){
          wwd+= days[z];
        }

        alarmjson[F("week")] = wwd;
        alarmjson.shrinkToFit();
        File fa = LittleFS.open("/alarm.json", "w+");   //creates alarm.json file

        //serializes json and passes it to "fc" var, in order to store it in FS 
        serializeJsonPretty(alarmjson, fa);
        fa.close();
        Serial.println("\nALARM SAVED");
      }
    }
    
    else if(LittleFS.exists("/alarm.json") && alarm_status==0){
      LittleFS.remove("/alarm.json");
      for(uint8_t i=0; i<7; i++){
        days[i]= 0; 
      }

      snooze=0;
      Serial.println("Alarm deleted");
    }

    request->send(200, "application/json", "{\"alarm\":\"updated\"}");
  });

  server.on("/alcheck", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument al_json;   //creates a json to send to client

    al_json["alarm"]= alarm_status;
    Serial.println(timehm);
    al_json["timehm"]= timehm;
    al_json["snooze"]= snooze;
    String sk="";
    
    for(uint8_t j=0; j<7; j++){
       sk+= String(days[j]);
    }

    al_json["week"]=sk;
    //Serial.println(timehm);
    //Serial.println("alcheck STRING > " + sk);
    //Serial.println("Snooze > " + String(snooze));

    String al_str;
    serializeJson(al_json, al_str);

    request->send(200,  "application/json", al_str);
  });

  /*
  server.on("/timer", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  });*/

  server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

    JsonDocument scf_json;
    deserializeJson(scf_json, data);
    bool saveconfig = scf_json["save"];
    //Serial.println(saveconfig);

    if((!LittleFS.exists("/config.json") && saveconfig==1)){ //user wants to save config
              
      JsonDocument config;

      config[F("ssid")] = ssid;           //const *char
      config[F("pw")] = password;         //const *char
      config[F("ntp_ad")] = ntp_addr;     //const *char
      config[F("offset")] = gmt_offset;   //offset saved as int
      config[F("br_auto")] = br_auto;     //bool as 1 or 0
      config[F("br")] = brightness;       //uint8_t
      config[F("blink")] = blink;        //bool as 1 or 0
      config[F("twelve")] = twelve;     
      config[F("lz")] = leadingzero;      
      config.shrinkToFit();
              
      File fc = LittleFS.open("/config.json", "w+");

      //serializes json and passes it to "fc" var, in order to store it in FS 
      serializeJsonPretty(config, fc);
      fc.close();
      Serial.println("\nCONFIG SAVED");
    }

                
    else if(LittleFS.exists("/config.json") && saveconfig==0){     //if user wants to delete config
              
      LittleFS.remove("/config.json");
      WiFi.disconnect();
      connected=false;
      creds_available = false;
      start_NtpClient=false;
      attempts=0;
      Serial.println(F("\n*Config.json DELETED*"));
    }

    request->send(200, "application/json", "{\"status\":\"updated\"}");
  });

  server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"ms\":\""+ String(millis()) +"\",\"msovfl\":\""+ String(ms_ovfl) + "\"}"); 
  });

  server.onNotFound(notFound);
  server.begin();  
}


void loop() {
  
  if(millis() == 4294967295UL){
    ms_ovfl++;  //can lead to a bug because uint8_t max value is 255, but it'll reach this value after 50days*256= 35years of activity
  }

  if(newScan==true){
    wifiScan();
    newScan=false;
  }

  if(start_NtpClient==true){
    /*The getLocalTime() function has an optional timeout parameter in milliseconds 
    (with the default value being 5 seconds).  This timeout value is used in case there
    is an NTP server request being made in the background
    (for example when you just called the configTime() function before calling the getLocalTime()function like in this example sketch).
    getLocalTime(&timeinfo, 5000);
    If needed, the getLocalTime() function willwait until either a valid system time is received or until the timeout occurs.
    The function will return false, if no valid system time was received before the timeout duration occurs.*/
    getLocalTime(&timeinfo);

    if(myTimer(1000)){
        //printLocalTime();
        if(alarm_status==1){

          //1- alarm rings at the right time
          if(days[timeinfo.tm_wday]==true && alarm_stop==0){ //wrap this in a function

            //RING at the exact time entered by user
            if(timeinfo.tm_hour == alarm_hour && timeinfo.tm_min == alarm_min){
              //Serial.println("alarm_min > " + String(alarm_min));
              //Serial.println("timeinfo.tm_min > " + String(timeinfo.tm_min));

              alarm_ring();

              if(snooze>0 && snoozeMsStart==0){  //activated Once, to start snoozetimer VAR
                snoozeMsStart=1; 
                //Serial.println("FROM alarm time> snoozeOn=1");
              }
            }
          }

          //2- after alarm time passes, enables snoozetimer adn snooze 
          if(timeinfo.tm_min != alarm_min && alarm_stop==0 && snoozeMsStart==1){
            snoozeMsStart=0;  // this makes the code access to this "if" once
            snoozeOn=1; 
            snoozeTimer= millis();
          }

          //RESTORES ALARM FOR NEXT DAY
          else if(timeinfo.tm_min != alarm_min && alarm_stop==1){  //questo ferma il primo allarme ma NON lo snooze (per ora)
            
            if(snooze==0){
              alarm_stop=0;
            }
            else if(millis() - snoozeTimer >= (snooze+3)*60*1000UL){  //alarm stop viene restored dopo un certo intervallo di tempo ()SBAGLAITO!!!
              alarm_stop=0;
              snoozeRing=0;
            }
          }
        }

        if(snoozeRing==1 && alarm_stop==0){
          alarm_ring(); 
        }

        if(br_auto==true){  
            switch(timeinfo.tm_hour){
              case 0 || 00: 
              brightness=0;
              mydisplay.setBrightness(0);
              break;

              case 9:
              brightness=6;
              mydisplay.setBrightness(6);
              break;

              case 17:
              brightness=3;
              mydisplay.setBrightness(3);
              break;

              case 20: //maybe i can remove this one and put brightness=2 at 17:00
              brightness=2; 
              mydisplay.setBrightness(2);
              break;
            }
        }   
           
        if(blink==1){
            if(colon==true){   //colon is ON
              if(!twelve){
                mydisplay.showNumberDecEx(timeinfo.tm_hour, 0b01000000, leadingzero, 2, 0);
                mydisplay.showNumberDecEx(timeinfo.tm_min, 0b01000000, leadingzero, 2, 2);
              }

              else{
                if(timeinfo.tm_hour <= 12){
                  mydisplay.showNumberDecEx(timeinfo.tm_hour, 0b01000000, leadingzero, 2, 0);
                 
                }

                else{
                  mydisplay.showNumberDecEx(timeinfo.tm_hour-12, 0b01000000, leadingzero, 2, 0);
                }

                mydisplay.showNumberDecEx(timeinfo.tm_min, 0b01000000, leadingzero, 2, 2);
              }
              colon=false;  
          }

          else if(colon==false){//colon is OFF

              if(!twelve){
                mydisplay.showNumberDecEx(timeinfo.tm_hour, 0, leadingzero, 2, 0);
                mydisplay.showNumberDecEx(timeinfo.tm_min, 0, leadingzero, 2, 2);
              }

              else{ //if 12hr mode is active
                if(timeinfo.tm_hour <= 12){
                  mydisplay.showNumberDecEx(timeinfo.tm_hour, 0, leadingzero, 2, 0);
                }
                
                else{
                  mydisplay.showNumberDecEx(timeinfo.tm_hour-12, 0, leadingzero, 2, 0);
                }

                mydisplay.showNumberDecEx(timeinfo.tm_min, 0, leadingzero, 2, 2);
              }

            colon=true;
          }
        }
        
        	
      else{ //when blink==0
        if(!twelve){
                mydisplay.showNumberDecEx(timeinfo.tm_hour, 0b01000000, leadingzero, 2, 0);
                mydisplay.showNumberDecEx(timeinfo.tm_min, 0b01000000, leadingzero, 2, 2);
        }

        else{//if 12hr mode is active

                  if(timeinfo.tm_hour <= 12){
                    mydisplay.showNumberDecEx(timeinfo.tm_hour, 0b01000000, leadingzero, 2, 0);
                  }

                  else{
                    mydisplay.showNumberDecEx(timeinfo.tm_hour-12, 0b01000000, leadingzero, 2, 0);
                  }
                  mydisplay.showNumberDecEx(timeinfo.tm_min, 0b01000000, leadingzero, 2, 2);
        }
      }    
    }

    if(snoozeOn==1 && alarm_stop==0){ 
    
      if(snoozeRing==0 && millis() - snoozeTimer >=  snooze*60*1000UL){   //rings for 1 min... 
        snoozeRing=1; //makes the alarm ring for snooze
      }
      
      //intervallo di stop    //test
      if(snoozeRing==1 && millis() - snoozeTimer >= (snooze+1)*60*1000UL){  //...then stops
        snoozeOn=0;
        snoozeRing=0;
      }
    }
  }     

  else{
    displayAnim();
  }

  //optimization: instead of using "bool connected", i can only use WL_CONNECTED
  if(connected == false && creds_available == true ){
    
    displayAnim();
    WiFi.begin(ssid, password);
    
    while(1){

      displayAnim();
      //cycles here until it's connected to wifi
      if (WiFi.status() != WL_CONNECTED && creds_available==true){
          delay(200);
      }
    
      //once connected, exit form while(1) with break, and then from first if since "connected==true" now
      else if(WiFi.status() == WL_CONNECTED){
        //configTime(gmt_offset*3600, 3600, ntp_addr);
        connected = true;
        initMDNS();
        break;
      }

      else if(attempts == 4){
        attempts=0;  //reset "attempts", so it can try a new connection
        creds_available=false;
        Serial.println("LOOP:attempts reset");
        Serial.println(password);
        break; //exit from while(1)
      }
    }
  }
}