#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266Webhook.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>

// channel : MJU_IOT(1737977), device : MQTT_4SJS
#define SECRET_MQTT_CLIENT_ID "CCIXBCQpODsWChQBCTAABho"
#define SECRET_MQTT_USERNAME "CCIXBCQpODsWChQBCTAABho"
#define SECRET_MQTT_PASSWORD "JInyrhufB+vNJfGmIf/6Tl8H"

WiFiClient myClient;
WiFiClientSecure httpsClient;
ESP8266WebServer httpServer(80);
PubSubClient mqttClient;

//#define WIFI_SSID "MJU_Wireless"
//#define WIFI_PWD  ""
#define WIFI_SSID "U+Net9700"
#define WIFI_PWD  "HB797@0FE6"
//#define WIFI_SSID "ipTIME Guest1"
//#define WIFI_PWD  "12341234"

#define IFTTT_WH_KEY "ctPKZOb5dEI3PetYB3amwa"

//LED 포트 번호
#define REDLED_PORT 13  //D7
#define GRNLED_PORT 15  //D8

DynamicJsonDocument doc(8192);
Webhook webhook(IFTTT_WH_KEY, "Proj_parent");

const char *host = "kr.api.riotgames.com";
const int httpsPort = 443;
String apiAddr = "/lol/summoner/v4/summoners/by-name/";
String apiKey = "RGAPI-2b6e0c3f-3416-4563-af8d-7934e9acb8f3";

String summoner = "섬%20벙"; //초기 추적 닉네임 '추적닉네임'
String summoner_show = "섬 벙"; //초기 추적 닉네임의 웹서버 표시명 '표시닉네임'
String summoner_new = ""; //새로 추적할 닉네임 '변경닉네임'

String line;

char tmpBuffer[2000];

//접속 API용 변수들
String api_login = "/lol/spectator/v4/active-games/by-summoner/";
String enc_id = "f0OBDw_rm2OjbLxm6TM-_S_21es6ZEF1jFtAxIfVYwe4hg"; //성범ID
int login_stat;
long long game_id = 0;

//MQTT Callback Function
void cbFunc(const char topic[], byte* data, unsigned int length) {
  Serial.printf("call back function\r\n");
  char str[9];
  int i;
  for(i=0; i<=(7<length-1?7:length-1); i++){   // Read data to str
    str[i] = data[i];
  }
  str[i] = 0; 
  int Shotgun = atoi(str);
  
  if(Shotgun == 1){       
    Serial.println("Shotgun enabled!");
    get_enc();
    get_login();
  }
}

//Root 페이지
void fnRoot(void) {
  //'변경닉네임'이 있을 경우 변경
  if (httpServer.hasArg("nickname")) {
    summoner_new = httpServer.arg("nickname");
    if (summoner_new != "") {         //'변경닉네임'이 빈 내용이 아닐경우
      summoner = summoner_new;        //'추적닉네임'에 삽입
      summoner_show = summoner_new;   //'표시닉네임'에 삽입
      summoner.replace(" ","%20");    //'추적닉네임'내 스페이스바(" ")를 "%20"으로 변환
    }
  }
  //Root 페이지 구성
  strcpy(tmpBuffer, "<meta charset=utf-8><html>\r\n");                    //한글표시가능
  strcat(tmpBuffer, "<img src=\"https:/");                                //이미지 출력
  strcat(tmpBuffer, "/d2u3dcdbebyaiu.cloudfront.net/uploads/atch_img/109/500e73934f2f3218accefd04c72bc6b3_crop.jpeg\"><br>\r\n");
  strcat(tmpBuffer, "엄마는 우리아들 믿어...<br>\r\n");                     //일반텍스트 출력
  strcat(tmpBuffer, "확인하려는 닉네임 :");                                 //일반텍스트 출력
  strcat(tmpBuffer, summoner_show.c_str());                               //'표시닉네임' 출력
  strcat(tmpBuffer, "<br>\r\n");
  strcat(tmpBuffer, "<form method=\"get\" action=\"\">");                 //'변경닉네임'용 input박스 생성
  strcat(tmpBuffer, "닉네임 <input type=\"text\" name=\"nickname\" >");
  strcat(tmpBuffer, "<input type=\"submit\" ></form>\r\n");
  strcat(tmpBuffer, "<a href=/status>게임 접속 확인</a><br>\r\n");          //status 페이지 링크
  strcat(tmpBuffer, "<a href=/sendMsg>중단 메세지 전송</a><br>\r\n");       //sendMsg 페이지 링크
  snprintf(tmpBuffer, sizeof(tmpBuffer), "%s%s", tmpBuffer, "</html>");
  httpServer.send(200, "text/html", tmpBuffer);
}
//추적 페이지
void fnStatus(void) {
  get_enc();
  get_login();
  //get_login을 통해 접속상태가 아닐때
  if (login_stat == 404) {
    strcpy(tmpBuffer, "<meta charset=utf-8><html>\r\n");                            //한글표시가능
    strcat(tmpBuffer, "<audio autoplay><source src=\"https:/");                     //사운드 자동출력
    strcat(tmpBuffer, "/s3.us-west-2.amazonaws.com/secure.notion-static.com/11343706-8edc-45ca-9076-64ae27b52f39/tcssca00.wav?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=AKIAT73L2G45EIPT3X45%2F20220619%2Fus-west-2%2Fs3%2Faws4_request&X-Amz-Date=20220619T074941Z&X-Amz-Expires=86400&X-Amz-Signature=361c789b8cd8e8de8d8ede01d4cea4163f64733feabb3ca6cf15063a7f8f6181&X-Amz-SignedHeaders=host&response-content-disposition=filename%20%3D%22tcssca00.wav%22&x-id=GetObject\" type=\"audio/wav\"></audio><br>\r\n");
    strcat(tmpBuffer, "자녀분이 게임을 하고있지'는' 않습니다<br>\r\n");                //일반텍스트 출력
    strcat(tmpBuffer, "<a href=/>돌아가기</a><br>\r\n");                             //Root 페이지 링크
    strcat(tmpBuffer, "<a href=/sendMsg>그래도 게임하지 말라고 알리기</a><br>\r\n");   //sendMsg 페이지 링크
    snprintf(tmpBuffer, sizeof(tmpBuffer), "%s%s", tmpBuffer, "</html>");
    httpServer.send(200, "text/html", tmpBuffer);
  }
  //get_login을 통해 접속상태일때
  if (game_id != 0) {
    strcpy(tmpBuffer, "<meta charset=utf-8><html>\r\n");                       //한글표시가능
    strcat(tmpBuffer, "<audio autoplay><source src=\"https:/");               //사운드 자동출력
    strcat(tmpBuffer, "/s3.us-west-2.amazonaws.com/secure.notion-static.com/11343706-8edc-45ca-9076-64ae27b52f39/tcssca00.wav?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=AKIAT73L2G45EIPT3X45%2F20220619%2Fus-west-2%2Fs3%2Faws4_request&X-Amz-Date=20220619T074941Z&X-Amz-Expires=86400&X-Amz-Signature=361c789b8cd8e8de8d8ede01d4cea4163f64733feabb3ca6cf15063a7f8f6181&X-Amz-SignedHeaders=host&response-content-disposition=filename%20%3D%22tcssca00.wav%22&x-id=GetObject\" type=\"audio/wav\"></audio><br>\r\n");
    strcat(tmpBuffer, "축하드립니다! 자녀분이 게임중입니다!<br>\r\n");           //일반텍스트 출력
    strcat(tmpBuffer, "<a href=/>돌아가기</a><br>\r\n");                       //Root 페이지 링크
    strcat(tmpBuffer, "<a href=/sendMsg>게임하지 말라고 알리기</a><br>\r\n");   //sendMsg 페이지 링크
    snprintf(tmpBuffer, sizeof(tmpBuffer), "%s%s", tmpBuffer, "</html>");
    httpServer.send(200, "text/html", tmpBuffer);
  }
  //둘다 아닐때 (get_enc에서 잘못 되거나, API key가 잘못되거나 등)
  else{
    strcpy(tmpBuffer, "<meta charset=utf-8><html>\r\n");                       //한글표시가능
    strcat(tmpBuffer, "<audio autoplay><source src=\"https:/");                //사운드 자동출력
    strcat(tmpBuffer, "/s3.us-west-2.amazonaws.com/secure.notion-static.com/11343706-8edc-45ca-9076-64ae27b52f39/tcssca00.wav?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=AKIAT73L2G45EIPT3X45%2F20220619%2Fus-west-2%2Fs3%2Faws4_request&X-Amz-Date=20220619T074941Z&X-Amz-Expires=86400&X-Amz-Signature=361c789b8cd8e8de8d8ede01d4cea4163f64733feabb3ca6cf15063a7f8f6181&X-Amz-SignedHeaders=host&response-content-disposition=filename%20%3D%22tcssca00.wav%22&x-id=GetObject\" type=\"audio/wav\"></audio><br>\r\n");
    strcat(tmpBuffer, "[오류]접속상태 확인 중 오류가 발생했습니다.<br>\r\n");     //일반텍스트 출력
    strcat(tmpBuffer, "<a href=/>돌아가기</a><br>\r\n");                       //Root 페이지 링크
    snprintf(tmpBuffer, sizeof(tmpBuffer), "%s%s", tmpBuffer, "</html>");
    httpServer.send(200, "text/html", tmpBuffer);
  }
}
//메세지 전송 페이지
void fnSendMsg(void) {
  strcpy(tmpBuffer, "<meta charset=utf-8><html>\r\n");                   //한글표시가능
  strcat(tmpBuffer, "<audio autoplay><source src=\"https:/");            //사운드 자동출력
  strcat(tmpBuffer, "/t1.daumcdn.net/cfile/tistory/99B972335F720D1502?original\"></audio><br>\r\n");
  strcat(tmpBuffer, "게임을 끄라는 알람을 보냈습니다.<br>\r\n");           //일반텍스트 출력
  strcat(tmpBuffer, "<a href=/>돌아가기</a><br>\r\n");                   //Root 페이지 링크
  snprintf(tmpBuffer, sizeof(tmpBuffer), "%s%s", tmpBuffer, "</html>");
  httpServer.send(200, "text/html", tmpBuffer);
  int pubResult = mqttClient.publish("channels/1737977/publish/fields/field2", "1");    //MQTT Field2에 PUB
  Serial.printf("pubResult : %d\r\n", pubResult);
}
//오류 페이지
void fnNotFound(void) {
  httpServer.send(404, "text/plain", "WRONG!!!");
}

//enc_id API
void get_enc(){
  digitalWrite(REDLED_PORT, LOW);
  digitalWrite(GRNLED_PORT, LOW);
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
  Serial.printf("%s\r\n", link.c_str());
  //헤더 읽기(표시는 안함)
  while (httpsClient.connected()) {
    line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  //Body 읽기
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
  Serial.printf("enc_id : %s\r\n",enc_id.c_str());
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
  Serial.printf("%s\r\n", link2.c_str());
  //헤더 읽기(표시는 안함)
  while (httpsClient.connected()) {
    line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  //Body 읽기
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
  mqttClient.setCallback(cbFunc);                     // Set Callback function
  int mqttConResult = mqttClient.connect(SECRET_MQTT_CLIENT_ID, SECRET_MQTT_USERNAME, SECRET_MQTT_PASSWORD);
  mqttClient.subscribe("channels/1737977/subscribe/fields/field1");        // Subscribe topic
  Serial.printf("MQTT Conn Result : %d\r\n", mqttConResult);
}

void loop()
{
  httpServer.handleClient();
  mqttClient.loop();
}
