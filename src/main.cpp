#include <M5CoreS3.h>

auto& Display = CoreS3.Display;

void setup() {
  auto cfg = M5.config();
  CoreS3.begin(cfg);

  Serial.begin(115200);
  Serial.println("hello, world!");

  Display.fillScreen(BLACK);
  Display.setTextColor(TFT_WHITE, TFT_BLACK);
  Display.setTextSize(3);
  Display.setCursor(20, 20);
  Display.println("hello, world!");
}

void loop() {
  CoreS3.update();
  delay(100);
}
