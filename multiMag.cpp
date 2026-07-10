#include <QMC5883LCompass.h>
#include <Wire.h>

// 定義腳位

// esp32接收腳 接HC-12發射腳TX 
#define HC12_RX 27 
// esp32發射腳 接HC-12接收腳
#define HC12_TX 26 
#define TCAadr 0x70
#define sensorCount 4

QMC5883LCompass compass;

struct Magnetometer {
  uint8_t ch;
  float x,y,z;
};

Magnetometer mySensors[] = {
  {0, 0.0, 0.0, 0.0},  // 第一顆：接在通道 0
  {1, 0.0, 0.0, 0.0},  // 第二顆：接在通道 1
  {2, 0.0, 0.0, 0.0},  // 第三顆：接在通道 2
  {3, 0.0, 0.0, 0.0}   // 第四顆：接在通道 3
};

// Select I2C BUS
void TCAselect(uint8_t bus){
  Wire.beginTransmission(TCAadr);  // TCA9548A address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

void  getSensorData(Magnetometer &sensor){
  TCAselect(sensor.ch);
  compass.read();

  sensor.x = compass.getX() / 3000.0;
  sensor.y = compass.getY() / 3000.0;
  sensor.z = compass.getZ() / 3000.0;
  // (gauss)
}

void setup() {
  Serial.begin(115200);
  // 設定HC-12通訊
  Serial2.begin(9600, SERIAL_8N1, HC12_RX, HC12_TX);  
  Wire.begin(21, 22);
  
  for (int i = 0; i < sensorCount; i++) {
    TCAselect(mySensors[i].ch);
    compass.init();
    Serial.printf("Sensor %d (CH%d) Initialized.\n", i + 1, mySensors[i].ch);
  }
  
  Serial.println("ESP32 System Ready.");
}

void loop() {
  unsigned long t = millis(); 
  char data[256];

  sprintf(data, "%lu", t);


  for (int i = 0; i < sensorCount; i++){
    getSensorData(mySensors[i]);

    char temp[64];
    sprintf(temp,",%.3f,%.3f,%.3f",mySensors[i].x, mySensors[i].y, mySensors[i].z);
    strcat(data,temp);
  }
  
  Serial.println(data);   // 印給 USB 看
  Serial2.println(data);  // 天線送出去

  delay(1000);
}