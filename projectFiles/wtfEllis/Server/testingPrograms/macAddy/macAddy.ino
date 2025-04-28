#include "esp_bt_device.h"
#include "esp_bt_main.h"

void setup() {
  Serial.begin(115200);
  delay(2000);  // Give time for Serial to initialize

  Serial.println("Booting...");

  // Start Bluetooth controller
  if (!btStart()) {
    Serial.println("Failed to start Bluetooth controller.");
    return;
  }
  Serial.println("Bluetooth controller started.");

  // Init and enable Bluedroid stack (needed for MAC access)
  if (esp_bluedroid_init() != ESP_OK) {
    Serial.println("Failed to init Bluedroid.");
    return;
  }
  Serial.println("Bluedroid initialized.");

  if (esp_bluedroid_enable() != ESP_OK) {
    Serial.println("Failed to enable Bluedroid.");
    return;
  }
  Serial.println("Bluedroid enabled.");

  // Now get the MAC
  const uint8_t* btMac = esp_bt_dev_get_address();
  if (btMac == nullptr) {
    Serial.println("Failed to get BT MAC address.");
    return;
  }

  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          btMac[0], btMac[1], btMac[2], btMac[3], btMac[4], btMac[5]);

  Serial.println("Bluetooth MAC address:");
  Serial.println(macStr);
}

void loop() {
  // Nothing
}
