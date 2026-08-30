[![Static Badge](https://img.shields.io/badge/SLIM%20case-Makerworld-08bf08?style=for-the-badge&logo=bambulab)](https://makerworld.com/it/models/1594116-espclock-digital-clock#profileId-2069321)
[![Static Badge](https://img.shields.io/badge/BOLD%20case-Makerworld-08bf08?style=for-the-badge&logo=bambulab)](https://makerworld.com/it/models/2405754-espclock-bold-digital-clock)
[![Static Badge](https://img.shields.io/badge/BIG%20case-Makerworld-08bf08?style=for-the-badge&logo=bambulab)](https://makerworld.com/it/models/2616382-espclock-big-digital-clock#profileId-2887323)
![GitHub Repo stars](https://img.shields.io/github/stars/telepath9/ESPclock?style=for-the-badge&logo=github&color=A02ADB	)
[![Static Badge](https://img.shields.io/badge/Mentioned_by-Hackster.io-%232E9FE6?style=for-the-badge&logo=hackster&logoColor=white&color=%232E9FE6)](https://www.hackster.io/news/a-diy-retro-modern-alarm-clock-for-under-10-4e4e972345d8)
[![Static Badge](https://img.shields.io/badge/Mentioned_by-SeeedStudio-%232E9FE6?style=for-the-badge&color=rgb(0,58,74))](https://www.linkedin.com/posts/seeedstudio_seeedxiao-xiao-esp32-activity-7438192154243493888-qwSJ)



---

<img src="https://github.com/telepath9/ESPclock/blob/cb65eefd7f321586202e2b84a2ed587d0e3370f0/pics/font3.webp" alt="Alt Text" width="100%" height="100%">


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

3. Download the latest .zip file that you can find in the <a href="https://github.com/telepath9/ESPclock/releases"><b>Release</b></a> section, extract it, then open in VSCode/VSCodium the ESPclock project directory;

4. Select the PIOarduino extension in VSCode/VSCodium, then you have to store the html page into the esp8266 flash memory in two steps:
	<b>🔴(Be sure to have closed all Serial Monitor instances, otherwise it will fail to upload the fs image)</b>
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
<img src="https://github.com/telepath9/ESPclock/blob/54d7e278927e74feca1dcb77fd9ceac6a4dd744d/pics/2.4.0.webp" alt="Alt Text" width="100%" height="100%">

<h2>🤖 CURRENT FEATURES</h2>

- [x] Asynchronous webserver
- [x] Automatic Wifi Scan</li>
- [x] Wifi and NTP server setup via web UI
- [x] Save/delete configuration (used to restore data after reboot)
- [x] Automatic Brightness mode
- [x] Blinking colon toggle
- [x] ESP32 port
- [x] Replace NTPclient.h with time.h library
- [x] Timezone Support
- [x] ESPmDNS: web UI can be accessed by simply pasting "espclock.local" in the address bar of the browser.
- [x] 12-Hour clock mode
- [x] Alarm clock mode with buzzer
- [ ] Multiple Alarm (Requested)
- [x] Snooze feature for Alarm clock mode
- [x] Show Uptime in webUI
- [x] Show Local IP in webUI
- [x] Added TTP223 Touch button
- [ ] ESPhome or Homeassistant integration
- [ ] Show Temperature
- [x] TM1652 0.8" display support
- [ ] Web installer

> [!NOTE]
> About ESPmDNS: if more ESPclocks are connected to the same network, in order to access the web UI of the first one, user must enter "espclock.local" in the address bar, "espclock.local-2" to access to the web UI of second clock, and so on...

<h2>🛒 PART LIST</h2>
<details>
<summary><b>TM1637 Display Version</b></summary>
<ul>
<li>XIAO ESP32 C3 or Wemos D1 MINI (mine is V3.0.0 clone)</li>
<li>TM1637 module https://it.aliexpress.com/item/1005001582129952.html?spm=a2g0o.order_list.order_list_main.49.370e3696i4W9Sn&gatewayAdapt=glo2ita </li>
<li>Dupont wires</li>
<li>100 Ohm resistor: x1</li>
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
<li>TM1652 display https://it.aliexpress.com/item/1005007337668399.html</li>
<li>Dupont wires</li>
<li>100 Ohm resistor: x1</li>
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


<h2>🔌 BOARD WIRING</h2>
<details> 
<summary><b>XIAO ESP32 C3 - TM1652 0.8" Display</b></summary>
<img src="https://github.com/telepath9/ESPclock/blob/a6fae0d17180ef585f9a34ca514e80f5d7f3edf5/pics/tm1652_wiring_2.3.0.webp" alt="Alt Text" width="100%" height="100%">
</details>
<details> 
<summary><b>XIAO ESP32 C3 - TM1637 0.56" Display</b></summary>
<img src="https://github.com/telepath9/ESPclock/blob/fa035fd003b75fdb8b4b9cf68d04e8ba993e35fc/pics/xiao-top1.0.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/a6fae0d17180ef585f9a34ca514e80f5d7f3edf5/pics/tm1637_wiring_2.3.0.webp" alt="Alt Text" width="100%" height="100%">
</details>
<details> 
<summary><b>ESP8266 (Wemos D1 Mini 3.0.0) - TM1637 0.56" Display</b></summary> 
<img src="https://github.com/telepath9/ESPclock/blob/0defb72bb5107271487eb4c452812158a96b2c5e/pics/top_pcb.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/0defb72bb5107271487eb4c452812158a96b2c5e/pics/bottom.jpg" alt="Alt Text" width="100%" height="100%">
<img src="https://github.com/telepath9/ESPclock/blob/ebe4f234343fb306297fa49ef42fd830830b3c9a/pics/esp8266_pinout.jpg" alt="Alt Text" width="100%" height="100%">
</details>

<h2>🪛 HOW TO ASSEMBLE ESPclock</h2>
<img src="https://github.com/telepath9/ESPclock/blob/fc197cfd72fc6f489c0196dcbc484aab385d8b21/pics/howtoassemble.webp" alt="Alt Text" width="100%" height="100%">

<h2>👨‍👩‍👧‍👦 COMMUNITY BUILDS</h2>
<span>
<img src="https://github.com/telepath9/ESPclock/blob/f22a80e5382a0b89a9741d32ecaa0e95cc034884/pics/community_builds/2.webp" alt="Alt Text" width="50%" height="50%"> 
<img src="https://github.com/telepath9/ESPclock/blob/f22a80e5382a0b89a9741d32ecaa0e95cc034884/pics/community_builds/1.webp" alt="Alt Text" width="40%" height="40%">
<img src="https://github.com/telepath9/ESPclock/blob/main/pics/community_builds/3.webp" alt="Alt Text" width="40%" height="40%">
<img src="https://github.com/telepath9/ESPclock/blob/main/pics/community_builds/4.webp" alt="Alt Text" width="40%" height="40%">
</span>


<h2>🤝 CODE CONTRIBUTIONS</h2>
This is a personal project which i'm using to learn and to improve my coding skills. So, in order to follow my own plans, I won't accept any pull requests.
If you have any suggestion/advice/feedback, you can open an issue.
Forks are encouranged too, and i'm eager to see new changes that people can add to this project! 

<h2>❤️ SUPPORT</h2>
If you find ESPclock an interesting project, please consider to  <b>star</b> ⭐ the repository!<br>
To not miss the next updates and releases, be sure to click on the  <b>watch</b> 👁️ button!
If you want to financially contribute, contact me.

<h2>👁️ SHOW YOUR BUILD!</h2>
If you've assembled you own build (vanilla or custom), don't forget to post a picture of the final result! You can do it on MakerWorld or in the "Issues" section.

<h2>⚠️ TROUBLESHOOTING</h2>
There are (at the moment) two errors that can be displayed from the 7-segment display:
<ul>
<li>Err0 -> when "LittleFS.begin()" fails</li>
<li>Err1 -> when "index.html" doesn't exists in flash memory (user forgot to upload it)</li>
</ul>

<h2>🙋🏽 SPECIAL THANKS</h2>
To maxint-rd and his <a href="https://github.com/maxint-rd/TM16xx">TM16xx</a> library, the "swiss-army knife" for TM16xx chips.
