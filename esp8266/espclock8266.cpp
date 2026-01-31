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
#include "RTClib.h"
#include "TM1637Display.h"

//JSON optimizations
#define ARDUINOJSON_SLOT_ID_SIZE 1
#define ARDUINOJSON_STRING_LENGTH_SIZE 1
#define ARDUINOJSON_USE_DOUBLE 0
#define ARDUINOJSON_USE_LONG_LONG 0
#include "ArduinoJson.h"
#include "AsyncJson.h"    //maybe is unused🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴


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
bool creds_available= false;
bool connected = false;   //wifi connection state
uint8_t brightness=7;

const char *esp_ssid = "ESPclock";

//AP pw should be long at least 8 chars, otherwise AP won't be customized 
const char *esp_password =  "waltwhite64";

//when true, ESP scan for networks again and overrides the previous networks on net_list
bool newScan = false;

//connection attempts --> when it reaches 0, it means pw is wrong
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

const uint8_t SEG_Err[]={
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, //E
  SEG_E | SEG_G, SEG_E | SEG_G,          //rr
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
uint8_t gmt_offset;
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
    //Creates JSON file
    //---------------------------------------------x
      File fx = LittleFS.open("/network_list.json", "w");

      //serializes json and passes it to "fx" var, in order to store it in FS 
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
        //creates a new json, instead of updating the entries' values of the older one
        net_listUp["found"] = n;
        JsonArray network = net_listUp["network"].to<JsonArray>();

        for(byte k = 0; k < n; k++){
            JsonArray network_n_credentials = network[k]["credentials"].to<JsonArray>();
            network_n_credentials.add(WiFi.SSID(k));
            network_n_credentials.add("");
        }
      }

        //if there are n>5 networks
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

  //this function gets called when you request resources from esp webserver that are not present
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
 
  //PHASE1 - AP_STA_MODE + WIFI SCAN
  //here scans for networks, and as already said, networks are then stored in json
  //Serial.println("\nPHASE 1.0: AP_STA_MODE + WIFI SCAN");

  //  IPAddress local_IP(192, 168, 1, 33); 
  IPAddress staticIP(192, 168, 1, 33); 
  IPAddress gateway(192,168,4,9);
  IPAddress subnet(255,255,255,0);
  IPAddress dns(1,1,1,1);
    
  WiFi.config(staticIP, gateway, subnet, dns);
  WiFi.mode(WIFI_AP_STA);
  
  delay(100);
  wifiScan();

  //---------------------------------------------x
  //PHASE 2: here user choose its ssid and enters pw
  //---------------------------------------------x
  
   //Serial.println("PHASE 2: Configuring AP");
    
  // You can remove the APpassword parameter if you want the hotspot to be open.
  WiFi.softAP(esp_ssid, esp_password, false, 2);     //Starting AP on given credential

  //Serial.print("AP IP address: ");  🟠
  //Serial.println(WiFi.softAPIP());  🟠           //Default AP-IP is 192.168.4.1
  //Serial.println(esp_ssid);         🟠
   

       //Route for root index.html
       server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
          request->send(LittleFS, "/index.html", "text/html" ); 
          Serial.println("Connection detected");
        });

        //cambia route da wifi_check --> uicheck ? o altro
        //oltre allo stato del wifi deve mandare anche le vars blink, br_auto, ...
        server.on("/wifi_check", HTTP_GET, [](AsyncWebServerRequest *request){

            //devo mandare 1 solo request->send con le vars nel json
            /*
            quindi sarà tipo:
            conn: connected (che è 0 o 1... poi JS lo interpreta)
            br_auto: ...
            blink: ... 
            
            */

            if(connected==1){
              request->send(200,  "application/json", "{\"conn\":\"true\"}"); 
            }

            else{
              request->send(200,  "application/json", "{\"conn\":\"false\"}"); 
            }
        });

          
        server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {

            File f = LittleFS.open("/network_list.json", "r");

            //checks json integrity
            if(!f) {
                Serial.println("Error opening /network_list.json");
                request->send(500, "application/json", "{\"error\":\"Failed to open network_list.json\"}");
                return;
            }

            else{
                request->send(LittleFS,  "/network_list.json", "application/json");
                newScan = true;
            }
        });

  //---------------------------------------------x
  //here client(JS) sends http POST req with wifi credentials (inside the body) to server
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
            
            request->send(200, "application/json", "{\"creds\":\"OK\"}");
          }
        ); 

        //refresh SSID list on frontend
        server.on("/refresh", HTTP_GET, [](AsyncWebServerRequest *request) {
           
          File f = LittleFS.open("/network_list.json", "r");

          //check json integrity
          if(!f) {
            Serial.println("Error opening /network_list.json");
            request->send(500, "application/json", "{\"error\":\"Failed to open config.json\"}");
            return;
          }

          else{
            request->send(LittleFS,  "/network_list.json", "application/json");
            newScan =true;
          }
        });

        //HTTP GET req from client, in order to know if connection attempt was successful
        server.on("/wifi_status", HTTP_GET, [](AsyncWebServerRequest *request) {

          Serial.print("attempt num: ");
          Serial.println(attempts);
          
          if(attempts == 6){
            creds_available = false;
            Serial.println("handler says: 7 attempts->WRONG PASSWORD");
            request->send(200, "application/json", "{\"stat\":\"fail\"}");
          }

          else{
              ++attempts;
              if(WiFi.status() == WL_CONNECTED){

              //Serial.println("WIFI CONNECTED"); 🟠
              //optimization: i may send only 200, without JSON STRING (?)
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

          //extract ntp address
          const char* user_ntp = ntp_json["ntp_addr"];  
          const char* user_offset_str = ntp_json["offset"];

          //cast from string to int
          int gmt_offset = (int)atoi(user_offset_str);
          ntp_addr = strdup(user_ntp); 

          timeClient.setPoolServerName(ntp_addr);  //poolservername must be const char*
          timeClient.setTimeOffset(gmt_offset*3600);    //this must be signed long

          if(!timeClient.isTimeSet()){
            Serial.println("TIME not SET, try new NTP addr");
          }

          start_NtpClient = true;
          timeClient.begin();
          
          request->send(200, "application/json", "{\"ntp\":\"OK\"}");
        });


        server.on("/slider", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
          
          //optimization: maybe i shouldn't use JSON for this (?)
          JsonDocument bgt_json;
          deserializeJson(bgt_json, data);

          //extract light value
          //const char* user_bgt = bgt_json["bgt"];  
          mydisplay.setBrightness((uint8_t)atoi(bgt_json["bgt"])); 

          request->send(200, "application/json", "{\"status\":\"BGT OK\"}");
        });

         server.on("/br_auto", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

            JsonDocument br_auto_json;
            deserializeJson(br_auto_json, data);
            br_auto = (uint8_t)br_auto_json["bl"];  //update blink var

            //i should replace it with a switch-case
            if(timeClient.getHours() >= 0 && timeClient.getHours() < 9){
              mydisplay.setBrightness(0);
              request->send(200, "application/json", "{\"status\":\"0\"}");
            }

            else if(timeClient.getHours() >= 7 && timeClient.getHours() < 17){
              mydisplay.setBrightness(6);
              request->send(200, "application/json", "{\"status\":\"6\"}");
            }

            else if(timeClient.getHours() >= 17 && timeClient.getHours() < 20){
              mydisplay.setBrightness(3);
              request->send(200, "application/json", "{\"status\":\"3\"}");
            }

            else if(timeClient.getHours() >= 20){
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



  server.onNotFound(notFound);

//start server
  server.begin();
  //Serial.println("PHASE2: HTTP server started");
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
              mydisplay.setBrightness(0);
              break;

              case 9:
              mydisplay.setBrightness(6);
              break;

              case 17:
              mydisplay.setBrightness(3);
              break;

              case 20: //magari tolgo questo e alle 17 brightness va a 2 
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
    /*
    Serial.println("Trying connection with these creds: ");
    Serial.print("SSID chosen: ");
    Serial.println(ssid);
    Serial.print("PW is: ");
    Serial.println(password); */

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
            
        //Serial.println("loop says: CONNECTED!");         
        //Serial.println(WiFi.localIP());
        break;
      }

      else if(attempts == 6){
        //Serial.print(WiFi.status());
        //Serial.println("loop says: WRONG PASSWORD, RETRY");
              
        //resets "attempts", so it can try a new connection
        attempts=0;  
        break; //exit from while(1)
      }
    }
  }
}


