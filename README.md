Welcome to the page of my (W.I.P.) project called <h2>ESPclock</h2>
which basically is a smart clock that connects to wifi to synchronize the time. 
At the actual state, it uses hardcoded wifi credentials (that the user must insert in the firmware before compiling it),
then, it retrieves the time (hour and minutes) from an NTP server.

After it has synced with NTP server for the first time, it can work also in OFFLINE mode, thanks to the RTC module, which can track the time without any connection to internet.

<img src="https://github.com/telepath9/ESPclock/blob/6b2b2224a42deda354a8a7220dcf724643eb9c2e/pics/pic1.jpg" alt="Alt Text" width="80%" height="80%">

🔜In the next release, the v2.0, user won't need to use hardcoded credentials anymore, plus there would be some other features so, 
stay tuned...

<img src="https://github.com/telepath9/ESPclock/blob/5e1feb3c85a5e5c88398d0bdf8993236540c67b5/pics/demo.png" alt="Alt Text" width="50%" height="50%">
