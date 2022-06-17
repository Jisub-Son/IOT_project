#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266HTTPClient.h>

WiFiClientSecure httpsClient;

#define WIFI_SSID "U+Net9700"
#define WIFI_PWD  "HB797@0FE6"
//#define HOST      "kr.api.riotgames.com"
//#define PORT      443
//#define API_ADDR  "/lol/summoner/v4/summoners/by-name/"
//#define SUMMONER  "54720857del"
//#define API_KEY   "RGAPI-2b6e0c3f-3416-4563-af8d-7934e9acb8f3"

const char *host = "kr.api.riotgames.com";
const int httpsPort = 443;
String apiAddr = "/lol/summoner/v4/summoners/by-name/";
String summoner = "54720857del";
String apiKey = "RGAPI-2b6e0c3f-3416-4563-af8d-7934e9acb8f3";

// SHA1 fingerprint
const char fingerprint[] PROGMEM = "2d 1c aa 62 c5 2f c7 0d 44 50 1a 1c b4 1b 45 f9 9e 27 a4 c7";

void setup()
{
  // Set UART
  Serial.begin(115200);
  delay(500);
  Serial.printf("UART OK\r\n");

  // Connect WiFi
  Serial.printf("WiFi connecting");
  WiFi.begin(WIFI_SSID, WIFI_PWD);              // connect to AP
  while( !(WiFi.status() == WL_CONNECTED) ){    // wait for connected
    Serial.printf(".");
    delay(500);
  }
  Serial.printf("\r\nWiFi Connected..!\r\n");

  // set fingerprint and timeout
  Serial.printf("Using fingerprint '%s'\r\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);

  // https connecting
  if(httpsClient.connect(host, httpsPort)){
    Serial.printf("Connected to web\r\n");
  }
  else{
    Serial.printf("Connection failed\r\n");
  }

  // Get data
  String link;
  link = "/lol/summoner/v4/summoners/by-name/도비준?api_key=RGAPI-2b6e0c3f-3416-4563-af8d-7934e9acb8f3";

  httpsClient.print(String("GET ") + link + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");
  
  Serial.println("request sent");
  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("reply was:");
  Serial.println("==========");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");
}

unsigned long long prev = 0;

void loop()
{
  
}
