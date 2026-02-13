Welcome to the page of my (W.I.P.) project called 

<img src="https://github.com/telepath9/ESPclock/blob/54c39c18d9204351de640fb42218651ce3664671/pics/newfont.png" alt="Alt Text" width="100%" height="100%">

which is a smart clock that connects to a NTP server to get the current time. 
It uses a ESP8266 (devboard: Wemos D1 mini, soon i will make it work also for ESP32).

<img src="https://github.com/telepath9/ESPclock/blob/6b2b2224a42deda354a8a7220dcf724643eb9c2e/pics/pic1.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/d3ea40822622406152dad12554adbceba28abb7e/pics/espclock_bold.jpg" alt="Alt Text" width="100%" height="100%">
(ESPclock <b>BOLD</b> coming soon... 😉)

<h2>📜 INSTRUCTIONS </h2>

1. Install VSCode or <b>VSCodium</b> (i prefer VSCodium because it basically is VSCode but <b>without</b> telemetry);

2. Install PIOarduino extension on VSCode/VSCodium;

3. Download the latest .zip file that you can find in the "Release" section, extract it, then open in VSCode/VSCodium the ESPclock project directory;

4. Select the PIOarduino extension in VSCode/VSCodium, then you have to store the html page into the esp8266 flash memory in two steps:
	1. Platform -> Build filesystem image
	2. Platform -> Upload filesystem image

5. Now lets upload the C/C++ code. Again from the PIOarduino extension in VSCode/VSCodium, go to:
	1. General -> Build
	2. General -> Upload

6. Now let's connect PC or mobile to the ESPclock access point, with the password: waltwhite64

7. After connecting to the ESPclock's AP, open your browser and paste in the address bar: http://192.168.4.1/

8. From the web UI, first add your home network, then the NTP server that matches your region, plus the GMT/UTC offset hour(s).

9. And we're done! Hope that you liked my project! If you have any suggestions, let me know!

<h2>📱 WEB UI </h2>
<img src="https://github.com/telepath9/ESPclock/blob/34c6915141348c3f2546474637ee8ee67e9d2076/pics/2.0.3.jpg" alt="Alt Text" width="100%" height="100%">

<h2>🤖 FEATURES </h2>
<ul>
<li>Asynchronous webserver</li>
<li>Automatic Wifi Scan</li>
<li>Wifi and NTP server setup via web UI</li>
<li>Automatic Brightness mode</li>
<li>Blinking colon toggle</li>
</ul>

<h2>🧰 NEW FEATURES TO ADD</h2>
<ul>
<li>🔳 ESP32 port</li>
<li>🔳 Replace NTPclient.h with ESP standard NTP functionality (no libraries to include!)<\li>	
<li>🔳 Alarm clock mode with buzzer</li>
<li>🔳 Display uptime in webUI</li>
<li>🔳 Remember/forget configuration and credentials</li>
<li>🔳 Adding 1 or 2 buttons to the case</li>
<li>🔳 ESPhome or Homeassistant integration</li>
<li>🔳 HA support</li>
<li>...</li>
</ul>

<h2>🛒 PART LIST</h2>
<ul>
<li>Wemos D1 MINI (mine is V3.0.0 clone)</li>
<li>TM1637 module https://it.aliexpress.com/item/1005001582129952.html?spm=a2g0o.order_list.order_list_main.49.370e3696i4W9Sn&gatewayAdapt=glo2ita </li>
<li>Dupont wires</li>
<li>DG308 8pins terminal block: x2</li>
<li>Female 8pins header: x2</li>
<li>7x3cm perfboard: x1</li>
<li>M2.5 screws: x10</li>
<li>M2.5 nuts: x2</li>
<li>Single row female 5pins header (for common GND): x1</li>
</ul>


<h2>🔌 ESP8266 WIRING (Wemos D1 Mini 3.0.0)</h2>
<img src="https://github.com/telepath9/ESPclock/blob/0defb72bb5107271487eb4c452812158a96b2c5e/pics/top_pcb.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/0defb72bb5107271487eb4c452812158a96b2c5e/pics/bottom.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/ebe4f234343fb306297fa49ef42fd830830b3c9a/pics/esp8266_pinout.jpg" alt="Alt Text" width="100%" height="100%">

<h2>🪛 HOW TO ASSEMBLE ESPclock</h2>
<img src="https://github.com/telepath9/ESPclock/blob/fc197cfd72fc6f489c0196dcbc484aab385d8b21/pics/howtoassemble.webp" alt="Alt Text" width="100%" height="100%">

<h2>❤️ SUPPORT</h2>
If you liked the project and want to financially contribute, you can buy me a coffee! <br><br>

[![Paypal Badge](https://img.shields.io/badge/Donate-PayPal-blue?logo=paypal)](https://www.paypal.com/donate/?hosted_button_id=NBJ3VHSWGQK7A)

<h2>⚠️ TROUBLESHOOTING</h2>
There are (at the moment) two errors that can be displayed from the 7-segment display:
<ul>
<li>Err0 -> when "LittleFS.begin()" fails</li>
<li>Err1 -> when "index.html" doesn't exists in flash memory (user forgot to upload it)</li>
</ul>

