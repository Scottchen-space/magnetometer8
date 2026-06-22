#include <QMC5883LCompass.h>
#include <Wire.h>

QMC5883LCompass compass1;


void setup() {
  Serial.begin(115200);
  Wire.begin(21,22);
  compass1.init();
}

void loop() {
  float raw_x, raw_y, raw_z;
  float gauss_x, gauss_y, gauss_z;
  
  compass1.read();

  // Return XYZ readings
  raw_x = compass1.getX();
  raw_y = compass1.getY();
  raw_z = compass1.getZ();

  gauss_x = raw_x/3000;
  gauss_y = raw_y/3000;
  gauss_z = raw_z/3000;

  //Serial.printf("X:%.3f, Y:%.3f, Z:%.3f\n", gauss_x, gauss_y, gauss_z);

  unsigned long t = millis(); 
  
  char buffer[64];  // 定義字元陣列大小
  sprintf(buffer, "%-12lu %10.3f %10.3f %10.3f", t, gauss_x, gauss_y, gauss_z);
  Serial.println(buffer);

  delay(100);

}
