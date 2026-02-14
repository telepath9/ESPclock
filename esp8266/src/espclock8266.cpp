#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>   //https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html
#include <FS.h>            //for file handling
#include <LittleFS.h>      //to access to filesystem
#include <WiFiUdp.h> 
#include "TM1637Display.h"

//JSON optimizations
#define ARDUINOJSON_SLOT_ID_SIZE 1
#define ARDUINOJSON_STRING_LENGTH_SIZE 1
#define ARDUINOJSON_USE_DOUBLE 0
#define ARDUINOJSON_USE_LONG_LONG 0
#include "ArduinoJson.h"

//NON-blocking timer function (delay() is EVIL)
unsigned long myTimer(unsigned long everywhen){

        static unsigned long t1, diff_time;
        int ret=0;
        diff_time= millis() - t1;
          
        if(diff_time >= everywhen){
            t1= millis();
            ret=1;
        }
        return ret;
}

const char* ssid;
const char* password;
bool creds_available=false;
bool connected=false;   //wifi connection state

const char *esp_ssid = "ESPclock";

//AP pw must be at least 8 chars, otherwise AP won't be customized 
const char *esp_password =  "waltwhite64";

//when true, ESP scan for networks again and overrides the previous networks on net_list
bool newScan = false;

//connection attempts --> when it is set to 0 again, it means pw is wrong
uint8_t attempts = 0;

//creating an Asyncwebserver object
AsyncWebServer server(80);

//TM1637 DISPLAY SETUP
#define CLK 5 //D1 pin 5
#define DIO 4 //D2 pin 4

TM1637Display mydisplay(CLK, DIO);
bool colon=true;
bool blink=true;
bool br_auto=false;
uint8_t brightness=7;

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
//END TM1637 DISPLAY SETUP

//NTP SETUP
WiFiUDP ntpUDP;  //NTP server uses UDP communication protocol
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, 3600); //<-- must be instantiated once, globally
                                                  //updateInterval = every 1hour (3600ms)
const char *ntp_addr;
int gmt_offset;
bool start_NtpClient = false;


void wifiScan(){
    //---------------------------------------------x
    //start wifiSCAN
    //---------------------------------------------x
    //Serial.println("PHASE 1.1: scanning in STA_MODE ");
    WiFi.disconnect();

    byte n = WiFi.scanNetworks();
    Serial.print(n);
    Serial.println(" network(s) found. Displaying the first 5");

    //---------------------------------------------x
    //SSIDs found are stored in json
    //arduinoJson7 doesn't use static/dynamicJsonDocument anymore, but it uses only JsonDocument
    //---------------------------------------------x    
      
    //If json doesn't exists yet, it creates it
    if(!LittleFS.exists("/network_list.json")){
        JsonDocument net_list;
        //Serial.println("Network list doesn't exists. Creating it now..."); 🟠

        //if the number of networks found is <5 (so from [0-4])...
        if(n<5){
            
            //stores number of found networks in json
            net_list["found"] = n;
            JsonArray network = net_list["network"].to<JsonArray>();

            for(byte j = 0; j < n; j++){
              JsonArray network_n_credentials = network[j]["credentials"].to<JsonArray>();
              network_n_credentials.add(WiFi.SSID(j));
              network_n_credentials.add("");
            }
        }

        //if it finds >5 networks, it will display only the top five networks, with index: [0-4]
        else{
          
          net_list["found"] = 5;
          JsonArray network = net_list["network"].to<JsonArray>();

          for(byte j = 0; j < 5; j++){
              JsonArray network_n_credentials = network[j]["credentials"].to<JsonArray>();
              network_n_credentials.add(WiFi.SSID(j));
              network_n_credentials.add("");
          }
      }

      //---------------------------------------------x
      //After creating JSON file (jsondocument), it must be stored in FS
      //---------------------------------------------x
      File fx = LittleFS.open("/network_list.json", "w");

      //serializes json and passes it to "fx" var
      serializeJsonPretty(net_list, fx);
      fx.close();
    }


    //---------EXISTING JSON---------------------
    //2. IF JSON ALREADY EXISTS: access to json, reset it, then add new networks to it
    else{
      //Serial.println("Network list already exists! Updating it..."); 🟠
      JsonDocument net_listUp;
     
      //1. fetch and open json from FS, then deserializes it
      File fxup = LittleFS.open("/network_list.json", "w+");
      deserializeJson(net_listUp, fxup);

      //if there are n<5 networks
      if(n<5){
        //updates the values of the entries of the older one
        net_listUp["found"] = n;
        JsonArray network = net_listUp["network"].to<JsonArray>();

        for(byte k = 0; k < n; k++){
            JsonArray network_n_credentials = network[k]["credentials"].to<JsonArray>();
            network_n_credentials.add(WiFi.SSID(k));
            network_n_credentials.add("");    //
        }
      }

      //if there are n>5 networks -> it truncates the list to only 5 ssids
      else{
            net_listUp["found"] = 5;
            JsonArray network = net_listUp["network"].to<JsonArray>();
            
            for(byte k = 0; k < 5; k++){
              //dynamically adds, to each entry "k", a new array to the main array "network"
              JsonArray network_n_credentials = network[k]["credentials"].to<JsonArray>();

              //adds SSID name to the entry[k][0]
              network_n_credentials.add(WiFi.SSID(k));

              //adds pw field (initially empty) to entry[k][1] 
              network_n_credentials.add("");
            }
      }

      //3. serializing
      serializeJsonPretty(net_listUp, fxup);
      fxup.close();
    }     
}

void checkConfig(void){

    if(LittleFS.exists("/config.json")){
      Serial.println(F("config esists, trying to restore it*"));
      creds_available=true;

      File fld = LittleFS.open("/config.json", "r");
      JsonDocument load_cf;
      
      DeserializationError error = deserializeJson(load_cf, fld);

      if (error) {
        fld.close();
        Serial.print(F("deserializeJson() failed: "));
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
          //Serial.print("+");

        if(myTimer(3000)){  
          
          ++attempts;
          mydisplay.showNumberDec(attempts, true, 1, 3);
        }

        else if(attempts==6){
          attempts=0;
          creds_available = false;
          //Serial.println(F("Can't connect. Goto webUI"));
          break;
        }
      }

      if(WiFi.status() == WL_CONNECTED){
        attempts=0;
        connected=true;
        Serial.println("WIFI RESTORED");

        start_NtpClient=true;
        timeClient.begin();
        ntp_addr= strdup(load_cf["ntp_ad"]); 
        gmt_offset = load_cf["offset"];
        
        timeClient.setPoolServerName(ntp_addr);  //poolservername accepts only const char*
        timeClient.setTimeOffset(gmt_offset*3600);  
        
        brightness = (uint8_t)load_cf["br"];
        mydisplay.setBrightness(brightness);
        blink=  load_cf[F("blink")];
        br_auto = load_cf[F("br_auto")];
        fld.close();
      }
  }
  return;
}

//this is called when you request resources from esp webserver that don't exists
void notFound(AsyncWebServerRequest *request){
    request->send(404, "text/plain", "NOT FOUND");
}

void setup() {
  Serial.begin(115200);
  
  //display
  mydisplay.setBrightness(7); 
  mydisplay.clear();

  //to format FS
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
  //PHASE1 - AP_STA_MODE + WIFI SCAN
  //here scans for networks, and as already said, networks are then stored in json
  //Serial.println("\nPHASE 1.0: AP_STA_MODE + WIFI SCAN");

  //IPAddress local_IP(192, 168, 1, 33); 
  IPAddress staticIP(192, 168, 1, 33); 
  IPAddress gateway(192,168,4,9);
  IPAddress subnet(255,255,255,0);
  IPAddress dns(1,1,1,1);
    
  WiFi.config(staticIP, gateway, subnet, dns);
  WiFi.mode(WIFI_AP_STA);
  
  delay(100);

  if(WiFi.status() != WL_CONNECTED){
    wifiScan();
  }
  
  //---------------------------------------------x
  //PHASE 2: here user choose its ssid and enters pw
  //---------------------------------------------x
  
  //Serial.println("PHASE 2: Configuring AP");  
  WiFi.softAP(esp_ssid, esp_password, false, 2);     //Starting AP on given credential

  //Serial.print("AP IP address: ");  🟠
  //Serial.println(WiFi.softAPIP());  🟠           //Default AP-IP is 192.168.4.1
  //Serial.println(esp_ssid);         🟠

  //Route for root index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send(LittleFS, "/index.html", "text/html" ); 
    Serial.println("Connection detected");
  });

  //this is triggered when entering to the webUI after the clock is set. It checks the status of all of the UI elements and updates it
  server.on("/uicheck", HTTP_GET, [](AsyncWebServerRequest *request){
       
    JsonDocument uicheck_json;

    uicheck_json["conn"] = connected;
    uicheck_json["bright"]= brightness; 
    uicheck_json["br_auto"] = br_auto;
    uicheck_json["blink"] = blink;
    uicheck_json["config"] = (LittleFS.exists("/config.json")) ? 1 : 0;

    String uc_str;
    serializeJson(uicheck_json, uc_str);

    request->send(200,  "application/json", uc_str);
  });

  //client requests list of ssids and server sends it to client
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {

    File f = LittleFS.open("/network_list.json", "r");

    //checks json integrity
    if(!f) {
      //Serial.println("Error opening /network_list.json");
      request->send(500, "application/json", "{\"error\":\"Failed to open network_list.json\"}");
      f.close();
    }

    else{
      request->send(LittleFS,  "/network_list.json", "application/json");
      newScan = true;
      f.close();
    }
  });

  //---------------------------------------------x
  //client(JS) sends http POST req with wifi credentials (inside the body) to server
  //---------------------------------------------x
  server.on("/sendcreds", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
           
    //deserializes http POST req body (has creds inside) from client
    JsonDocument thebody;
    deserializeJson(thebody, data);

    const char* user_ssid_str = thebody["ssid"];  
    const char* user_pw = thebody["pw"];

    ssid = strdup(user_ssid_str);
    password = strdup(user_pw);
    creds_available = true;
    Serial.println(ssid);
    Serial.println(password);
            
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

    //Serial.print("attempt num: ");
    //Serial.println(attempts);
          
    if(attempts == 6){
      creds_available = false;
      attempts=0;
      Serial.println(F("handler says: 7 attempts->WRONG PASSWORD"));
      request->send(200, "application/json", "{\"stat\":\"fail\"}");
    }

    else{
      ++attempts;
      if(WiFi.status() == WL_CONNECTED){
        attempts=0;
        request->send(200, "application/json", "{\"stat\":\"ok\"}");
      }

      else{
        Serial.println("PLEASE WAIT");
        request->send(200, "application/json", "{\"stat\":\"wait\"}");
      }
    }
  });


  server.on("/updatetime", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

    JsonDocument ntp_json;
    deserializeJson(ntp_json, data);

    ntp_addr = strdup(ntp_json["ntp_addr"]); 
    gmt_offset = (int)atoi(ntp_json["offset"]);
          
    timeClient.setPoolServerName(ntp_addr);  //poolservername must be const char*
    timeClient.setTimeOffset(gmt_offset*3600);    //setTimeOffset only accepts "signed long"

    if(!timeClient.isTimeSet()){
      Serial.println("TIME not SET, try new NTP addr");
    }

    if(start_NtpClient == false){
      start_NtpClient=true;
      timeClient.begin();
    }
                  
    request->send(200, "application/json", "{\"ntp\":\"OK\"}");
  });


  server.on("/slider", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
          
          //optimization: maybe i shouldn't use JSON for this (?)
          JsonDocument bgt_json;
          deserializeJson(bgt_json, data);

          //extract light value
          brightness =(uint8_t)atoi(bgt_json["bgt"]);
          mydisplay.setBrightness(brightness); 
          Serial.print("SLIDER: ");
          Serial.println((uint8_t)atoi(bgt_json["bgt"]));

          request->send(200, "application/json", "{\"status\":\"BGT OK\"}");
  });

  server.on("/br_auto", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

            JsonDocument br_auto_json;
            deserializeJson(br_auto_json, data);
            br_auto = br_auto_json["br"];

            //optimization: should i replace it with a switch-case (?)
            if(timeClient.getHours() >= 0 && timeClient.getHours() < 9){
              brightness=0;
              mydisplay.setBrightness(0);
              request->send(200, "application/json", "{\"status\":\"0\"}");
            }

            else if(timeClient.getHours() >= 7 && timeClient.getHours() < 17){
              brightness=6;
              mydisplay.setBrightness(6);
              request->send(200, "application/json", "{\"status\":\"6\"}");
            }

            else if(timeClient.getHours() >= 17 && timeClient.getHours() < 20){
              brightness=3;
              mydisplay.setBrightness(3);
              request->send(200, "application/json", "{\"status\":\"3\"}");
            }

            else if(timeClient.getHours() >= 20){
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

  server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

    JsonDocument scf_json;
    deserializeJson(scf_json, data);
    bool saveconfig = scf_json["save"];
    Serial.print("ACTION: ");
    Serial.println(saveconfig); //PERCHè QUI DIVENTA 0 SE FACCIO DELETE'??

    if((!LittleFS.exists("/config.json") && saveconfig==1)){ //vuol dire chhe user vuole salvare config
              
      JsonDocument config;

      config[F("ssid")] = ssid;           //const *char
      config[F("pw")] = password;         //const *char
      config[F("ntp_ad")] = ntp_addr;     //const *char
      config[F("offset")] = gmt_offset;   //offset saved as int
      config[F("br_auto")] = br_auto;     //bool as 1 or 0
      config[F("br")] = brightness;       //uint8_t
      config[F("blink")] = blink;        //bool as 1 or 0

      config.shrinkToFit();
              
      //fai store di ssid, pw, ... su config
      //scrive su flash
      File fc = LittleFS.open("/config.json", "w+");

      //serializes json and passes it to "fc" var, in order to store it in FS 
      serializeJsonPretty(config, fc);
      fc.close();
      Serial.println(F("\nCONFIG SAVED"));
    }

                
    else if(LittleFS.exists("/config.json") && saveconfig==0){     //se prima config era salvata e poi user non vuole più salvarla...
              
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

  server.onNotFound(notFound);

  //start server
  server.begin();
}


void loop() {

  if(newScan==true){
    wifiScan();
    newScan=false;
  }

  if(start_NtpClient==true){
  
    timeClient.update();  

    if(myTimer(1000)){

        if(br_auto==true){

            switch(timeClient.getHours()){
    
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

        if(blink ==1){
            if(colon==true){
            //colon is ON
            mydisplay.showNumberDecEx(timeClient.getHours(), 0b01000000, true, 2, 0);
            mydisplay.showNumberDecEx(timeClient.getMinutes(), 0b01000000, true, 2, 2);
            colon=false;
          }

          else if(colon==false){
            //colon is OFF
            mydisplay.showNumberDecEx(timeClient.getHours(), 0, true, 2, 0);
            mydisplay.showNumberDecEx(timeClient.getMinutes(), 0, true, 2, 2);
            colon=true;
          }
        }
        
        else{
            mydisplay.showNumberDecEx(timeClient.getHours(), 0b01000000, true, 2, 0);
            mydisplay.showNumberDecEx(timeClient.getMinutes(), 0b01000000, true, 2, 2);
        }
    }
  }     

  else{
    displayAnim();
  }


  //optimization: instead of using "bool connected", i can only use WL_CONNECTED
  if(connected == false && creds_available == true ){
    
    displayAnim();
    
    Serial.println("Trying connection with these creds: ");
    Serial.print("SSID chosen: ");
    Serial.println(ssid);
    Serial.print("PW is: ");
    Serial.println(password); 

    WiFi.begin(ssid, password);
    
    while(1){

      displayAnim();
            
      //cycles here until it's connected to wifi
      if (WiFi.status() != WL_CONNECTED && creds_available==true){
          delay(200);
          //Serial.print(".");
      }
    
      //once connected, exit form while(1) with break, and then from first if since "connected==true" now
      else if(WiFi.status() == WL_CONNECTED){
        connected = true;
        //attempts=0;
        break;
      }

      else if(attempts == 6){
       
        //resets "attempts", so it can try a new connection
        attempts=0;  
        break; //exit from while(1)
      }
    }
  }
}


