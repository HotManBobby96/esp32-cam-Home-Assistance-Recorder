#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "FS.h"
#include "SD_MMC.h"
#include "esp_camera.h"
#include <base64.h> // used globally like in the libraies folder
#include <string>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // unique codes 
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHUNK_SIZE 200 // could be bigger but safer qucik enough too 

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h" // used in our own project
#define FLASH_LED_PIN 4

bool deviceConnected = false;
String base64Image = "";
int offset = 0;

bool shouldSendChunks = false;

BLECharacteristic* pCharacteristic; // Global BLE characteristic pointer

// Class 1 
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
    offset = 0;
    base64Image = "";
    BLEDevice::startAdvertising();
    Serial.println(">> Advertising restarted");
  }
};



void sendChunk() {
  Serial.print("Running Chunk");
  if (!deviceConnected || base64Image.length() == 0) {
    Serial.print("!device || base64 image.length == 0");
    return;
  }

  int remaining = base64Image.length() - offset;

  if (remaining <= 0) {
    pCharacteristic->setValue("EOF"); // signifys when its done
    pCharacteristic->notify();
    offset = 0;
    base64Image = "";
    Serial.println("Finished sending image");
    return;
  }

  int len = min(CHUNK_SIZE, remaining);
  String chunk = base64Image.substring(offset, offset + len);
  pCharacteristic->setValue(chunk.c_str());
  pCharacteristic->notify();

  offset += len;
  delay(100); // Small delay to avoid flooding BLE stack
}

void captureAndEncode() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  base64Image = base64::encode((uint8_t*)fb->buf, fb->len);

  Serial.printf("Captured image. JPEG size: %d bytes, Base64 size: %d\n", fb->len, base64Image.length());
  Serial.print(base64Image);

  esp_camera_fb_return(fb);
  offset = 0;
  return;
}

// class thing for receiving data and lgoic mobob
// class 2
class CharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
  std::string rxValue = pCharacteristic->getValue().c_str();

  Serial.print("Received From client: ");
  for(int i = 0; i < rxValue.length(); i++) {
    Serial.println((uint8_t)rxValue[i], HEX);
    Serial.println(" ");
  } // for
    Serial.println();

    // logic down here for receiving exact key and capture, encode, and chunk (done) 
    pCharacteristic->setValue("");

  if (deviceConnected) { // fix this shit later
    if (base64Image.length() == 0) {
    captureAndEncode();
    delay(1000);
    }
    shouldSendChunks = true;
  } else {
    Serial.println("Waiting for connection...");
    delay(1000);
  } 
       
  } // void
}; // end of class



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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QQVGA; // 160x120
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
}

void startServer() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("esp32bryson");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->setCallbacks(new CharacteristicCallbacks()); // I think this is for reading the shit or something

  //pCharacteristic->setValue("Hello World says Neil");
  // I dont thnik that line is important (it wasnt. fucked me over multiple times, fuck neil from wherever the fuck i got that from)

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you  can read it in your phone!");
}

void setup() {
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  startServer();
  startCamera();
}

void loop() {   
  if (deviceConnected && shouldSendChunks) {
    sendChunk();
    if(base64Image.length() == 0) {
      shouldSendChunks = false;
    }
  }
  delay(100); // prevent cpu from gettiung jaked (pegged)
  Serial.println("Waiting for connection");
}
