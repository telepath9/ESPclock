#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h> 
#include "RTClib.h"
#include "TM1637Display.h"

/* Libraries used:
RTC module: https://github.com/adafruit/RTClib
NTP client: https://github.com/arduino-libraries/NTPClient/tree/master
display: https://github.com/avishorp/TM1637/blob/master/TM1637Display.h
*/

//wemos d1 mini (clone)

    // NON-blocking timer function (delay() is EVIL)
    int myTimer(int everywhen){

      static unsigned long t1, diff_time;
      int ret=0;
      diff_time= millis() - t1;
        
      if(diff_time >= everywhen){
        t1= millis();
        ret=1;
      }
      return ret;
    }



//TM1637 DISPLAY SETUP
#define CLK 13
#define DIO 12

    TM1637Display mydisplay(CLK, DIO);
    
//END TM1637 DISPLAY SETUP


  //wifi credentials
  const char *ssid     = "your_ssid";
  const char *password = "your_pw";

  //SETUP NTP
  WiFiUDP ntpUDP;  //NTP server uses UDP communication protocol

// You can specify the time server pool and the offset, (in seconds, es:(3600=1h))
// additionally you can specify the update interval (in milliseconds).
  NTPClient timeClient(ntpUDP, "ntp1.inrim.it", 7200);

  RTC_DS1307 rtc;

void setup() {
    
    Serial.begin(115200);
    mydisplay.setBrightness(7); //display brightness
    pinMode(14, INPUT_PULLUP);


//WIFI connection
    WiFi.begin(ssid, password);
    mydisplay.clear();
    int ct=4;   //counter used for waiting-4-wifi animation

    //cycle that waits for connection
      while (WiFi.status() != WL_CONNECTED ) {
          
           //setSegments(const uint8_t segments[], uint8_t length = 1, uint8_t pos = 0); i'll use for an other animation
  
           if(myTimer(300)){

                  //animation code
                  if(ct<4 && ct>-1){
                  mydisplay.showNumberDecEx(0, 0, false, 1, ct);
                  Serial.println( "Trying to connect. Press button 2 skip 2 RTC" );
                  ct--;          
                  }

                  //clear() needs to be called in order to refresh the display 
                  else if(ct==4){
                    mydisplay.clear();
                    //Serial.println("ct==4" );
                    ct--;
                  }

                  //resetting ct counter
                  else if(ct==-1){
                  ct=4;      
                  //Serial.println("ct== -1");
                  }

            }


              //when button is pressed, skips directly to RTC MODE
              if(digitalRead(14) == LOW){  
                  break;
              }     
      }
    
      if(WiFi.status() == WL_CONNECTED){
          Serial.println("connected!");
          timeClient.begin();
          timeClient.update();
      }


 
//SETUP RTC 

  //this checks if you wired the RTC module the right way
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    //while (1) delay(10); //comment this if you're not gonna use RTC, uncomment if you want to use RTC
  }

  //checks if RTC hasn't data in it yet
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");

    // When time needs to be set on a new device, or after a power loss,
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    rtc.adjust(DateTime(2025, 3, 20, timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds()));
    //PS: __DATE__ e __TIME__ are macros that sets the RTC to the date & time this sketch was compiled
  }

}


  byte bgt=7;
  bool blink=true;

void loop() {

  //about BRIGHTNESS CONTROL: i must find a way to replace delay() with a non-blocking timer
  //in order that a single press won't decrease brightness more than 1 point.
  
   if(digitalRead(14) == LOW){ 

    //byte bounce=1;
    
    if(bgt<=7 && bgt>1){
      --bgt;
    }
    else if(bgt==1){
      bgt=7;
    }

    mydisplay.setBrightness(bgt);
     
    Serial.print("BRIGHTNESS: ");
    Serial.println(bgt);         

    delay(400); //need to replace this later...
              }     

  //NTP Mode
  if(WiFi.status() == WL_CONNECTED){
  
  timeClient.update(); //mandatory
 

  //this part of code makes the colon (":") blink on/off every 1sec.
  if(myTimer(1000)){

    if(blink==true){
      //colon is ON
      mydisplay.showNumberDecEx(timeClient.getHours(), 0b01000000, true, 2, 0);
      mydisplay.showNumberDecEx(timeClient.getMinutes(), 0b01000000, true, 2, 2);
      blink=false;
   }

   else if(blink==false){
      //colon is OFF
      mydisplay.showNumberDecEx(timeClient.getHours(), 0, true, 2, 0);
      mydisplay.showNumberDecEx(timeClient.getMinutes(), 0, true, 2, 2);
      blink=true;
      }
      
  }

  }
    
  //RTC Mode (when there's no wifi)
  else{
  DateTime adesso = rtc.now();
 
  mydisplay.showNumberDecEx(adesso.hour(), 0b01000000, true, 2, 0);
  mydisplay.showNumberDecEx(adesso.minute(), 0b01000000, true, 2, 2);
     }
}

   
