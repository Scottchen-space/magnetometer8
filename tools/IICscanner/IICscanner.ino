#include <Wire.h>
#define TCAADDR 0x70

void tcaSelect(uint8_t bus) {
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << bus);
  Wire.endTransmission();
  delay(5);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Serial.println("\n--- 🚀 開始掃描 TCA9548A 所有通道 ---");

  for (uint8_t t = 0; t < 4; t++) {
    tcaSelect(t);
    Serial.printf("通道 CH%d: ", t);

    bool found = false;
    for (uint8_t addr = 1; addr < 127; addr++) {
      if (addr == TCAADDR) continue; // 跳過總機自己的位址

      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        Serial.printf("找到裝置 [0x%02X]  ", addr);
        found = true;
      }
    }
    if (!found) Serial.print("未偵測到任何 I2C 裝置");
    Serial.println();
  }
  Serial.println("--- 掃描結束 ---\n");
}

void loop() {}