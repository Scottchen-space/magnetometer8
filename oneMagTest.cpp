#include <QMC5883LCompass.h>
#include <Wire.h>

// 定義腳位
// TXRX要交叉對接!!!
#define HC05_RX 27 
#define HC05_TX 26 

QMC5883LCompass compass1;

void setup() {
  Serial.begin(115200);
  
  // 強制使用 9600 鮑率，這是 HC-05 通訊模式的絕對標準值
  Serial2.begin(9600, SERIAL_8N1, HC05_RX, HC05_TX);  

  Wire.begin(21, 22);
  compass1.init();
  
  Serial.println("ESP32 System Ready.");
}

void loop() {
  float raw_x, raw_y, raw_z;
  float gauss_x, gauss_y, gauss_z;
  
  compass1.read();

  raw_x = compass1.getX();
  raw_y = compass1.getY();
  raw_z = compass1.getZ();

  gauss_x = raw_x / 3000.0;
  gauss_y = raw_y / 3000.0;
  gauss_z = raw_z / 3000.0;

  unsigned long t = millis(); 
  
  char buffer[64];  
  sprintf(buffer, "%-12lu %10.3f %10.3f %10.3f", t, gauss_x, gauss_y, gauss_z);
  
  Serial.println(buffer);   // 印給 USB 看
  Serial2.println(buffer);  // 印給藍牙看

  delay(500);
}