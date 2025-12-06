#include <M5CoreS3.h>
#include <math.h>

auto& Display = CoreS3.Display;
auto& Imu = CoreS3.Imu;

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
}

void loop() {
  CoreS3.update();

  float ax, ay, az;
  Imu.getAccel(&ax, &ay, &az);

  // Serial.printf("X: %.3f\n", ax);
  Serial.printf("Y: %.3f\n", ay);
  // Serial.printf("Z: %.3f\n", az);

  Display.clear();
  Display.setCursor(0, 0);
  Display.printf("X: %.3f g\n", ax);
  Display.printf("Y: %.3f g\n", ay);
  Display.printf("Z: %.3f g\n", az);

  delay(100);
}
