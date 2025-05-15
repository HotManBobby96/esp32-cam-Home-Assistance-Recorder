#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "FS.h"
#include "SD.h"
#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// goofy ass logic
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"
#define FLASH_LED_PIN 4

bool deviceConnected = false;
String name = "esp32bryson";

const char base64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; // using this for the base 64

// CLASSES 
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println(">> A device has connected");
    digitalWrite(FLASH_LED_PIN, HIGH); 
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("<< A device has disconnected.");
    digitalWrite(FLASH_LED_PIN, LOW);
    
    // Just restart advertising
    BLEDevice::startAdvertising(); // starting the server after it is connected and disconnected
    Serial.println(">> Advertising restarted");
  }
};

// FUNCTIONS

void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    while (true);
  }
}

void takePicture() {
  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed"); // meaning no sd card
    return;
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed"); // issues taking the picture
    return;
  }

  File file = SD_MMC.open("/photo.jpg", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len); // Save image to SD card
    Serial.println("Saved file to /photo.jpg");
  }

  file.close();
  esp_camera_fb_return(fb);
}

void startServer() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init(name);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic =
    pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

// base 64 encoding

byte* readBinaryFile(const char* filename, size_t &length) {
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file");
    length = 0;
    return nullptr;
  }

  length = file.size();
  byte* buffer = (byte*)malloc(length);
  if (!buffer) {
    Serial.println("Memory allocation failed");
    file.close();
    return nullptr;
  }

  file.read(buffer, length);
  file.close();

  return buffer;
}

String base64Encode(const byte *data, size_t length) {
  String result;
  int val = 0, valb = -6;

  for (size_t i = 0; i < length; i++) {
    val = (val << 8) + data[i];
    valb += 8;
    while (valb >= 0) {
      result += base64Table[(val >> valb) & 0x3F];
      valb -= 6;
    }
  }

  if (valb > -6) result += base64Table[((val << 8) >> (valb + 8)) & 0x3F];
  while (result.length() % 4) result += '=';

  return result;
}

// make function that writes to the client with base64 code. 

// SETUP & LOOP
void setup() {
  startServer(); 

  // camera setup
  startCamera();

  pinMode(FLASH_LED_PIN, OUTPUT);

}

void loop() {
  if (deviceConnected) {
    Serial.println("Connected");
    takePicture();
  } else {
    Serial.println("Waiting for connection");
  }
  delay(1000); // Less spammy output
}
