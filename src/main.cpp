#include <M5CoreS3.h>
#include <math.h>
#include <NimBLEDevice.h>

const char* DEVICE_NAME = "spaholder-drill-smasher";

const char* UUID_SVC_ACCEL = "11111111-2222-3333-4444-555555555555";
const char* UUID_CHR_ACCEL = "11111111-2222-3333-4444-666666666666";

const char* UUID_SVC_WATER_PUMP = "22222222-3333-4444-5555-666666666666";
const char* UUID_CHR_WATER_PUMP = "22222222-3333-4444-5555-777777777777";

constexpr int PB_OUT = 9;

static NimBLEServer*         g_server     = nullptr;
static NimBLEService*        g_svc_accel = nullptr;
static NimBLECharacteristic* g_chr_accel = nullptr;
static NimBLEService*        g_svc_water_pump = nullptr;
static NimBLECharacteristic* g_chr_water_pump = nullptr;

auto& Display = CoreS3.Display;
auto& Imu = CoreS3.Imu;

class WaterPumpWriteCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* chr, NimBLEConnInfo& conn_info) override {
    std::string value = chr->getValue();
    String command = value.c_str();

    Serial.println(command);

    digitalWrite(PB_OUT, HIGH);
    delay(3000);
    digitalWrite(PB_OUT, LOW);
  }
};

void setup_ble() {
  NimBLEDevice::init(DEVICE_NAME);
  NimBLEDevice::setSecurityAuth(false, false, false);

  g_server = NimBLEDevice::createServer();
  g_server->advertiseOnDisconnect(true);

  g_svc_accel = g_server->createService(UUID_SVC_ACCEL);
  g_chr_accel = g_svc_accel->createCharacteristic(
      UUID_CHR_ACCEL,
      NIMBLE_PROPERTY::NOTIFY
  );
  g_svc_accel->start();

  g_svc_water_pump = g_server->createService(UUID_SVC_WATER_PUMP);
  g_chr_water_pump = g_svc_water_pump->createCharacteristic(
      UUID_CHR_WATER_PUMP,
      NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  g_chr_water_pump->setCallbacks(new WaterPumpWriteCallbacks());
  g_svc_water_pump->start();

  auto adv = g_server->getAdvertising();
  adv->addServiceUUID(UUID_SVC_ACCEL);
  adv->addServiceUUID(UUID_SVC_WATER_PUMP);

  NimBLEAdvertisementData adv_data;
  adv_data.setName(DEVICE_NAME);
  adv_data.setCompleteServices(BLEUUID(UUID_SVC_ACCEL));
  adv->setAdvertisementData(adv_data);

  NimBLEAdvertisementData scan_data;
  scan_data.setName(DEVICE_NAME);
  scan_data.addServiceUUID(UUID_SVC_ACCEL);
  adv->setScanResponseData(scan_data);

  adv->start();
}

void setup() {
  auto cfg = M5.config();
  CoreS3.begin(cfg);

  Serial.begin(115200);

  pinMode(PB_OUT, OUTPUT);
  digitalWrite(PB_OUT, LOW);

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
  Imu.getAccel(&ax, &ay, &az); // X: 左右, Y: 前後, Z: 上下

  float accel[3] = {ax, ay, az};
  g_chr_accel->setValue((uint8_t*)accel, sizeof(accel));
  g_chr_accel->notify();

  Serial.printf("X: %.3f, Y: %.3f, Z: %.3f\n", ax, ay, az);

  Display.clear();
  Display.setCursor(0, 0);
  Display.printf("X: %.3f g\n", ax);
  Display.printf("Y: %.3f g\n", ay);
  Display.printf("Z: %.3f g\n", az);

  delay(100);
}
