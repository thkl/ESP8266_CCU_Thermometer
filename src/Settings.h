#include <EEPROM.h>

#define CONFIG_VERSION "AA2"
#define CONFIG_START 0


struct StoreStruct {
  // StoreStruct version
  char version[4];
  // Device settings:
  IPAddress ccu_ip;
  char ccu_variable[64];
  uint8_t resetCounter, wdtCounter;
}

deviceSettings = {
  CONFIG_VERSION,
  IPAddress(0,0,0,0),
  "ESP_Thermometer",
  0, 0
};

void eepromSave() {
  for (uint16_t t = 0; t < sizeof(deviceSettings); t++) {
    EEPROM.write(CONFIG_START + t, *((char*)&deviceSettings + t));
  }
  EEPROM.commit();
}

void eepromLoad() {
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2]) {

    StoreStruct tmpStore;
    tmpStore = deviceSettings;

    for (uint16_t t = 0; t < sizeof(deviceSettings); t++)
      *((char*)&deviceSettings + t) = EEPROM.read(CONFIG_START + t);

    if (deviceSettings.resetCounter >= 5 || deviceSettings.wdtCounter >= 10) {
      deviceSettings.wdtCounter = 0;
      deviceSettings.resetCounter = 0;
      deviceSettings = tmpStore;
    }

  } else {
    eepromSave();
    delay(500);

    ESP.eraseConfig();
    while(1);
  }
}
