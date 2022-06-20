#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ESP8266Webhook.h>

// channel : MJU_IOT(1737977), device : MQTT_explorer
#define SECRET_MQTT_CLIENT_ID "EjolJjkAGiYCISw5AhQZKxw"
#define SECRET_MQTT_USERNAME "EjolJjkAGiYCISw5AhQZKxw"
#define SECRET_MQTT_PASSWORD "CCEYGlQuMgjHXaOpjDXIiK7L"

// define WiFi5
//#define WIFI_SSID "U+Net9700"
//#define WIFI_PWD  "HB797@0FE6"
//#define WIFI_SSID "ipTIME Guest1"
//#define WIFI_PWD  "12341234"
//#define WIFI_SSID "MJU_Wireless"
//#define WIFI_PWD  ""
#define WIFI_SSID "SJS"
#define WIFI_PWD  "19980304"

// define IFTTT
#define IFTTT_KEY_SJS "TAHf6d1iVRTvH1yfFsaBd"
#define IFTTT_EVENT "mom_sayed"

int MPU_Addr = 0x68;
int16_t AcZ;
unsigned long long prev = 0;
int cur = 0;
int before = 0;

WiFiClient myClient;
PubSubClient mqttClient;
HTTPClient httpClient;
Webhook webhook(IFTTT_KEY_SJS, IFTTT_EVENT);  // create an object

// MQTT Callback Function
void cbFunc(const char topic[], byte* data, unsigned int length)
{
  Serial.printf("call back function\r\n");
  char str[9];
  int i;
  for (i = 0; i <= (7 < length - 1 ? 7 : length - 1); i++) { // Read data to str
    str[i] = data[i];
  }
  str[i] = 0;
  int STOPGAME = atoi(str);

  if (STOPGAME == 1) {
    Serial.println("revceived STOPGAME..!");
    webhook.trigger();    // Send notification to child phone
  }
}

void setup()
{
  // Set UART
  Serial.begin(115200);
  delay(500);
  Serial.printf("UART OK\r\n");

  // Set 12C wake-up
  Wire.begin(4, 5);                 // Set as Master SDA : GPIO4, SCL : GPIO5
  Wire.beginTransmission(MPU_Addr); // Transmit to MPU6050
  Wire.write(0x6B);                 // Reg. Power Management
  Wire.write(1);                    // Wake-up MPU6050
  Wire.endTransmission();

  // Connect WiFi
  Serial.printf("WiFi connecting");
  WiFi.begin(WIFI_SSID, WIFI_PWD);              // connect to AP
  while ( !(WiFi.status() == WL_CONNECTED) ) {  // wait for connected
    Serial.printf(".");
    delay(500);
  }
  Serial.printf("\r\nWiFi Connected..!\r\n");

  // Connect MQTT
  Serial.println("MQTT Connect...");
  mqttClient.setClient(myClient);
  mqttClient.setServer("mqtt3.thingspeak.com", 1883);   // Set MQTT server and port
  mqttClient.setCallback(cbFunc);                       // Set Callback function
  int mqttConResult = mqttClient.connect(SECRET_MQTT_CLIENT_ID, SECRET_MQTT_USERNAME, SECRET_MQTT_PASSWORD);
  mqttClient.subscribe("channels/1737977/subscribe/fields/field2");        // Subscribe topic
  Serial.printf("MQTT Conn Result : %d\r\n", mqttConResult);
}

void loop()
{
  if (millis() - prev >= 10) {
    prev = millis();

    // Read MPU6050 AcZ
    Wire.beginTransmission(MPU_Addr);
    Wire.write(0x3F);           // Reg. ACCEL_XOUT
    Wire.endTransmission();

    Wire.requestFrom(MPU_Addr, 2, true); // Request 2Byte data
    AcZ = Wire.read() << 8 | Wire.read(); // ACCEL_ZOUT
//    Serial.printf("%d\r\n", AcZ);

    // Set shotgun 1 when gamer smashed the desk
    if (AcZ >= 20000) {
      cur = 1;
    }
    else {
      cur = 0;
    }

    if (cur == 1 && before == 0) {
      int shotgun = 1;
      Serial.printf("SHOTGUN..! : %d\r\n", AcZ);

      // publish to thingspeak server by MQTT
      char strBuf[80];
      sprintf(strBuf, "%d", shotgun);
      int pubResult = mqttClient.publish("channels/1737977/publish/fields/field1", strBuf);
      Serial.printf("Pubresult %d\r\n", pubResult);
    }
    before = cur;
  }
  mqttClient.loop();
}
