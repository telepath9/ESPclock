Welcome to the page of my (W.I.P.) project called 

<img src="https://github.com/telepath9/ESPclock/blob/54c39c18d9204351de640fb42218651ce3664671/pics/newfont.png" alt="Alt Text" width="100%" height="100%">

which is a smart clock that connects to a NTP server to get the current time. 
It uses a ESP8266 (devboard: Wemos D1 mini, soon i will make it work also for ESP32).

After it has synced with NTP server for the first time, it can work also in OFFLINE mode (even without using RTC module, that was present in the older version, but then i removed in the v2.0).

<img src="https://github.com/telepath9/ESPclock/blob/6b2b2224a42deda354a8a7220dcf724643eb9c2e/pics/pic1.jpg" alt="Alt Text" width="100%" height="100%">

<h2>📜 INSTRUCTIONS </h2>

1. Install VSCode or <b>VSCodium</b> (i prefer VSCodium because it basically is VSCode but <b>without</b> telemetry);

2. Install PIOarduino extension on VSCode/VSCodium;

3. Download espclock.zip, extract it, then open in VSCode/VSCodium the espclock project directory;

4. Select the pioarduino extension in VSCode/VSCodium, then you have to store the html page into the esp8266 flash memory in two steps:
	1. Platform -> Build filesystem image
	2. Platform -> Upload filesystem image

5. Now lets upload the C/C++ code. Again from the pioarduino extension in VSCode/VSCodium, go to:
	1. General -> Build
	2. General -> Upload

6. Now let's connect pc or mobile to the Espclock access point, with the password: waltwhite64

7. After connecting to the AP, open your browser and paste in the address bar: http://192.168.4.1/

8. From the web interface, first add your home network, then the NTP server that matches your region, plus the GMT/UTC offset hour(s).

9. and we're done! Hope that you liked my project! If you have any suggestions, let me know!

<h2>📱 WEB UI </h2>
<img src="https://github.com/telepath9/ESPclock/blob/34c6915141348c3f2546474637ee8ee67e9d2076/pics/2.0.3.jpg" alt="Alt Text" width="100%" height="100%">

<h2>🤖 FEATURES </h2>
<ul>
<li>Asynchronous webserver</li>
<li>Automatic Wifi Scan</li>
<li>Wifi and NTP server setup via web UI</li>
<li>Automatic Brightness mode</li>
<li>Blinking colon toggle</li>
<li>... other features are coming soon!</li>
</ul>

<h2>🪛 PART LIST</h2>
<ul>
<li>Wemos D1 MINI (mine is V3.0.0 clone)</li>
<li>TM1637 module https://it.aliexpress.com/item/1005001582129952.html?spm=a2g0o.order_list.order_list_main.49.370e3696i4W9Sn&gatewayAdapt=glo2ita </li>
<li>Dupont wires</li>
<li>DG308 terminal block: x2</li>
<li>female 8Pin header: x2</li>
<li>7x3cm perfboard: x1</li>
<li>M2.5 nuts: x10</li>
<li>single row female 5 PINS (for common GND): x1</li>
</ul>


<h2>🔌 ESP8266 PINOUT (Wemos D1 Mini 3.0.0 devboard)</h2>
<img src="https://github.com/telepath9/ESPclock/blob/ebe4f234343fb306297fa49ef42fd830830b3c9a/pics/esp8266_pinout.jpg" alt="Alt Text" width="100%" height="100%">


<h2>⚠️ TROUBLESHOOTING</h2>
There are (at the moment) two errors that can be displayed from the 7-segment display:
<ul>
<li>Err0 -> when "LittleFS.begin()" fails</li>
<li>Err1 -> when "index.html" doesn't exists in flash memory (user forgot to upload it)</li>
</ul>
