/**
*
* To make this work connecte RST with GPIO16 (Pin D0)
*
*
* */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiManager.h>
#include "Settings.h"

// Time in seconds the ESP goes to sleep
#define SLEEP_TIME 600
#define HOSTNAME "ESP_Thermometer"

// data wire is connected with GPIO2:
#define ONE_WIRE_BUS 2

// if you want to monitor the battery level connect A0 thru 220kO with 3.3V
#define BATT_LEVEL A0
// if not set BATT_LEVEL to 0

#define RESET_CONFIG_PIN 4

// setup one wire instance:
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiManager wifiManager;
WiFiManagerParameter *ccu_ip;
WiFiManagerParameter *ccu_variable;

void configModeCallback (WiFiManager *myWiFiManager);
void saveConfigCallback();
void queryTemperature();
void queryBattery();
void regaCall(String url);

void setup()
{
  // set the blue led to off
  pinMode (LED_BUILTIN,OUTPUT);
  pinMode (RESET_CONFIG_PIN, INPUT);
  digitalWrite(LED_BUILTIN,HIGH);

  // start serial debug port
  Serial.begin(9600);
  Serial.println("");

  // start eeporom
  EEPROM.begin(512);

  bool rst = digitalRead(RESET_CONFIG_PIN);
  // Check the ResetPin
  // if set to 1 load config else erase the config and bring up the Portal
  if (rst == 1) {
    // load settings
    eepromLoad();
  } else {
    Serial.println("---- ");
    Serial.println("Reset button");
    Serial.println("---- ");
  }
  // some debug
  Serial.println("Settings are :");
  Serial.print("ccu_ip :");
  Serial.println(deviceSettings.ccu_ip);
  Serial.print("Variable :");
  Serial.println(deviceSettings.ccu_variable);

  //create wifi manager extra parametes
  ccu_ip =  new WiFiManagerParameter("CCU", "CCU IP", deviceSettings.ccu_ip.toString().c_str(), 40);
  ccu_variable =  new WiFiManagerParameter("CCUVariable", "CCU Variable", deviceSettings.ccu_variable, 40);

  //set config save callback (will call when the user saved the settings)
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.addParameter(ccu_ip);
  wifiManager.addParameter(ccu_variable);

  // check invalid IPs -> default settings
  if ((deviceSettings.ccu_ip == IPAddress(2,0,0,1)) || (deviceSettings.ccu_ip == IPAddress(0,0,0,0))) {
      Serial.println("Invalid CCU IP");
      // call the Config Mode
      wifiManager.startConfigPortal(HOSTNAME);
  } else {
      // seems legit connect to wifi
      wifiManager.autoConnect(HOSTNAME,NULL);
  }
  // initialize DS1820 sensor with 12 bit resolution:
  sensors.begin();
  sensors.setResolution(12);
  Serial.println("Query Sensor");
  queryTemperature();
  delay(500);
  queryBattery();
  delay(1000);
  Serial.println("Going to sleep");
  // sleeptime are microseconds so we have to multiply
  ESP.deepSleep(SLEEP_TIME * 1000000);
}

void configModeCallback (WiFiManager *myWiFiManager) {
 // enlight the blue led
 digitalWrite(LED_BUILTIN,LOW);
}


//check battery level and send to ccu
void queryBattery() {
  if (BATT_LEVEL != 0) {
    // template
    String requestUrl = "http://{ccuip}:8181/tclrega.exe?x=dom.GetObject(ID_SYSTEM_VARIABLES).Get(\"Bat_{varname}\").State({batt});";
    // load value from analog input
    double  batteryLevel = analogRead(BATT_LEVEL);
    // calculate voltage based on a 220k resistor
    batteryLevel = batteryLevel / 188.31;
    Serial.print("Battery :");
    Serial.println(batteryLevel);
    // replace values
    requestUrl.replace("{ccuip}", deviceSettings.ccu_ip.toString());
    requestUrl.replace("{varname}", String(deviceSettings.ccu_variable));
    requestUrl.replace("{batt}", String(batteryLevel));
    // send call
    regaCall(requestUrl);
  }
}

// make a http call
void regaCall(String url) {
  HTTPClient http;
  Serial.print("Url is:");
  Serial.println(url);
  // send request
  http.begin(url);
  http.GET();
  // end http
  http.end();
}


// request temperature from sensor and send this to ccu
void queryTemperature() {
  float temperature;
  // get the temperature
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  // ccu set variable template
  String requestUrl = "http://{ccuip}:8181/tclrega.exe?x=dom.GetObject(ID_SYSTEM_VARIABLES).Get(\"{varname}\").State({temp});";
  // replace variables
  requestUrl.replace("{ccuip}", deviceSettings.ccu_ip.toString());
  requestUrl.replace("{varname}", String(deviceSettings.ccu_variable));
  requestUrl.replace("{temp}", String(temperature));
  regaCall(requestUrl);
}

void saveConfigCallback () {
  IPAddress addr;
  Serial.println("Saving :");
  // generate ip from WifiManager Parameter
  if (addr.fromString(ccu_ip->getValue())) {
    deviceSettings.ccu_ip = addr;
  }

  //copy variable name
  strcpy(deviceSettings.ccu_variable,ccu_variable->getValue());

  Serial.print("ccu_ip :");
  Serial.println(deviceSettings.ccu_ip);
  Serial.print("Variable :");
  Serial.println(deviceSettings.ccu_variable);

  // save to eeprom
  eepromSave();
  // set the blue led off again
  digitalWrite(LED_BUILTIN,HIGH);

}

void loop()
{
}
