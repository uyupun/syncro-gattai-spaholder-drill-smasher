#include <M5CoreS3.h>
#include <math.h>
#include <NimBLEDevice.h>

#if defined(DEV_RED)
  static constexpr const char*    DEVICE_NAME  = "spaholder-drill-smasher-red";
  static constexpr uint16_t       DEVICE_COLOR = TFT_RED;
#elif defined(DEV_BLUE)
  static constexpr const char*    DEVICE_NAME  = "spaholder-drill-smasher-blue";
  static constexpr uint16_t       DEVICE_COLOR = TFT_BLUE;
#else
  #error "DEV_RED or DEV_BLUE must be defined"
#endif

const char* UUID_SVC_ACCEL = "11111111-2222-3333-4444-555555555555";
const char* UUID_CHR_ACCEL = "11111111-2222-3333-4444-666666666666";

const char* UUID_SVC_VIBRATOR = "22222222-3333-4444-5555-666666666666";
const char* UUID_CHR_VIBRATOR = "22222222-3333-4444-5555-777777777777";

constexpr int PB_OUT = 9;

static NimBLEServer*         g_server     = nullptr;
static NimBLEService*        g_svc_accel = nullptr;
static NimBLECharacteristic* g_chr_accel = nullptr;
static NimBLEService*        g_svc_vibrator = nullptr;
static NimBLECharacteristic* g_chr_vibrator = nullptr;

auto& Display = CoreS3.Display;
auto& Imu = CoreS3.Imu;

class VibratorWriteCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* chr, NimBLEConnInfo& conn_info) override {
    std::string value = chr->getValue();
    if (value.empty()) return;

    uint8_t strength = static_cast<uint8_t>(value[0]);
    Serial.printf("Vibrator strength: %d\n", strength);

    analogWrite(PB_OUT, strength);
    delay(1000);
    analogWrite(PB_OUT, 0);
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
  g_svc_vibrator = g_server->createService(UUID_SVC_VIBRATOR);
  g_chr_vibrator = g_svc_vibrator->createCharacteristic(
      UUID_CHR_VIBRATOR,
      NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  g_chr_vibrator->setCallbacks(new VibratorWriteCallbacks());

  auto adv = g_server->getAdvertising();
  adv->addServiceUUID(UUID_SVC_ACCEL);
  adv->addServiceUUID(UUID_SVC_VIBRATOR);

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

  Display.fillScreen(DEVICE_COLOR);
  Display.setTextColor(TFT_WHITE, DEVICE_COLOR);
  Display.setTextSize(3);
  Display.setCursor(20, 20);

  if (!Imu.begin()) {
    Display.setCursor(0, 0);
    Display.println("IMU init failed");
    Serial.println("IMU init failed");
    while (true) delay(100);
  }

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

  Display.fillScreen(DEVICE_COLOR);
  Display.setCursor(0, 0);
  Display.printf("X: %.3f g\n", ax);
  Display.printf("Y: %.3f g\n", ay);
  Display.printf("Z: %.3f g\n", az);

  delay(100);
}
