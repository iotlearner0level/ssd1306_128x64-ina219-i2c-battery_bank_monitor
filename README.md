# ssd1306_128x64_i2c_battery_bank_monitor
Monitor 2x12V batteries connected to solar panels and publish data to thinkgspeak...

This projects uses a INA219 sensor connected via i2c to the esp8266 (wemos d1 mini, pins D1 & D2 SCL & SDA). Also connected is a 128x64 OLED to for realtime parameters.

The main sketch reads values obtained from the INA219 sensor to get V, i power readings. using a resistor divider, we get voltage from other series connected battery in the bank.

This data is published to thingspeak every few minutes and displayed on the OLED.


A webserver is also created which can be accessed from any web browser. As we are already doing quite a bit on this small microcontroller, it is run @160Mhz. The webpage is structured in a way to send some javascript code to the browser on the first run. 


On the next request, esp8266 sends values of various parameters like i,v,p for both the batteries using a json string to avoid data overheads.


Of course, using the dual cores of esp32 will make the sketch faster and more robust. 


There is a lot to improve upon, if you find it interesting or have any suggestions please do make a mention in the issues tab!

In this repo, please find .ino & headers file as usual with arduino. i'd edited the sketch with visual micro addon for visual basic community edition, it can be safely ignored. main advantage there was it directly allowed working from github.

thank you.
