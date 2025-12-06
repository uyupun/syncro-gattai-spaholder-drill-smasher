#include <M5CoreS3.h>
#include <math.h>
#include <NimBLEDevice.h>

const char* DEVICE_NAME = "uyupun-drill";
const char* UUID_SVC_ACCEL_Y = "11111111-2222-3333-4444-555555555555";
const char* UUID_CHR_ACCEL_Y = "11111111-2222-3333-4444-666666666666";
const char* UUID_SVC_WATER_PUMP = "22222222-3333-4444-5555-666666666666";
const char* UUID_CHR_WATER_PUMP = "22222222-3333-4444-5555-777777777777";

static NimBLEServer*         g_server     = nullptr;
static NimBLEService*        g_svc_accel_y = nullptr;
static NimBLECharacteristic* g_chr_accel_y = nullptr;
static NimBLEService*        g_svc_water_pump = nullptr;
static NimBLECharacteristic* g_chr_water_pump = nullptr;

auto& Display = CoreS3.Display;
auto& Imu = CoreS3.Imu;

class WaterPumpWriteCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* chr, NimBLEConnInfo& conn_info) override {
    std::string value = chr->getValue();
    String command = value.c_str();

    Serial.print("Water pump command received: ");
    Serial.println(command);

    // ここで直接ウォーターポンプの制御処理を実行
    // 例: if (command == "ON") { /* ポンプON処理 */ }
    //     if (command == "OFF") { /* ポンプOFF処理 */ }
  }
};

void setup_ble() {
  NimBLEDevice::init(DEVICE_NAME);
  NimBLEDevice::setSecurityAuth(false, false, false);

  g_server = NimBLEDevice::createServer();
  g_server->advertiseOnDisconnect(true);

  g_svc_accel_y = g_server->createService(UUID_SVC_ACCEL_Y);
  g_chr_accel_y = g_svc_accel_y->createCharacteristic(
      UUID_CHR_ACCEL_Y,
      NIMBLE_PROPERTY::NOTIFY
  );
  g_svc_accel_y->start();

  g_svc_water_pump = g_server->createService(UUID_SVC_WATER_PUMP);
  g_chr_water_pump = g_svc_water_pump->createCharacteristic(
      UUID_CHR_WATER_PUMP,
      NIMBLE_PROPERTY::WRITE
  );
  g_chr_water_pump->setCallbacks(new WaterPumpWriteCallbacks());
  g_svc_water_pump->start();

  auto adv = g_server->getAdvertising();
  adv->addServiceUUID(UUID_SVC_ACCEL_Y);
  adv->addServiceUUID(UUID_SVC_WATER_PUMP);

  NimBLEAdvertisementData adv_data;
  adv_data.setName(DEVICE_NAME);
  adv_data.setCompleteServices(BLEUUID(UUID_SVC_ACCEL_Y));
  adv->setAdvertisementData(adv_data);

  NimBLEAdvertisementData scan_data;
  scan_data.setName(DEVICE_NAME);
  scan_data.addServiceUUID(UUID_SVC_ACCEL_Y);
  adv->setScanResponseData(scan_data);

  adv->start();
}

void setup() {
  auto cfg = M5.config();
  CoreS3.begin(cfg);

  Serial.begin(115200);

  Display.fillScreen(TFT_BLACK);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.setTextSize(3);
  Display.setCursor(20, 20);

  if (!Imu.begin()) {
    Display.setCursor(0, 0);
    Display.println("IMU init failed");
    Serial.println("IMU init failed");
    while (true) delay(100);
  }

  randomSeed((uint32_t)esp_random());

  setup_ble();
}

void loop() {
  CoreS3.update();

  float ax, ay, az;
  Imu.getAccel(&ax, &ay, &az);

  g_chr_accel_y->setValue((uint8_t*)&ay, sizeof(ay));
  g_chr_accel_y->notify();

  // Serial.printf("X: %.3f\n", ax);
  // Serial.printf("Y: %.3f\n", ay);
  // Serial.printf("Z: %.3f\n", az);

  Display.clear();
  Display.setCursor(0, 0);
  Display.printf("X: %.3f g\n", ax);
  Display.printf("Y: %.3f g\n", ay);
  Display.printf("Z: %.3f g\n", az);

  delay(100);
}
