Welcome to the page of my (W.I.P.) project called 

<img src="https://github.com/telepath9/ESPclock/blob/54c39c18d9204351de640fb42218651ce3664671/pics/newfont.png" alt="Alt Text" width="100%" height="100%">

<h1>v2.0</h1>
which is a smart clock that connects to a NTP server to get the current time. 
It uses a ESP8266 (devboard: Wemos D1 mini, soon i will make it work also for ESP32).

After it has synced with NTP server for the first time, it can work also in OFFLINE mode (even without using RTC module, that was present in the older version, but that i removed in the v2.0).

<img src="https://github.com/telepath9/ESPclock/blob/6b2b2224a42deda354a8a7220dcf724643eb9c2e/pics/pic1.jpg" alt="Alt Text" width="100%" height="100%">

<h2>🛠️ INSTRUCTIONS 🤖</h2>

1. Install VSCode or <b>VSCodium</b> (i prefer VSCodium because it basically is VSCode but <b>without</b> telemetry);

2. Install PIOarduino extension on VSCode/VSCodium;

3. Download espclock.zip, extract it, then open in VSCode/VSCodium the espclock project directory;

4. Select the pioarduino extension in VSCode/VSCodium, then you have to store the html page into the esp8266 flash memory in two steps:
	1. Platform -> Build filesystem image
	2. Platform -> Upload filesystem image

5. Now lets upload the C/C++ code. Again from the pioarduino extension in VSCode/VSCodium, go to:
	1. General -> Build
	2. General -> Upload

6. Now let's connect pc or mobile to the Espclock access point, with the password: supermario64

7. After connecting to the AP, open your browser and paste in the address bar: http://192.168.4.1/

8. From the web interface, first add your home network, then the NTP server that matches your region, plus the GMT/UTC offset hour(s).

9. and we're done! Hope that you liked my project! If you have any suggestions, let me know!


<img src="https://github.com/telepath9/ESPclock/blob/fc8962a5a3ac24c1769f0fc2af601251f94913a0/pics/v2.0_new.jpg" alt="Alt Text" width="100%" height="100%">

<h2>ESP8266 (Wemos D1 Mini 3.0.0 devboard) PINOUT</h2>
<img src="https://github.com/telepath9/ESPclock/blob/29cbf9b2a12f70d9c52a61b395483ac53f8dbaba/pics/esp8266_v2.0_pinout.jpg" alt="Alt Text" width="100%" height="100%">

