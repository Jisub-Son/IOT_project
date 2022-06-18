#include <Wire.h>

int MPU_Addr = 0x68;
int16_t AcZ;

unsigned long long prev = 0;
int cur = 0;
int before = 0;
int shotgun = 0;

void setup() 
{
  // Set UART
  Serial.begin(115200);       // set baud rate
  delay(1000);
  Serial.println("Hello");    // print "Hello"

  // Set 12C wake-up
  Wire.begin(4, 5);                 // Set as Master SDA : GPIO4, SCL : GPIO5
  Wire.beginTransmission(MPU_Addr); // transmit to MPU6050
  Wire.write(0x6B);                 // Reg. Power Management
  Wire.write(1);                    // wake-up MPU6050
  Wire.endTransmission();
}

void loop() 
{
  if(millis() - prev >= 10){
    prev = millis();
    
    Wire.beginTransmission(MPU_Addr);
    Wire.write(0x3F);           // Reg. ACCEL_XOUT  
    Wire.endTransmission();

    Wire.requestFrom(MPU_Addr, 2, true); // Request 2Byte data
    AcZ = Wire.read() << 8 | Wire.read(); // ACCEL_ZOUT
//    Serial.printf("%d\r\n", AcZ);

    if(AcZ >= 20000){
      cur = 1;
    }
    else{
      cur = 0;
    }
    
    if(cur == 1 && before == 0){
      shotgun = 1;
      Serial.printf("SHOTGUN..! : %d\r\n", AcZ);
    }
    before = cur;
    
  }
}
