#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

#define beepPin 7
#define RST_PIN 9
#define SS_PIN 10
#define TRIG 4
#define ECHO 5
#define SERVO_PIN 6
#define LED_BUILTIN_PIN 13


bool timerIsrFlag = false;
boolean sonic_Flag = false;
boolean sonic_S_Flag = false;

char lcdLine1[17] = "Smart Parking";
char lcdLine2[17] = "Service";

unsigned int secCount;
unsigned int myservoTime = 0;
unsigned int mysonicTime = 0;
int sonic;
int Tx = 2;
int Rx = 3;
int beepFlag = 0;

SoftwareSerial my_blue(Tx, Rx);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;
MFRC522 mfrc(SS_PIN, RST_PIN);

void timerIsr();
void lcdDisplay(int x, int y, char * str);
int ultrasonic(int trig, int echo);


void setup() {
  lcd.init();
  lcd.backlight();
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);

  pinMode(TRIG, OUTPUT);    // 초음파 센서 trig을 출력으로 설정 
  pinMode(ECHO, INPUT);    // 초음파 센서 echo을 입력으로 설정 

  MsTimer2::set(1000, timerIsr); // 1000ms period
  MsTimer2::start();

  myservo.attach(SERVO_PIN);
  myservoTime = secCount;
  Serial.begin(9600);
  my_blue.begin(9600);
  SPI.begin(); 
  mfrc.PCD_Init();
  pinMode(7, OUTPUT);
}

void loop() {
  if(my_blue.available())
  {
    Serial.write(my_blue.read());
  }
  if(Serial.available())
  {
    my_blue.write(Serial.read());
  }

  if (timerIsrFlag) //1초에 한번씩 실행
  {
    
    sonic = ultrasonic(TRIG,ECHO);
    
    Serial.print("sensor1 :");
    Serial.print(sonic);
    Serial.println("cm");

    if(sonic<7)  // 주차공간 위의 초음파센서
    {
        mysonicTime = millis();
        digitalWrite(LED_BUILTIN_PIN,HIGH);
        myservo.write(100);
        sonic_Flag = true;
        beepFlag = 1;
        //tone(7, 261);
        //digitalWrite(7, HIGH);
    }
    else
    {
        myservo.write(5);
        sonic_Flag = false;
        beepFlag = 0;
        //tone(7, 294);
        //digitalWrite(7, LOW);
    }
    timerIsrFlag = false;

    //RFID
    if(!mfrc.PICC_IsNewCardPresent())
      return;
    if(!mfrc.PICC_ReadCardSerial())
      return; 

    Serial.print("Card UID:"); 
    char lcdLine3[17] = "";
    char str[4][4] = {"",};

    for (byte i = 0; i < 4; i++) {
      Serial.print(mfrc.uid.uidByte[i]); 
      
      sprintf(str[i], "%d", mfrc.uid.uidByte[i]);
      Serial.print(" "); 
    }
    for (int i = 0; i < 4; i++) {
      Serial.print(str[i]); 
      Serial.print(" "); 
    }

    Serial.print(lcdLine3); 
    lcdDisplay(0, 1, lcdLine3);

    
    Serial.println(); 

    if (!(secCount % 10)) //10초에 한번씩 실행
    {
    }
    
  }
}

void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
}
void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x, y);
  lcd.print(str);
  for (int i = len; i > 0; i--)
    lcd.write(' ');
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
