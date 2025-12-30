Welcome to the page of my (W.I.P.) project called 

<img src="https://github.com/telepath9/ESPclock/blob/54c39c18d9204351de640fb42218651ce3664671/pics/newfont.png" alt="Alt Text" width="100%" height="100%">

which basically is a smart clock that connects to wifi to get the time. 
It relies on a ESP8266 (devboard: Wemos D1 mini, soon i will make it work also for ESP32).

At the actual state, it uses hardcoded wifi credentials (that the user must insert in the firmware before compiling it),
then, it retrieves the time (hour and minutes) from an NTP server.

After it has synced with NTP server for the first time, it can work also in OFFLINE mode, thanks to the RTC module, which can track the time without any connection to internet.

<img src="https://github.com/telepath9/ESPclock/blob/6b2b2224a42deda354a8a7220dcf724643eb9c2e/pics/pic1.jpg" alt="Alt Text" width="100%" height="100%">

🔜In the next release, the v2.0, user won't need to use hardcoded credentials anymore, plus there would be some other features so, 
stay tuned...

<h2>🛠️INSTRUCTIONS🤖</h2>

1. install VSCode or VSCodium (i prefer VSCodium because it basically is VSCode but WITHOUT telemetry);

2. install PIOarduino extension on VSCodium;

3. download the espclock.zip, extract it, then open in VSCodium the espclock project directory;

4. Select the pioarduino extension in VSCodium, then you have to store the html page into the esp8266 flash memory in two steps:
	1. Platform -> Build filesystem image
	2. Platform -> Upload filesystem image

5. Now lets upload the C/C++ code. Again from the pioarduino extension in VSCodium, go to:
	1. General -> Build
	2. General -> Upload

6. Now let's connect pc or mobile to the Espclock access point, with the password: supermario64

7. after connecting to the AP, open your browser and paste in the address bar: http://192.168.4.1/

8. From the web interface, first add your home network, then the NTP server that matches your region, plus the GMT/UTC offset hour(s).

9. aaand we're done! Hope that you liked my project! If you have any suggestions, let me know!


<img src="https://github.com/telepath9/ESPclock/blob/fc8962a5a3ac24c1769f0fc2af601251f94913a0/pics/v2.0_new.jpg" alt="Alt Text" width="100%" height="100%">
