#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "FS.h"
#include "SD_MMC.h"
#include "esp_camera.h"
#include <base64.h> // used globally like in the libraies folder

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHUNK_SIZE 200

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h" // used in our own project
#define FLASH_LED_PIN 4

bool deviceConnected = false;
String base64Image = "";
int offset = 0;


BLECharacteristic* pCharacteristic; // Global BLE characteristic pointer

class MyServerCallbacks : public BLEServerCallbacks { // class using esp32 ble
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println(">> A device has connected");
    digitalWrite(FLASH_LED_PIN, HIGH); // showing that something is connected personal use
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("<< A device has disconnected.");
    digitalWrite(FLASH_LED_PIN, LOW);
    offset = 0;
    base64Image = "";
    BLEDevice::startAdvertising(); // when disconncects restarts the server
    Serial.println(">> Advertising restarted");
  }
};

void sendChunk() {
  Serial.println("Running Chunk");
  if (!deviceConnected || base64Image.length() == 0) { // loggic to see if something went wrong
    Serial.println("!device || base64 image.length == 0");
    return;
  }

  int remaining = base64Image.length() - offset; // reducing offset to know how much is left

  if (remaining <= 0) {
    pCharacteristic->setValue("EOF"); // to signify when it ended
    pCharacteristic->notify();
    offset = 0; // resets everything
    base64Image = "";
    Serial.println("Finished sending image");
    return;
  }

  int len = min(CHUNK_SIZE, remaining);
  String chunk = base64Image.substring(offset, offset + len); // logic for what to send in that chunk
  pCharacteristic->setValue(chunk.c_str());
  pCharacteristic->notify();

  offset += len;
  delay(5); // so it dosnt overload the ble waves
}

void captureAndEncode() {
  delay(7500);
  camera_fb_t *fb = esp_camera_fb_get(); // take picture 
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  base64Image = base64::encode((uint8_t*)fb->buf, fb->len); // getting binary data, length of the data

  Serial.printf("Captured image. JPEG size: %d bytes, Base64 size: %d\n", fb->len, base64Image.length()); // prints out the base64 
  Serial.println(base64Image);

  esp_camera_fb_return(fb);
  offset = 0;
  return;
}



void startCamera() {
  camera_config_t config; // pins for my setup
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
    //config.frame_size = FRAMESIZE_QQVGA; // 160x120
    config.frame_size = FRAMESIZE_QVGA; // 320x x240
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
        //config.frame_size = FRAMESIZE_QQVGA; // 160x120
    config.frame_size = FRAMESIZE_QVGA; // 320x x240
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err); // error if setting up someting happend
    return;
  }
}

void startServer() {
  Serial.begin(115200); //serial port
  Serial.println("Starting BLE work!");

  BLEDevice::init("esp32bryson"); // name of device
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic( // deffining classes and what not
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12); // allocating bytes

  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your client deivce!");
}

void setup() {
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  startServer();
  startCamera();
}

void loop() {
  delay(1000);
  if (deviceConnected) {
    if (base64Image.length() == 0) {
    captureAndEncode();
    delay(1000);
    }
    sendChunk();
  } else {
    Serial.println("Waiting for connection...");
    delay(1000);
  }
}
