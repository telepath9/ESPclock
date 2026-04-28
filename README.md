[![Static Badge](https://img.shields.io/badge/SLIM_case-MakerWorld-%2308bf08?style=flat&logo=bambulab)](https://makerworld.com/it/models/1594116-espclock-digital-clock#profileId-2069321)
[![Static Badge](https://img.shields.io/badge/BOLD_case-MakerWorld-%2308bf08?style=flat&logo=bambulab)](https://makerworld.com/it/models/2405754-espclock-bold-digital-clock)
[![Static Badge](https://img.shields.io/badge/BIG_case-MakerWorld-%2308bf08?style=flat&logo=bambulab)](https://makerworld.com/it/models/2616382-espclock-big-digital-clock#profileId-2887323)

![GitHub Repo stars](https://img.shields.io/github/stars/telepath9/ESPclock?style=social&logo=github)

[![Static Badge](https://img.shields.io/badge/Mentioned_by-Hackster.io-%232E9FE6?style=flat&logo=hackster&logoColor=white&color=%232E9FE6)](https://www.hackster.io/news/a-diy-retro-modern-alarm-clock-for-under-10-4e4e972345d8)

<h2>⭐ BEFORE WE START...</h2>
If you find ESPclock an interesting project, please consider to  <b>star</b> ⭐ the repository!<br>
And if you don't want to miss the next updates and releases, be sure to click on the  <b>watch</b> 👁️ button!<br><br><br>


<img src="https://github.com/telepath9/ESPclock/blob/54c39c18d9204351de640fb42218651ce3664671/pics/newfont.png" alt="Alt Text" width="100%" height="100%">


is a smart clock that connects to a NTP server to get the current time. 
It uses a ESP8266 (Wemos D1 mini) or ESP32 (XIAO ESP32 C3)

<img src="https://github.com/telepath9/ESPclock/blob/092779388d558fc8df550596075db075423a6bf2/pics/big.jpeg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/6b2b2224a42deda354a8a7220dcf724643eb9c2e/pics/pic1.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/d3ea40822622406152dad12554adbceba28abb7e/pics/espclock_bold.jpg" alt="Alt Text" width="100%" height="100%">

<h2>📜 INSTRUCTIONS </h2>
<details>
<summary><b>(Click to expand)</b></summary>

1. Install VSCode or <b>VSCodium</b> (i prefer VSCodium because it basically is VSCode but <b>without</b> telemetry);

2. Install PIOarduino extension on VSCode/VSCodium;

3. Download the latest .zip file that you can find in the <a href="https://github.com/telepath9/ESPclock/releases">Release</a> section, extract it, then open in VSCode/VSCodium the ESPclock project directory;

4. Select the PIOarduino extension in VSCode/VSCodium, then you have to store the html page into the esp8266 flash memory in two steps:
	1. Platform -> Build filesystem image
	2. Platform -> Upload filesystem image

5. Now lets upload the C/C++ code. Again from the PIOarduino extension in VSCode/VSCodium, go to:
	1. General -> Build
	2. General -> Upload

6. Now let's connect PC or mobile to the ESPclock access point, with the password: waltwhite64

7. After connecting to the ESPclock's AP, open your browser and paste in the address bar: http://192.168.4.1/ or "espclock.local".

8. From the web UI, first add your home network, then the NTP server that matches your region, plus the GMT/UTC offset hour(s).

9. And we're done! Hope that you liked my project! If you have any suggestions, let me know!
</details>

<h2>📱 WEB UI </h2>
<img src="https://github.com/telepath9/ESPclock/blob/669b48f9ad6c6df066c71207a9129f1ba0dcdb98/pics/2.3.0.webp" alt="Alt Text" width="100%" height="100%">

<h2>🤖 CURRENT FEATURES</h2>

- [x] Asynchronous webserver
- [x] Automatic Wifi Scan</li>
- [x] Wifi and NTP server setup via web UI
- [x] Save/delete configuration (used to restore data after reboot)
- [x] Automatic Brightness mode
- [x] Blinking colon toggle
- [x] ESP32 port
- [x] Replace NTPclient.h with time.h library
- [x] ESPmDNS
- [x] 12-Hour clock mode
- [x] Alarm clock mode with buzzer
- [x] Snooze feature for Alarm clock mode
- [x] Display Uptime in webUI
- [ ] Timezone Support
- [x] Added TTP223 Touch button
- [ ] ESPhome or Homeassistant integration
- [ ] HA support
- [x] TM1652 0.8" display support	

<h2>🛒 PART LIST</h2>
<details>
<summary><b>TM1637 Display Version</b></summary>
<ul>
<li>XIAO ESP32 C3 or Wemos D1 MINI (mine is V3.0.0 clone)</li>
<li>TM1637 module https://it.aliexpress.com/item/1005001582129952.html?spm=a2g0o.order_list.order_list_main.49.370e3696i4W9Sn&gatewayAdapt=glo2ita </li>
<li>Dupont wires</li>
<li>DG308 7pins terminal block (for XIAO) OR DG308 8pins terminal block (for WEMOS): x2</li>
<li>Female 7pins header(for XIAO) OR Female 8pins header (for WEMOS): x2</li>
<li>7x3cm perfboard: x1</li>
<li>M2.5 screws: x10</li>
<li>M2.5 nuts: x2</li>
<li>Single row female 5pins header (for common GND): x1</li>
<li>Passive Buzzer: x1</li>
<li>(optional) TTP223 Touch button: x1</li>
</ul></details>

<details>
<summary><b>TM1652 Display Version</b></summary>
<ul>
<li>XIAO ESP32 C3 or Wemos D1 MINI (mine is V3.0.0 clone)</li>
<li>TM1652 display https://it.aliexpress.com/item/1005007337668399.html?spm=a2g0o.order_list.order_list_main.10.47683696Rt836v&gatewayAdapt=glo2ita</li>
<li>Dupont wires</li>
<li>DG308 7pins terminal block (for XIAO) OR DG308 8pins terminal block (for WEMOS): x2</li>
<li>Female 7pins header(for XIAO) OR Female 8pins header (for WEMOS): x2</li>
<li>7x3cm perfboard: x1</li>
<li>M3 screws: x4 (for display's mounting holes)</li>
<li>M2.5 screws: x6</li>
<li>M2.5 nuts: x2</li>
<li>Single row female 5pins header (for common GND): x1</li>
<li>Passive Buzzer: x1</li>
<li>(optional) TTP223 Touch button: x1</li>
</ul>
</details>


<h2>🔌 BOARD WIRING (buzzer and TTP223 coming soon...)</h2>
<details> 
<summary><b>XIAO ESP32 C3 - TM1652 0.8" Display</b></summary>
<img src="https://github.com/telepath9/ESPclock/blob/cab8b1194b71c58ab70c8662392b62cac567df08/pics/espclock_xiao%2Btm1652%2Bbuzzer%2Bttp223.webp" alt="Alt Text" width="100%" height="100%">
</details>
<details> 
<summary><b>XIAO ESP32 C3 - TM1637 0.56" Display</b></summary>
<img src="https://github.com/telepath9/ESPclock/blob/fa035fd003b75fdb8b4b9cf68d04e8ba993e35fc/pics/xiao-top1.0.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/ff830831e95c264ad7939fba758ce32de801831d/pics/xiao_top3_w.jpg" alt="Alt Text" width="100%" height="100%">
</details>
<details> 
<summary><b>ESP8266 (Wemos D1 Mini 3.0.0) - TM1637 0.56" Display</b></summary> 
<img src="https://github.com/telepath9/ESPclock/blob/0defb72bb5107271487eb4c452812158a96b2c5e/pics/top_pcb.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/0defb72bb5107271487eb4c452812158a96b2c5e/pics/bottom.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/ebe4f234343fb306297fa49ef42fd830830b3c9a/pics/esp8266_pinout.jpg" alt="Alt Text" width="100%" height="100%">
</details>

<h2>🪛 HOW TO ASSEMBLE ESPclock</h2>
<img src="https://github.com/telepath9/ESPclock/blob/fc197cfd72fc6f489c0196dcbc484aab385d8b21/pics/howtoassemble.webp" alt="Alt Text" width="100%" height="100%">



<h2>🤝 CODE CONTRIBUTIONS</h2>
This is a personal project which i'm using to learn and to improve my coding skills. So, in order to follow my own plans, I won't accept any pull request.
If you have any suggestion/advice/feedback, you can open an issue.
Forks are encouranged too, and i'm eager to see new changes that people can add to this project! 

<h2>❤️ SUPPORT</h2>
If you liked the project and want to financially contribute, you can buy me a coffee! <br><br>

[![Paypal Badge](https://img.shields.io/badge/Donate-PayPal-blue?logo=paypal)](https://www.paypal.com/donate/?hosted_button_id=NBJ3VHSWGQK7A)
[![Static Badge](https://img.shields.io/badge/Donate-LiberaPay-%23f6c915?style=flat&logo=liberapay&link=https%3A%2F%2Fliberapay.com%2Ftelepath%2Fdonate)](https://liberapay.com/telepath/donate)


<details> 
<summary><h2>⚠️ TROUBLESHOOTING</h2></summary>
There are (at the moment) two errors that can be displayed from the 7-segment display:
<ul>
<li>Err0 -> when "LittleFS.begin()" fails</li>
<li>Err1 -> when "index.html" doesn't exists in flash memory (user forgot to upload it)</li>
</ul>
</details>

<h2>🙋🏽 SPECIAL THANKS</h2>
To maxint-rd and his <a href="https://github.com/maxint-rd/TM16xx">TM16xx</a> library, the "swiss knife" for TM16xx chips.
