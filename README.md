# ESP8266_CCU_Thermometer

Compiling
=========
PlatformIO: this project is ready to use on PlatformIO.

ArduinoIDE: src/* is what you need. You have to add WifiManager and DallasTemperature to your libraries.


Config
======
Just connect with your phone to the AccessPoint ESP_Thermometer.

You have to setup your Wifi, CCU IP and variable name.

CCU
===
Create a variable of type number for saving the current temperature. (eg. ESP_Thermometer).

To monitor the current voltage connect A0 via a 220kO resistor to 3.3V . To save this value to ccu create a 2nd variable with the same name but a "Bat_" Prefix (eg. Bat_ESP_Thermometer). If you don't want to monitor the voltage set BATT_LEVEL to 0.

Sleep
=====
After saving the current temperature the ESP will go to deep sleep for 60 minutes to save battery life.

You may change the sleep time via SLEEP_TIME variable.


Reset To default : Press and hold the ResetSettings Button (connected to PIN D2) and reset the ESP (or powercycle)

Todo
====
* config for sleep time
* config for battery level
* create ccu variables if they are not present
