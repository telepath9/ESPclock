#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <FFat.h>
#include "RTClib.h"
#include <ArduinoJson.h>


// JSON optimizations
#define ARDUINOJSON_SLOT_ID_SIZE 1
#define ARDUINOJSON_STRING_LENGTH_SIZE 1
#define ARDUINOJSON_USE_DOUBLE 0
#define ARDUINOJSON_USE_LONG_LONG 0
//#include "ArduinoJson.h"

// --- TM1652 HARDWARE CONFIGURATION ---
#define TM1652_SDA_Pin 7
#define TM1652_BITDELAY 49
const uint8_t dig_num[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

volatile bool brightnessDirty = false;
volatile uint8_t brightnessTarget = 5;   // TM1652 expects 1..8
uint8_t lastAppliedBrightness = 5;

static const char* WIFI_CRED_PATH = "/wifi.json";

// Embedded Minified HTML (Stripped for brevity, keep your original HTML block here)
const char index_html[] PROGMEM = R"rawliteral( <!DOCTYPE html><html><head><title>ESPclock</title><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><link rel="icon" href="data:,"><style>body{color:#ced4da;background-color:#101011;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Oxygen,Ubuntu,Cantarell,"Helvetica Neue",sans-serif}.flex_container{display:flex;flex-direction:column;width:315px;border-radius:14px;border:1px solid #3c3e3f;background-color:#141414;text-align:center;margin:0 auto;padding:0 8px 4px 8px}.ssid_wrapper{display:flex;flex-direction:row;margin:0 18px 15px 20px}.pw_wrapper{display:flex;flex-direction:row;width:275px;height:41px;margin:0 20px 36px;border-radius:6px;background-color:#444}.ntp_wrapper,#ntp_id{display:flex;flex-direction:row}#connect{cursor:pointer;font-size:14px;font-weight:550;width:275px;height:40px;text-align:center;border-radius:8px;border-style:none;color:#ced4da;background-color:#ae0423;user-select:none}#connect:hover{background-color:#d40228}.butpop{margin:0 20px 20px}.ntp_form{border-radius:10px}#ntp_id{margin:0 0 10px 20px;font-size:15px;padding:0 0 0 6px;color:#bec1c4;background-color:#444;width:150px;height:40px;border-radius:6px;border-style:none}#gmt_offset{margin:0 18px 0 10px;font-size:15px;padding:0 0 0 6px;background-color:#444;color:#bec1c4;width:120px;height:40px;border-radius:6px;border-style:none}#update_but{cursor:not-allowed;color:#ced4da;background-color:#8099b3;font-size:14px;width:275px;height:40px;font-weight:550;border-radius:8px;border-style:none;margin:0 0 20px 20px}.popup .popuptext{visibility:hidden;width:160px;background-color:#dcdcdc;color:#000;text-align:center;border-radius:6px;padding:8px 0;position:absolute;z-index:1;bottom:125%;left:50%;margin-left:-80px}.popup .popuptext::after{content:"";position:absolute;top:100%;left:50%;margin-left:-5px;border-width:5px;border-style:solid;border-color:#dcdcdc transparent transparent transparent}.popup .show{visibility:visible;animation:fadeIn 5s}#pw{font-size:15px;padding:0 0 0 6px;background-color:#444;color:#bec1c4;height:41px;width:230px;border-radius:6px 0 0 6px;border-color:transparent;border-style:none}::placeholder{color:#adb5bd}.blinky{height:30px}.switch{margin:0 18px 0 0;position:relative;display:inline-block;width:56px;height:30px}.switch input{opacity:0;width:0;height:0}.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#9c9c9c;transition:.4s}.slider:before{position:absolute;content:"";height:22px;width:22px;left:4px;bottom:4px;background-color:#fff;transition:.4s}input:checked+.slider{background-color:#ae0423}input:focus+.slider{box-shadow:0 0 1px #ae0423}input:checked+.slider:before{transform:translateX(26px)}.slider.round{border-radius:34px}.slider.round:before{border-radius:50%}@keyframes fadeIn{0%,100%{opacity:0}50%{opacity:1}}</style></head><body><div class="flex_container"><h2 style="margin:8px 0;padding-top:5px;padding-bottom:5px;border-radius:12px">ESPclock Setup</h2><form style="background-color:#1a1a1a;text-align:left;border:1px solid #3c3e3f;border-radius:12px"><h3 style="margin:18px 0 18px 26px">WiFi Settings</h3><label style="padding-left:26px" for="ssid_list">Available networks</label><div class="ssid_wrapper"><select id="ssid_list" name="ssid" style="padding-left:2px;color:#bec1c4;font-size:15px;background-color:#444;height:40px;width:237px;border-radius:6px 0 0 6px;border:none"></select><button type="button" id="refresh" style="border-radius:0 6px 6px 0;border:none;cursor:pointer;width:40px;height:40px;background-color:#444"><svg style="width:30px;height:30px;margin:6px 8px 6px 1px"><use transform="scale(.88)" href="#refresh_id"/></svg></button></div><label style="padding-left:26px" for="pw">Password</label><br><div class="pw_wrapper"><input type="password" placeholder="Enter password" id="pw" maxlength="32" name="pw" required><button type="button" id="toggleopen_id" style="border:none;cursor:pointer;width:40px;height:40px;background-color:transparent;padding:0"><svg style="width:30px;height:37px;margin:3px 5px 0 8px"><use id="useopen" transform="scale(.78)" href="#openeye_id"/></svg></button><br></div><div class="butpop"><div class="popup" style="position:relative;display:inline-block;cursor:pointer"><button type="submit" id="connect">CONNECT</button><span class="popuptext" style="font-size:14px;user-select:none" id="myPopup"></span></div></div></form><div class="ntp_form" style="background-color:#1a1a1a;text-align:left;margin-top:6px;border:1px solid #3c3e3f;border-radius:12px"><h3 style="margin:18px 0 18px 26px">NTP Server Settings</h3><div class="ntp_wrapper"><input id="ntp_id" type="text" placeholder="Enter NTP address" required><input required type="number" placeholder="GMT Offset" id="gmt_offset" name="gmt_offset" min="-12" max="12"></div><button disabled type="submit" id="update_but">UPDATE TIME</button></div><div class="brightness_form" style="background-color:#1a1a1a;text-align:left;margin-top:6px;border:1px solid #3c3e3f;border-radius:12px"><h3 style="margin:18px 10px 5px 26px">Display Settings</h3><label style="display:flex;align-items:center"><p style="font-size:17px;margin:18px 0 2px 26px">Brightness:</p><p id="brp" style="margin:18px 0 2px 6px"></p></label><input style="margin:0 0 2px 20px;width:275px;height:40px" type="range" id="brightness" name="brightness" min="0" max="7"><label style="display:flex;align-items:center;justify-content:space-between;margin:4px 0 20px 0" class="blinky"><p style="font-size:17px;margin:0 0 2px 26px">Brightness (auto)</p><label class="switch"><input id="bright_auto" type="checkbox"><span class="slider round"></span></label></label><label style="display:flex;align-items:center;justify-content:space-between;margin:4px 0 20px 0" class="blinky"><p style="font-size:17px;margin:0 0 2px 26px">Blinking colon</p><label class="switch"><input id="blink_togg" type="checkbox" checked><span class="slider round"></span></label></label></div><p style="margin:4px 12px 0 12px;color:#dee2e6">ESPclock v2.0.4 - A project by <strong>telepath</strong></p></div><svg style="display:none"><symbol id="refresh_id" viewBox="0 0 640 640"><path fill="#dee2e6" d="M552 256L408 256c-9.7 0-18.5-5.8-22.2-14.8-3.7-9-1.7-19.3 5.2-26.2L437.7 168.3c-75.3-58.6-184.3-53.3-253.5 15.9-75 75-75 196.5 0 271.5 75 75 196.5 75 271.5 0 8.2-8.2 15.5-16.9 21.9-26.1 10.1-14.5 30.1-18 44.6-7.9 14.5 10.1 18 30.1 7.9 44.6-8.5 12.2-18.2 23.8-29.1 34.7-100 100-262.1 100-362 0-99.9-100-100-262 0-362 94.3-94.3 243.7-99.6 344.3-16.2L535 71c6.9-6.9 17.2-8.9 26.2-5.2 9 3.7 14.8 12.5 14.8 22.2V232c0 13.3-10.7 24-24 24z"/></symbol><symbol id="openeye_id" viewBox="0 0 576 380"><path fill="#dee2e6" d="M288 32c-80.8 0-145.5 36.8-192.6 80.6C48.6 156.1 17.3 208 2.4 243.7c-3.3 7.9-3.3 16.7 0 24.6 14.9 35.7 46.2 87.7 93 131.1C142.5 443.1 207.2 480 288 480s145.5-36.8 192.6-80.6c46.8-43.5 78.1-95.4 93-131.1 3.3-7.9 3.3-16.7 0-24.6-14.9-35.7-46.2-87.7-93-131.1C433.5 68.8 368.8 32 288 32zM144 256a144 144 0 1 1 288 0 144 144 0 0 1-288 0zm144-64c0 35.3-28.7 64-64 64-11.5 0-22.3-3-31.7-8.4-1 10.9-.1 22.1 2.9 33.2 13.7 51.2 66.4 81.6 117.6 67.9s81.6-66.4 67.9-117.6c-12.2-45.7-55.5-74.8-101.1-70.8 5.3 9.3 8.4 20.1 8.4 31.7z"/></symbol><symbol id="shuteye_id" viewBox="0 0 576 380"><path fill="#dee2e6" d="M41-24.9c-9.4-9.4-24.6-9.4-33.9 0s-9.4 24.6 0 34l528 528c9.4 9.4 24.6 9.4 33.9 0s9.4-24.6 0-33.9l-96.4-96.4c2.7-2.4 5.4-4.8 8-7.2 46.8-43.5 78.1-95.4 93-131.1 3.3-7.9 3.3-16.7 0-24.6-14.9-35.7-46.2-87.7-93-131.1C433.5-3.3 368.8-40.1 288-40.1c-56.8 0-105.6 18.2-146 44.2L41-24.9zM204.5 138.7c23.5-16.8 52.4-26.7 83.5-26.7 79.5 0 144 64.5 144 144 0 31.1-9.9 59.9-26.7 83.5l-34.7-34.7c12.7-21.4 17-47.7 10.1-73.7-13.7-51.2-66.4-81.6-117.6-67.9-8.6 2.3-16.7 5.7-24 10l-34.7-34.7zM325.3 395.1c-11.9 3.2-24.4 4.9-37.3 4.9-79.5 0-144-64.5-144-144 0-12.9 1.7-25.4 4.9-37.3L69.4 139.2c-32.6 36.8-55 75.8-66.9 104.5-3.3 7.9-3.3 16.7 0 24.6 14.9 35.7 46.2 87.7 93 131.1 47.1 43.7 111.8 80.6 192.6 80.6 37.3 0 71.2-7.9 101.5-20.6l-64.2-64.2z"/></symbol></svg><script>let network_list;function networkFetch(e){fetch(e).then(e=>e.json()).then(e=>{network_list=e;const t=document.getElementById("ssid_list");let n=e.found;for(let r=0;r<n;r++){let n=document.createElement("option");n.value=r,n.innerText=e.network[r].credentials[0],t.appendChild(n)}})}function disable_connect_but(){connect_but.setAttribute("disabled",""),connect_but.style.backgroundColor="#ab6975",connect_but.style.cursor="not-allowed"}function disable_refresh_but(){refresh_but.setAttribute("disabled",""),refresh_but.style.cursor="not-allowed",document.querySelectorAll("path")[0].style.fill="#808080"}window.onload=function(){networkFetch("/scan"),fetch("/uicheck").then(e=>e.json()).then(e=>{console.log(e.conn),console.log(e.br_auto),console.log(e.blink),true===e.conn&&(disable_connect_but(),connect_but.innerHTML="CONNECTED",connect_but.style.backgroundColor="#52885d",disable_refresh_but(),update_but.removeAttribute("disabled"),update_but.style.cursor="pointer",update_but.style.backgroundColor="#267bbc"),true===e.br_auto&&(slider.setAttribute("disabled",""),bright_auto.setAttribute("checked","")),false===e.blink&&blink.removeAttribute("checked","")})};function networkRefresh(){const e=document.getElementById("ssid_list").querySelectorAll("option");for(let t=e.length-1;t>-1;--t)e[t].remove();networkFetch("/refresh")}const refresh_but=document.getElementById("refresh");refresh_but.addEventListener("click",networkRefresh);const password=document.getElementById("pw"),buttoneye=document.getElementById("toggleopen_id"),useshift=document.getElementById("useopen"),popup_span_elem=document.getElementById("myPopup");buttoneye.onclick=function(){"password"===password.type?(password.type="text",useshift.setAttribute("href","#shuteye_id")):(password.type="password",useshift.setAttribute("href","#openeye_id"))};const connect_but=document.getElementById("connect"),update_but=document.getElementById("update_but"),pw_element=document.getElementById("pw");let polling_state=!1,connected=!1;const ascii_anim=["🔵 ⚪ ⚪","⚪ 🔵 ⚪","⚪ ⚪ 🔵"];function pollingfunc(){disable_connect_but(),disable_refresh_but();let e=0,t=setInterval(()=>{3===e?e=0:(connect_but.innerHTML=ascii_anim[e],e++)},500);const n=setInterval(()=>{fetch("/wifi_status").then(e=>e.json()).then(r=>{console.log("data.stat:",r.stat),"ok"===r.stat?(clearInterval(n),connected=!0,clearInterval(t),connect_but.innerHTML="CONNECTED",connect_but.style.backgroundColor="#52885d",popup_span_elem.innerHTML="CONNECTED✅",popup_span_elem.classList.add("show"),update_but.removeAttribute("disabled"),update_but.style.cursor="pointer",update_but.style.backgroundColor="#267bbc",setTimeout(()=>{popup_span_elem.classList.remove("show")},4900)):"fail"===r.stat&&(console.log("WRONG PASSWORD!"),clearInterval(n),clearInterval(t),connect_but.innerHTML="CONNECT",popup_span_elem.innerHTML="WRONG PASSWORD❌",popup_span_elem.classList.add("show"),setTimeout(()=>{popup_span_elem.classList.remove("show"),connect_but.removeAttribute("disabled"),connect_but.style.cursor="pointer",connect_but.style.backgroundColor="#9a031e",refresh_but.removeAttribute("disabled"),refresh_but.style.cursor="pointer",document.querySelectorAll("path")[0].style.fill="#dee2e6"},4900))})},3e3)}connect_but.onclick=function(){let e=pw_element.value;if(console.log("Password is: ",e),0==e.length)alert("🔴Password field is empty!!!");else{const t=document.getElementById("ssid_list");let n=t.value;const r=t.querySelectorAll("option");let o=r[n].innerText;fetch("/sendcreds",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({ssid:o,pw:e})}).then(e=>e.json()).then(e=>{polling_state=!0}),pollingfunc()}},update_but.onclick=function(){const e=document.getElementById("ntp_id");let t=e.value,n=document.getElementById("gmt_offset").value;n>12||n<-12?alert("🔴GMT/UTC Offset must be a number from -12 to +12 !!"):(console.log("NTP addr: ",t,"Offset:",n),fetch("/updatetime",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({ntp_addr:t,offset:n})}).then(e=>e.json()).then(e=>{console.log(e.ntp)}))};const slider=document.getElementById("brightness"),br_tag=document.getElementById("brp");slider.onchange=function(){br_tag.textContent=slider.value,fetch("/slider",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({bgt:this.value})}).then(e=>e.json()).then(e=>{console.log(e.status)})};const blink=document.getElementById("blink_togg");blink.onchange=function(){let e=this.checked?1:0;fetch("/blink",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({bl:e})}).then(e=>e.json()).then(e=>{console.log(e.status)})};const bright_auto=document.getElementById("bright_auto");bright_auto.onchange=function(){this.checked?slider.setAttribute("disabled",""):slider.removeAttribute("disabled","");let e=!!this.checked;fetch("/br_auto",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({br:e})}).then(e=>e.json()).then(e=>{br_tag.textContent=e.status})};</script></body></html> )rawliteral";

// --- GLOBAL VARIABLES ---
char* ssid = nullptr;
char* password = nullptr;
bool creds_available = false;
bool connected = false;
uint8_t brightness_val = 5;
const char *esp_ssid = "ESPclock_S3";
const char *esp_password = "waltwhite64";
bool newScan = false;
uint8_t attempts = 0;

AsyncWebServer server(80);

// Display State
bool colon = true;
bool blink_enabled = true;
bool br_auto = false;
bool start_NtpClient = false;

WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, 3600);
static const unsigned long NTP_UPDATE_MS = 15UL * 60UL * 1000UL; // 15 minutes
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, NTP_UPDATE_MS);

// --- TM1652 FUNCTIONS ---
uint8_t reverseByte(uint8_t b) {
  b = (b & 0b11110000) >> 4 | (b & 0b00001111) << 4;
  b = (b & 0b11001100) >> 2 | (b & 0b00110011) << 2;
  b = (b & 0b10101010) >> 1 | (b & 0b01010101) << 1;
  return b;
}

void tm1652_send(uint8_t data) {
  bool fParity = true;
  noInterrupts();
  digitalWrite(TM1652_SDA_Pin, LOW);
  delayMicroseconds(TM1652_BITDELAY);
  for (int nBit = 0; nBit < 8; nBit++) {
    if (data & 1) fParity = !fParity;
    digitalWrite(TM1652_SDA_Pin, (data & 1) ? HIGH : LOW);
    data >>= 1;
    delayMicroseconds(TM1652_BITDELAY);
  }
  digitalWrite(TM1652_SDA_Pin, fParity);
  delayMicroseconds(TM1652_BITDELAY);
  digitalWrite(TM1652_SDA_Pin, HIGH);
  interrupts();
  delayMicroseconds(TM1652_BITDELAY);
}

void tm1652_set_brightness(uint8_t brightness) {
  if (brightness < 1) brightness = 1;
  if (brightness > 8) brightness = 8;
  tm1652_send(0x18);
  tm1652_send(0x10 | (reverseByte(brightness - 1) >> 4 & 0x0f));
}

void tm1652_write_time(int hour, int minute, bool showColon) {
  tm1652_send(0x08);
  tm1652_send(dig_num[hour / 10]);
  tm1652_send(dig_num[hour % 10] | (showColon ? 0x80 : 0x00));
  tm1652_send(dig_num[minute / 10]);
  tm1652_send(dig_num[minute % 10]);
}

void tm1652_init() {
  pinMode(TM1652_SDA_Pin, OUTPUT);
  digitalWrite(TM1652_SDA_Pin, HIGH);
  delay(20);
  tm1652_set_brightness(5);
}

// --- UTILS ---
unsigned long myTimer(unsigned long everywhen) {
  static unsigned long t1;
  if (millis() - t1 >= everywhen) {
    t1 = millis();
    return 1;
  }
  return 0;
}

void wifiScan() {
  WiFi.scanDelete();
  delay(100);

  int n = WiFi.scanNetworks();
  int found = (n < 5) ? n : 5;

  DynamicJsonDocument net_list(1024);
  net_list["found"] = found;

  JsonArray network = net_list.createNestedArray("network");

  for (int j = 0; j < found; j++) {
    JsonObject item = network.createNestedObject();
    JsonArray credentials = item.createNestedArray("credentials");
    credentials.add(WiFi.SSID(j));
    credentials.add("");
  }

  File fx = FFat.open("/network_list.json", "w");
  if (fx) {
    serializeJson(net_list, fx);
    fx.close();
  }
}

bool saveWiFiCreds(const char* s, const char* p) {
  if (!s || !*s) return false;

  DynamicJsonDocument doc(256);
  doc["ssid"] = s;
  doc["pw"]   = (p ? p : "");

  File f = FFat.open(WIFI_CRED_PATH, "w");
  if (!f) return false;

  if (serializeJson(doc, f) == 0) {
    f.close();
    return false;
  }
  f.close();
  return true;
}

bool loadWiFiCreds(String &outSsid, String &outPw) {
  if (!FFat.exists(WIFI_CRED_PATH)) return false;

  File f = FFat.open(WIFI_CRED_PATH, "r");
  if (!f) return false;

  DynamicJsonDocument doc(256);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  // Correct ArduinoJson default usage
  outSsid = doc["ssid"] | "";
  outPw   = doc["pw"]   | "";

  return outSsid.length() > 0;
}

void setup() {
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  if (!FFat.begin(true)) Serial.println("FFat Mount Failed");

  WiFi.mode(WIFI_AP_STA);

  String savedSsid, savedPw;
  if (loadWiFiCreds(savedSsid, savedPw)) {
    if (ssid) free(ssid);
    if (password) free(password);
    ssid = strdup(savedSsid.c_str());
    password = strdup(savedPw.c_str());
    creds_available = true; // triggers WiFi.begin in loop()
  }

  WiFi.softAP(esp_ssid, esp_password);
  
  tm1652_init();
  wifiScan();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
    wifiScan();

    if (FFat.exists("/network_list.json")) {
      request->send(FFat, "/network_list.json", "application/json");
    } else {
      request->send(500, "application/json", "{\"error\":\"scan failed\"}");
    }
  });

  server.on("/refresh", HTTP_GET, [](AsyncWebServerRequest *request) {
    wifiScan();

    if (FFat.exists("/network_list.json")) {
      request->send(FFat, "/network_list.json", "application/json");
    } else {
      request->send(500, "application/json", "{\"error\":\"refresh failed\"}");
    }
  });

  server.on("/sendcreds", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

      DynamicJsonDocument doc(256);
      DeserializationError err = deserializeJson(doc, data, len);
      if (err) {
        request->send(400, "application/json", "{\"error\":\"bad json\"}");
        return;
      }

      if (ssid) free(ssid);
      if (password) free(password);
      ssid = strdup(doc["ssid"] | "");
      password = strdup(doc["pw"] | "");
      creds_available = true;

      saveWiFiCreds(ssid, password);

      request->send(200, "application/json", "{\"creds\":\"OK\"}");
    }
  );

  server.on("/wifi_status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", (WiFi.status() == WL_CONNECTED) ? "{\"stat\":\"ok\"}" : "{\"stat\":\"wait\"}");
  });

  server.on("/slider", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

      DynamicJsonDocument doc(128);
      if (deserializeJson(doc, data, len)) {
        request->send(400, "application/json", "{\"status\":\"bad json\"}");
        return;
      }

      int uiVal = doc["bgt"] | 0;      // UI slider: 0..7
      if (uiVal < 0) uiVal = 0;
      if (uiVal > 7) uiVal = 7;

      uint8_t tmVal = (uint8_t)uiVal + 1; // TM1652: 1..8
      brightness_val = tmVal;
      tm1652_set_brightness(tmVal);

      request->send(200, "application/json", "{\"status\":\"ok\"}");
    }
  );

  // (Add your other server handlers here following the same pattern)

  server.begin();
}

void loop() {
  if (newScan) { wifiScan(); newScan = false; }

  if (creds_available) {
    WiFi.begin(ssid, password);
    creds_available = false;
  }

  if (WiFi.status() == WL_CONNECTED && !connected) {
    connected = true;
    WiFi.mode(WIFI_STA); // Switch to Station only to save power/heat
    timeClient.begin();
    start_NtpClient = true;
  }

  if (start_NtpClient && myTimer(1000)) {
    timeClient.update();
    tm1652_write_time(timeClient.getHours(), timeClient.getMinutes(), colon);
    if (blink_enabled) colon = !colon;
  }

  delay(10);
}
