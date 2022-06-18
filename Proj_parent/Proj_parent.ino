#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

WiFiClientSecure httpsClient;

#define WIFI_SSID "MJU_Wireless"
#define WIFI_PWD  ""
//#define WIFI_SSID "U+Net9700"
//#define WIFI_PWD  "HB797@0FE6"
//#define WIFI_SSID "ipTIME Guest1"
//#define WIFI_PWD  "12341234"

//LED 포트 번호
#define REDLED_PORT 13
#define GRNLED_PORT 15

DynamicJsonDocument doc(8192);

const char *host = "kr.api.riotgames.com";
const int httpsPort = 443;
String apiAddr = "/lol/summoner/v4/summoners/by-name/";
String summoner = "도비준";
String apiKey = "RGAPI-2b6e0c3f-3416-4563-af8d-7934e9acb8f3";

//접속 API용 변수들
String api_login = "/lol/spectator/v4/active-games/by-summoner/";
//String enc_id = "f0OBDw_rm2OjbLxm6TM-_S_21es6ZEF1jFtAxIfVYwe4hg";
String enc_id = "QJoWwcX_a6_ZKD1izgitCM4Es_PLAOa--wWcjsov8VbOBA";
int login_stat;
long long game_id = 0;

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
  delay(1000);
  
  // https connecting
  if(httpsClient.connect(host, httpsPort)){
    Serial.printf("Connected to web\r\n");
  }
  else{
    Serial.printf("Connection failed\r\n");
  }

  // Get data
  String link, link2;
  link = apiAddr + summoner + "?api_key=" + apiKey;
  link2 = api_login + enc_id + "?api_key=" + apiKey;

  //enc_id api
  /*
  httpsClient.print(String("GET ") + link + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");
  */
  //접속상태 api
  httpsClient.print(String("GET ") + link2 + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");
  
  Serial.println("request sent");
  
  //헤더 읽기(표시는 안함)
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  //Body 읽기
  Serial.println("reply was:");
  Serial.println("==========");
  String line;
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
  }
  Serial.printf("%d\r\n",login_stat);
  Serial.printf("%lld\r\n",game_id);
}

unsigned long long prev = 0;

void loop()
{
  
}
