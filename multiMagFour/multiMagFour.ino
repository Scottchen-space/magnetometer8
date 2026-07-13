#include <Adafruit_QMC5883P.h>
#include <Wire.h>

// 定義腳位

// esp32接收腳 接HC-12發射腳TX 
#define HC12_RX 27 
// esp32發射腳 接HC-12接收腳
#define HC12_TX 26 
#define TCAadr 0x70
#define sensorCount 4

Adafruit_QMC5883P qmc;

unsigned long lastTime = 0;
int counter = 0;   //紀錄讀到多少次了

struct Magnetometer {
  uint8_t ch;
  float x,y,z;
  float sumX, sumY,sumZ;
};

Magnetometer mySensors[] = {
  {0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
};

// Select I2C BUS
void TCAselect(uint8_t bus){
  Wire.beginTransmission(TCAadr);  // TCA9548A address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  delay(2);
}

void  addSensorData(Magnetometer &sensor){
  TCAselect(sensor.ch);

  Wire.beginTransmission(0x2C);
  if(Wire.endTransmission() == 0){
    float gx = 0, gy = 0, gz = 0;

    if (qmc.getGaussField(&gx, &gy, &gz)) {
      // 3. 成功讀到後，直接加進這顆感測器的累加池！
      sensor.sumX += gx;
      sensor.sumY += gy;
      sensor.sumZ += gz;
    }
  }
  else {
    Serial.printf("⚠️ [CH%d] I2C 讀取失敗!", sensor.ch);
  }
  
}

void setup() {
  Serial.begin(115200);
  // 設定HC-12通訊
  Serial2.begin(9600, SERIAL_8N1, HC12_RX, HC12_TX);  
  Wire.begin(21, 22);
  
  for (int i = 0; i < sensorCount; i++) {
    TCAselect(mySensors[i].ch);
    
    // 初始化該通道的 QMC5883P [cite: 3]
    if (qmc.begin()) {
      qmc.setMode(QMC5883P_MODE_NORMAL);   
      qmc.setODR(QMC5883P_ODR_100HZ);   
      qmc.setOSR(QMC5883P_OSR_4);          
      qmc.setRange(QMC5883P_RANGE_2G);     
      qmc.setSetResetMode(QMC5883P_SETRESET_ON);
      
      Serial.printf("Sensor %d (CH%d) 初始化成功!\n", i + 1, mySensors[i].ch);
    } else {
      Serial.printf("Sensor %d (CH%d) 找不到晶片!\n", i + 1, mySensors[i].ch);
    }
  }
}

void loop() {
  unsigned long tNow = millis(); 
  
  if(tNow-lastTime >= 100){
    lastTime = tNow;
    
    for(int i = 0;i < sensorCount;i++){
      addSensorData(mySensors[i]);
    }

    counter++;

    if(counter >= 10){
      char data[256];

      sprintf(data, "%lu", tNow);


      for (int i = 0; i < sensorCount; i++){
        mySensors[i].x = mySensors[i].sumX/10.0;
        mySensors[i].y = mySensors[i].sumY/10.0;
        mySensors[i].z = mySensors[i].sumZ/10.0;

        char temp[64];
        sprintf(temp,",%.3f,%.3f,%.3f",mySensors[i].x, mySensors[i].y, mySensors[i].z);
        strcat(data,temp);

        mySensors[i].sumX = 0;
        mySensors[i].sumY = 0;
        mySensors[i].sumZ = 0;
      }
      Serial.println(data);   // 印給 USB 看
      Serial2.println(data);  // 天線送出去

      counter = 0;
    }
  }
}