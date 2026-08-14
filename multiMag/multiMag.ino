#include <Adafruit_QMC5883P.h>
#include <Wire.h>

// 定義腳位

// esp32接收腳 接HC-12發射腳TX 
#define HC12_RX 27 
// esp32發射腳 接HC-12接收腳
#define HC12_TX 26 
#define TCAadr 0x70
#define sensorCount 8

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
  {3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {6, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {7, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
};

// Clock Generator for IIC bus hang
void clearI2CBus(int sda_pin, int scl_pin) {
  pinMode(sda_pin, INPUT_PULLUP);
  pinMode(scl_pin, INPUT_PULLUP);
  delay(1);

  // bus hang (SDA被鎖在LOW)
  if (digitalRead(sda_pin) == LOW) {
    // 把 SCL 設為輸出，準備手動送出9個Clock脈衝
    pinMode(scl_pin, OUTPUT);
    
    // 補足 9 個 Clock 能強迫 Slave 走完該幀通訊並釋放 SDA
    for (int i = 0; i < 9; i++) {
      digitalWrite(scl_pin, LOW);
      delayMicroseconds(5);
      digitalWrite(scl_pin, HIGH);
      delayMicroseconds(5);
      if (digitalRead(sda_pin) == HIGH) {
        break;
      }
    }
  }

  // STOP 訊號(SCL 為 High 時，SDA 由 Low 變 High)，讓所有 I2C 設備狀態機歸零
  pinMode(sda_pin, OUTPUT);
  digitalWrite(sda_pin, LOW);
  delayMicroseconds(5);
  digitalWrite(scl_pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(sda_pin, HIGH);
  delayMicroseconds(5);

  pinMode(sda_pin, INPUT);
  pinMode(scl_pin, INPUT);
}


// Select I2C BUS
void TCAselect(uint8_t bus){
  Wire.beginTransmission(TCAadr);  
  Wire.write(1 << bus);          
  Wire.endTransmission();
  delay(10); 
}

void  addSensorData(Magnetometer &sensor){
  TCAselect(sensor.ch);

  Wire.beginTransmission(0x2C);
  if(Wire.endTransmission() == 0){  // Wire.endTransmission() == 0 代表I2C傳輸成功(ACK)
    float gx = 0, gy = 0, gz = 0;

    if (qmc.getGaussField(&gx, &gy, &gz)) {
      // 執行讀取資料後判斷讀取成不成功
      sensor.sumX += gx;
      sensor.sumY += gy;
      sensor.sumZ += gz;
    }
  }
  else {
    Serial.printf(" [CH%d] I2C 讀取失敗!", sensor.ch);
  }
  
}

void setup() {
  delay(2000);

  Serial.begin(115200);
  // 設定HC-12通訊
  Serial2.begin(9600, SERIAL_8N1, HC12_RX, HC12_TX);  
  clearI2CBus(21, 22);
  Wire.begin(21, 22);
  Wire.setClock(50000); 
  
  for (int i = sensorCount - 1; i >= 0; i--) {
    TCAselect(mySensors[i].ch);
    
    
    
    bool initSuccess = false;
    
    // 嘗試初始化晶片，最多三次
    for (int retry = 0; retry < 3; retry++) {

      // 將磁力計重新開機
      Wire.beginTransmission(0x2C);
      Wire.write(0x0A); 
      Wire.write(0x80); 
      Wire.endTransmission();
      delay(100); // 等待晶片腦袋重新開機

      if (qmc.begin()) {
        qmc.setMode(QMC5883P_MODE_NORMAL);   
        qmc.setODR(QMC5883P_ODR_100HZ);   
        qmc.setOSR(QMC5883P_OSR_4);          
        qmc.setRange(QMC5883P_RANGE_2G);     
        qmc.setSetResetMode(QMC5883P_SETRESET_ON);
        
        initSuccess = true;
        break; 
      }
      delay(100); // 失敗的話等 100 毫秒再試一次
    }

    if (initSuccess) {
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
    // 讀取十次資料
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