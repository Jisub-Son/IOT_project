#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266Webhook.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>

// 지섭 1738848
#define SECRET_MQTT_USERNAME_SJS "Ix4oExgcHzgaIzkYCRYmGQ4"
#define SECRET_MQTT_CLIENT_ID_SJS "Ix4oExgcHzgaIzkYCRYmGQ4"
#define SECRET_MQTT_PASSWORD_SJS "nsLL7mJ1yCUQPdDmgkt5SKzO"

// 성범 1737977
#define SECRET_MQTT_CLIENT_ID "EjolJjkAGiYCISw5AhQZKxw"
#define SECRET_MQTT_USERNAME "EjolJjkAGiYCISw5AhQZKxw"
#define SECRET_MQTT_PASSWORD "CCEYGlQuMgjHXaOpjDXIiK7L"

WiFiClient myClient;
WiFiClientSecure httpsClient;
ESP8266WebServer httpServer(80);
PubSubClient mqttClient;

#define WIFI_SSID "MJU_Wireless"
#define WIFI_PWD  ""
//#define WIFI_SSID "U+Net9700"
//#define WIFI_PWD  "HB797@0FE6"
//#define WIFI_SSID "ipTIME Guest1"
//#define WIFI_PWD  "12341234"

#define IFTTT_WH_KEY "ctPKZOb5dEI3PetYB3amwa"

//LED 포트 번호
#define REDLED_PORT 13
#define GRNLED_PORT 15

DynamicJsonDocument doc(8192);
Webhook webhook(IFTTT_WH_KEY, "Proj_parent");

const char *host = "kr.api.riotgames.com";
const int httpsPort = 443;
String apiAddr = "/lol/summoner/v4/summoners/by-name/";
String summoner = "도비준";
String apiKey = "RGAPI-2b6e0c3f-3416-4563-af8d-7934e9acb8f3";

String line;

//접속 API용 변수들
String api_login = "/lol/spectator/v4/active-games/by-summoner/";
//String enc_id = "f0OBDw_rm2OjbLxm6TM-_S_21es6ZEF1jFtAxIfVYwe4hg"; //성범ID
String enc_id = "QJoWwcX_a6_ZKD1izgitCM4Es_PLAOa--wWcjsov8VbOBA"; //친구 ID
int login_stat;
long long game_id = 0;

void fnRoot(void)
{
  char tmpBuffer[2000];
  strcpy(tmpBuffer, "<html>\r\n");
  strcat(tmpBuffer, "I trust u Son..<br>\r\n");
  strcat(tmpBuffer, "<a href=/status>check game state</a><br>\r\n");
  strcat(tmpBuffer, "<a href=/sendMsg>send message</a><br>\r\n");
  snprintf(tmpBuffer, sizeof(tmpBuffer), "%s%s", tmpBuffer, "</html>");
  httpServer.send(200, "text/html", tmpBuffer);
}

void fnStatus(void)
{
  Serial.printf("status cb function\r\n");
  httpServer.send(200, "text/html", "Not yet");
}

void fnSendMsg(void)
{
  Serial.printf("sendMsg cb function\r\n");
  httpServer.send(200, "text/html", "send Message");
  mqttClient.publish("channels/1738848/publish/fields/field2", "1");
}

void fnNotFound(void)
{
  httpServer.send(404, "text/plain", "WRONG!!!");
}

//enc_id API
void get_enc(){
  // https connecting
  if(httpsClient.connect(host, httpsPort)){
    Serial.printf("Connected to web\r\n");
  }
  else{
    Serial.printf("Connection failed\r\n");
  }
  String link = apiAddr + summoner + "?api_key=" + apiKey;

  httpsClient.print(String("GET ") + link + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");
  Serial.println("request sent");
  //헤더 읽기(표시는 안함)
  while (httpsClient.connected()) {
    line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  //Body 읽기
  Serial.println("reply was:");
  Serial.println("==========");
  //첫줄 받아서 넘기기
  line = httpsClient.readStringUntil('\n');  //Read Line by Line
  Serial.println(line); //Print response
  //둘째줄 JSON 해체
  line = httpsClient.readStringUntil('\n');  //Read Line by Line
  Serial.println(line); //Print response
  deserializeJson(doc, line.c_str());
  login_stat = doc["status"]["status_code"];
  game_id = doc["gameId"];
  const char *enc_id_c = doc["id"];
  String enc_id_load(enc_id_c);
  enc_id = enc_id_load;
  //나머지 넘기기
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");

  Serial.printf("%s\r\n",enc_id.c_str());
}
//접속상태 API
void get_login(){
  // https connecting
  if(httpsClient.connect(host, httpsPort)){
    Serial.printf("Connected to web\r\n");
  }
  else{
    Serial.printf("Connection failed\r\n");
  }
  String link2 = api_login + enc_id + "?api_key=" + apiKey;

  httpsClient.print(String("GET ") + link2 + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");
  Serial.println("request sent");
  //헤더 읽기(표시는 안함)
  while (httpsClient.connected()) {
    line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  //Body 읽기
  Serial.println("reply was:");
  Serial.println("==========");
  //첫줄 받아서 넘기기
  line = httpsClient.readStringUntil('\n');  //Read Line by Line
  Serial.println(line); //Print response
  //둘째줄 JSON 해체
  line = httpsClient.readStringUntil('\n');  //Read Line by Line
  Serial.println(line); //Print response
  deserializeJson(doc, line.c_str());
  login_stat = doc["status"]["status_code"];
  game_id = doc["gameId"];
  //나머지 넘기기
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");
  
  //접속상태 확인
  if(login_stat == 404){
    Serial.println("자녀분이 게임을 하고있지'는' 않습니다");
    digitalWrite(GRNLED_PORT,HIGH);
  }
  if(game_id != 0){
    Serial.println("축하드립니다! 자녀분이 게임중입니다!");
    digitalWrite(REDLED_PORT,HIGH);
    webhook.trigger();
  }
  Serial.printf("%d\r\n",login_stat);
  Serial.printf("%lld\r\n",game_id);
}


void setup()
{
  // Set UART
  Serial.begin(115200);
  delay(500);
  Serial.printf("UART OK\r\n");

  //LED 설정
  pinMode(GRNLED_PORT,OUTPUT);
  pinMode(REDLED_PORT,OUTPUT);
  
  // Connect WiFi
  Serial.printf("WiFi connecting");
  WiFi.begin(WIFI_SSID, WIFI_PWD);              // connect to AP
  while( !(WiFi.status() == WL_CONNECTED) ){    // wait for connected
    Serial.printf(".");
    delay(500);
  }
  Serial.printf("\r\nWiFi Connected..!\r\n");
  httpsClient.setInsecure();
  delay(500);
  
  // Get local IP
  Serial.printf("Please contact IP Addr...");
  Serial.println(WiFi.localIP());

  // Callback functions & start web server
  httpServer.on("/", fnRoot);
  httpServer.on("/status", fnStatus);
  httpServer.on("/sendMsg", fnSendMsg);
  httpServer.onNotFound(fnNotFound);
  httpServer.begin();
  
  // Connect MQTT 
  Serial.println("MQTT Connect...");
  mqttClient.setClient(myClient);
  mqttClient.setServer("mqtt3.thingspeak.com", 1883);  // Set MQTT server and port
//  mqttClient.setCallback(cbFunc);                     // Set Callback function  
  int mqttConResult = mqttClient.connect(SECRET_MQTT_CLIENT_ID_SJS, SECRET_MQTT_USERNAME_SJS, SECRET_MQTT_PASSWORD_SJS);
//  mqttClient.subscribe("channels/1738848/subscribe/fields/field2");        // Subscribe topic
  Serial.printf("MQTT Conn Result : %d\r\n", mqttConResult);
  
 get_enc();
 delay(500);
 get_login();
}

unsigned long long prev = 0;

void loop()
{
  httpServer.handleClient();
}
