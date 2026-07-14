/*
 *scanning IIC device address 
 *esp32 keeping calling address 
 *if address match IIC device reply and record the address
*/
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
  Serial.println("\n--- scanning IIC channel ---");

  for (uint8_t t = 0; t < 8; t++) {
    tcaSelect(t);
    Serial.printf("channel CH%d: ", t);

    bool found = false;
    for (uint8_t addr = 1; addr < 127; addr++) {
      if (addr == TCAADDR) continue; // 跳過總機自己的位址

      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        Serial.printf("find device address [0x%02X]  ", addr);
        found = true;
      }
    }
    if (!found) Serial.print("no IIC device found ");
    Serial.println();
  }
  Serial.println("--- end ---\n");
}

void loop() {}