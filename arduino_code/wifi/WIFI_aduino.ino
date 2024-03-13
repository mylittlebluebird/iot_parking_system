/*
  WiFiEsp test: ClientTest
  http://www.kccistc.net/
  작성일 : 2023.11.16
  작성자 : IoT 임베디드 KSH
*/
//#define DEBUG
//#define DEBUG_WIFI

#include <WiFiEsp.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <DHT.h>

#define AP_SSID "SEMICON_2.4G"
#define AP_PASS "a1234567890"
#define SERVER_NAME "10.10.52.212"
#define SERVER_PORT 5000
#define LOGID "parking_db"
#define LOGINID "aduino1"
#define PASSWD "PASSWD"
#define TRIG 12
#define ECHO 11
#define TRIGF 9
#define ECHOF 8
#define CDS_PIN A0
#define MOTOR_PIN 5
#define BUTTON_PIN 2

#define LED_LAMP_PIN 3  //Relay On/Off
#define DHTPIN 4
#define WIFIRX 6  //6:RX-->ESP8266 TX
#define WIFITX 7  //7:TX -->ESP8266 RX
#define LED_BUILTIN_PIN 10
#define LED_PIN 13

#define CMD_SIZE 50
#define ARR_CNT 5
#define DHTTYPE DHT11
bool timerIsrFlag = false;
int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50; 
boolean ledOn = false;      // LED의 현재 상태 (on/off)
boolean cdsFlag = false;
boolean sonic_Flag = false;
boolean sonic_S_Flag = false;
int car_re_num=0;
int con_humi = 0;
int con_illu = 0;
int con_temp = 0;
int secCount=0;

//char sendId[10] = "KSH_ARD";
char sendBuf[CMD_SIZE];
char lcdLine1[17] = "IoT By team_2 ";
char lcdLine2[17] = "WiFi Connecting!";
int mysonicTime;
int cds = 0;
int sonic;
int sonic_F;
int motor_speed = 255;
char getSensorId[10];
int sensorTime;
float temp = 0.0;
float humi = 0.0;
bool updatTimeFlag = false;
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
} DATETIME;
DATETIME dateTime = {0, 0, 0, 12, 0, 0};
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial wifiSerial(WIFIRX, WIFITX);
WiFiEspClient client;
LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup() {
  lcd.init();
  lcd.backlight();
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);

  pinMode(TRIG, OUTPUT);    // 초음파 센서 trig을 출력으로 설정 
  pinMode(ECHO, INPUT);    // 초음파 센서 echo을 입력으로 설정 
  pinMode(TRIGF, OUTPUT);    // 초음파 센서 trig_front을 출력으로 설정 
  pinMode(ECHOF, INPUT);    // 초음파 센서 echo_front을 입력으로 설정 

  pinMode(CDS_PIN, INPUT);    // a0 핀을 입력으로 설정 (생략 가능)
  pinMode(MOTOR_PIN, OUTPUT);    // a3 핀을 출력으로 설정 (생략 가능)
  pinMode(BUTTON_PIN, INPUT);    // 버튼 핀을 입력으로 설정 (생략 가능)
  pinMode(LED_LAMP_PIN, OUTPUT);    // LED 핀을 출력으로 설정
  pinMode(LED_BUILTIN_PIN, OUTPUT); //D13
  pinMode(LED_PIN, OUTPUT); //D13


  Serial.begin(115200); //DEBUG

  wifi_Setup();
  MsTimer2::set(1000, timerIsr); // 1000ms period
  MsTimer2::start();

  dht.begin();
}
void ultrasonic();
void loop() {

  if (client.available()) {
    socketEvent();
  }
  if (timerIsrFlag) //1초에 한번씩 실행
  {
    timerIsrFlag = false;
    sonic=ultrasonic(TRIG,ECHO);
    sonic_F=ultrasonic(TRIGF,ECHOF);
 
    if(sonic<=4 && sonic_Flag==false)  // 주차공간 위의 초음파센서
    {
      //  mysonicTime = millis();
        digitalWrite(LED_PIN,HIGH);
        sprintf(sendBuf, "[%s]PLACE@1F11@%d\n", LOGID, 1);
        client.write(sendBuf, strlen(sendBuf));
        client.flush();
        sonic_Flag = true;
     
    }
    else if(sonic<=4 && sonic_Flag==true)
    {
     //   if(millis()-mysonicTime>10000) //  일정시간 후 불 꺼줌.
     //     digitalWrite(LED_BUILTIN_PIN,LOW);
        
    }
    else if(sonic>=5 && sonic_Flag==true)
    {
  
        sprintf(sendBuf, "[%s]PLACE@1F11@%d\n", LOGID, 0);
        client.write(sendBuf, strlen(sendBuf));
        client.flush();
        sonic_Flag = false;
         digitalWrite(LED_PIN,LOW);
    }
    if(sonic_F<16 && sonic_S_Flag ==false) // 통로쪽 초음파센서
    {
        digitalWrite(LED_LAMP_PIN,HIGH);
        sprintf(sendBuf, "[%s]PLACE@1F1S@%d\n", LOGID, 1);
        client.write(sendBuf, strlen(sendBuf));
        client.flush();
        sonic_S_Flag = true;
    }
    else if(sonic_F<15 && sonic_S_Flag ==true)    // 
    {
    }
    else if(sonic_F>=15 && sonic_S_Flag ==false)    // 
    {
    }
    else if(sonic_F>=17 && sonic_S_Flag ==true)    // 
    {
        digitalWrite(LED_LAMP_PIN,LOW);
        sprintf(sendBuf, "[%s]PLACE@1F1S@%d\n", LOGID, 0);
        client.write(sendBuf, strlen(sendBuf));
        client.flush();
        sonic_S_Flag = false;
    }
      cds = map(analogRead(CDS_PIN), 0, 1023, 0, 100);
      if ((cds >= con_illu) && cdsFlag)
      {
        cdsFlag = false;
        //sprintf(sendBuf, "[%s]CDS@%d\n", LOGID, cds);
       // client.write(sendBuf, strlen(sendBuf));
       // client.flush();
         digitalWrite(LED_BUILTIN_PIN, LOW);     // LED 상태 변경
      } else if ((cds < con_illu) && !cdsFlag)
      {
        cdsFlag = true;
        //sprintf(sendBuf, "[%s]CDS@%d\n", LOGID, cds);
        //client.write(sendBuf, strlen(sendBuf));
        //client.flush();
        digitalWrite(LED_BUILTIN_PIN, HIGH);     // LED 상태 변경
      }
      humi = dht.readHumidity();
      temp = dht.readTemperature();
      if((int)humi<con_humi)
        motor_speed =0;
        
      analogWrite(MOTOR_PIN,motor_speed);

     

    if (!client.connected()) {
        lcdDisplay(0, 1, "Server Down");
        server_Connect();
      }
    
    if (sensorTime != 0 && !(secCount % sensorTime ))
    {
      sprintf(sendBuf, "[%s]SENSOR@%d@%d@%d\n", LOGID, cds,(int)temp,(int)humi);
      /*      char tempStr[5];
            char humiStr[5];
            dtostrf(humi, 4, 1, humiStr);  //50.0 4:전체자리수,1:소수이하 자리수
            dtostrf(temp, 4, 1, tempStr);  //25.1
            sprintf(sendBuf,"[%s]SENSOR@%d@%s@%s\r\n",getSensorId,cdsValue,tempStr,humiStr);
      */
      client.write(sendBuf, strlen(sendBuf));
      client.flush();
    }
  

      sprintf(lcdLine1, "%02d.%02d  %02d:%02d:%02d", dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec );
      lcdDisplay(0, 0, lcdLine1);

      if(updatTimeFlag)
      {
        client.print("[GETTIME]\n");
        updatTimeFlag=false;
      }
    }
   
  buttonDebounce();
}
void socketEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len;

  sendBuf[0] = '\0';
  len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
  client.flush();

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  if(!strcmp(pArray[0],"aduino1"))
  {
    if(strncmp(pArray[1]," Alr",4))
      return;
    
  //  Serial.print("aduino1 returned");
    

  }
  //[KSH_ARD]LED@ON : pArray[0] = "KSH_ARD", pArray[1] = "LED", pArray[2] = "ON"
  if ((strlen(pArray[1]) + strlen(pArray[2])) < 16)
  {
    if(!strncmp(pArray[1], "CAR",3))
    {
      sprintf(lcdLine2, "parking : %d", car_re_num);  
      
    }
    else
    {
      sprintf(lcdLine2, "%s %s", pArray[1], pArray[2]);
    }
    lcdDisplay(0, 1, lcdLine2);
  }
  if (!strncmp(pArray[1], " New", 4)) // New Connected
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    strcpy(lcdLine2, "Server Connected");
    lcdDisplay(0, 1, lcdLine2);
    updatTimeFlag = true;
    return ;
  }
  else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    client.stop();
    server_Connect();
    updatTimeFlag = true;
    
    return ;
  }
  else if (!strncmp(pArray[1], "GETSE",5)) {
    sensorTime = atoi(pArray[2]);
    return;
  }
  else if (!strncmp(pArray[1], "temp",4)) {
    con_temp = atoi(pArray[2]);
    return;
  }
  else if (!strncmp(pArray[1], "humi",4)) {
    con_humi=atoi(pArray[2]);
    return;
  }
  else if (!strncmp(pArray[1], "illu",4)) {
    con_illu = atoi(pArray[2]);
    return;
  }
  else if (!strcmp(pArray[1], "CAR")) {
    car_re_num = 15- atoi(pArray[2]);
    return;
   
  }
  
  else if(!strcmp(pArray[0],"GETTIME")) {  //GETTIME
    dateTime.year = (pArray[1][0]-0x30) * 10 + pArray[1][1]-0x30 ;
    dateTime.month =  (pArray[1][3]-0x30) * 10 + pArray[1][4]-0x30 ;
    dateTime.day =  (pArray[1][6]-0x30) * 10 + pArray[1][7]-0x30 ;
    dateTime.hour = (pArray[1][9]-0x30) * 10 + pArray[1][10]-0x30 ;
    dateTime.min =  (pArray[1][12]-0x30) * 10 + pArray[1][13]-0x30 ;
    dateTime.sec =  (pArray[1][15]-0x30) * 10 + pArray[1][16]-0x30 ;
    return;
  } 
  else
    return;

  client.write(sendBuf, strlen(sendBuf));
  client.flush();

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif
}
void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
  clock_calc(&dateTime);
}
void clock_calc(DATETIME *dateTime)
{
  int ret = 0;
  dateTime->sec++;          // increment second

  if(dateTime->sec >= 60)                              // if second = 60, second = 0
  { 
      dateTime->sec = 0;
      dateTime->min++; 
             
      if(dateTime->min >= 60)                          // if minute = 60, minute = 0
      { 
          dateTime->min = 0;
          dateTime->hour++;                               // increment hour
          if(dateTime->hour == 24) 
          {
            dateTime->hour = 0;
            updatTimeFlag = true;
          }
       }
    }
}
void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x, y);
  lcd.print(str);
  for (int i = len; i > 0; i--)
    lcd.write(' ');
}
void buttonDebounce() {
  int i;
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        ledOn = !ledOn;                        // LED 상태 값 반전
        digitalWrite(LED_BUILTIN_PIN, ledOn);  // LED 상태 변경
        sprintf(sendBuf, "[KSH_LIN]EMERG%s\n", ledOn ? "ON" : "OFF");
        client.write(sendBuf, strlen(sendBuf));
        client.flush();
    #ifdef DEBUG
        Serial.print(sendBuf);
    #endif
      }
    }
  }
  lastButtonState = reading;
}
void wifi_Setup() {
  wifiSerial.begin(19200);
  wifi_Init();
  server_Connect();
}
void wifi_Init()
{
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI
      Serial.println("WiFi shield not present");
#endif
    }
    else
      break;
  } while (1);

#ifdef DEBUG_WIFI
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif
  while (WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {
#ifdef DEBUG_WIFI
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);
#endif
  }
  sprintf(lcdLine1, "ID:%s", LOGID);
  lcdDisplay(0, 0, lcdLine1);
  sprintf(lcdLine2, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  lcdDisplay(0, 1, lcdLine2);

#ifdef DEBUG_WIFI
  Serial.println("You're connected to the network");
  printWifiStatus();
#endif
}
int server_Connect()
{
#ifdef DEBUG_WIFI
  Serial.println("Starting connection to server...");
#endif

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI
    Serial.println("Connect to server");
#endif
    client.print("["LOGINID":"PASSWD"]");   
  }
  else
  {
#ifdef DEBUG_WIFI
    Serial.println("server connection failure");
#endif
  }
}
void printWifiStatus()
{
  // print the SSID of the network you're attached to

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
int ultrasonic(int trig, int echo)
{
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH);
  long dis_cm = duration / 29 / 2;

  return dis_cm;
}
