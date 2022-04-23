#include <DHT.h> //온습도
#include <LiquidCrystal_I2C.h> //LCD
#include <SPI.h>
#include <deprecated.h> //RFID
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
#include <Servo.h> //서보모터
#include <SoftwareSerial.h> //시리얼 통신

//LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//RFID
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc(SS_PIN, RST_PIN);

//습도
DHT dht(A1, DHT11);

//서브모터
Servo my_Servo;

//시리얼 통신
#define TX 3
#define RX 4
SoftwareSerial soft_Serial(TX, RX);
char* ary[4];
String ary1[4];

//초음파
int trig = 6;
int echo = 7;

//랜덤값
long randomNum;
unsigned long pre_time = 0;
unsigned long cur_time = 0;

//전역변수
int cds = 0; //조도
float humi = 0; //습도
float temp = 0; //온도
int pos = 0; //서보모터
int flag = 0; //플래그

void isr() { //인터럽트
  cur_time = millis(); //눌렸을 때 현재 시간 저장
  if(cur_time - pre_time >= 1000) {
    randomNum = random(1, 10);
    soft_Serial.print(randomNum);
    pre_time = cur_time;
  }
}

void setup() {
  Serial.begin(9600);
  soft_Serial.begin(9600);
  SPI.begin();
  mfrc.PCD_Init();
  pinMode(A0, INPUT); //조도
  dht.begin(); //습도
  pinMode(trig, OUTPUT); //초음파
  pinMode(echo, INPUT);
  my_Servo.attach(5); //서보모터
  
  randomSeed(analogRead(0)); //인터럽트 - 랜덤값
  attachInterrupt(0, isr, RISING);
}

void loop() {
  if(flag == 0) { //초음파 -> 카드입력 -> 문열림 //랜덤값 전송
    getKey();
    ultra_wave1();
  }
  else if(flag == 1) { //문열림 동작
    Motor();
    flag = 2;
  }
  else if(flag == 2) { //온도 습도 조도 출력
    LCD1();
  }
  else if(flag == 3) { //문열림 동작
    Motor();
    soft_Serial.print(0);
    flag = 0;
  }
}

void CDS() { //조도
  cds = analogRead(A0);
}

void temp_humi() { //온도 습도
  humi = dht.readHumidity();
  temp = dht.readTemperature();
}

void Random() {
  lcd.setCursor(1, 0);
  lcd.print(randomNum);
}

void ultra_wave1() {
  lcd.init();
  lcd.backlight();
  
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  int dis = pulseIn(echo, HIGH) * 340 / 2 / 10000; //cm 단위
  delay(100);

  if(dis > 10) {
    lcd.clear();
    Random();
    lcd.setCursor(3, 0);
    lcd.print("cage");
  }
  else {
    lcd.clear();
    lcd.print("Enter the Key");
    RFID1();
  }
}

void ultra_wave2() {
  lcd.init();
  lcd.backlight();
  
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  int dis = pulseIn(echo, HIGH) * 340 / 2 / 10000; //cm 단위

  if(dis <= 10) {
    lcd.clear();
    lcd.print("Enter the Key");
    RFID2();
  }
  else
    flag = 2;
}

void LCD1() {
  temp_humi();
  CDS();
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("TEM:");
  lcd.setCursor(4, 0);
  lcd.print("    ");
  lcd.setCursor(4, 0);
  lcd.print((int)temp);
  
  lcd.setCursor(8, 0);
  lcd.print("HUM:");
  lcd.setCursor(12, 0);
  lcd.print("    ");
  lcd.setCursor(12, 0);
  lcd.print((int)humi);
  
  lcd.setCursor(0, 1);
  lcd.print("CDS:");
  lcd.setCursor(4, 1);
  lcd.print("    ");
  lcd.setCursor(4, 1);
  lcd.print(cds);

  ultra_wave2();
}

void Motor() {
  for (pos = 0; pos <= 180; pos += 1) {
    my_Servo.write(pos);
    delay(50);
  }
}

void getKey() {
  char card_num[30] = {'\0'};
  int i = 0;

  while(soft_Serial.available()){ //카드 번호 받아오기
    card_num[i] = soft_Serial.read();
    //Serial.print(card_num[i]);
    i++;
    delay(1);
  }
  
  char* ptr = strtok(card_num, " ");
  int j = 0;
  
  while (ptr != '\0') { //parsing
    ary[j] = ptr;
    Serial.print(ptr);
    ptr = strtok(NULL, " ");
    j++;
  }

  if(card_num[0] != NULL){
    for(int i = 0; i < 4; i++) {
      ary1[i] = ary[i];
    }
  }
}

void RFID1() {
  if(!mfrc.PICC_IsNewCardPresent()) //카드 인식 안될 때
    return;
  if(!mfrc.PICC_ReadCardSerial()) //예외
    return;

//  String a0 = (String)mfrc.uid.uidByte[0];
//  String a1 = (String)mfrc.uid.uidByte[1];
//  String a2 = (String)mfrc.uid.uidByte[2];
//  String a3 = (String)mfrc.uid.uidByte[3];

  String a0 = String(mfrc.uid.uidByte[0]);
  String a1 = String(mfrc.uid.uidByte[1]);
  String a2 = String(mfrc.uid.uidByte[2]);
  String a3 = String(mfrc.uid.uidByte[3]);
  
  if(a0.equals(ary1[0]) && a1.equals(ary1[1]) && a2.equals(ary1[2]) && a3.equals(ary1[3])) {
    lcd.clear();
    lcd.print("OPEN");
    delay(500);
    flag = 1;
  }
  else {
    lcd.clear();
    lcd.print("RE-Enter the Key");
    delay(500);
  }
}

void RFID2() {
  if(!mfrc.PICC_IsNewCardPresent()) //카드 인식 안될 때
    return;
  if(!mfrc.PICC_ReadCardSerial()) //예외
    return;

//  String a0 = (String)mfrc.uid.uidByte[0];
//  String a1 = (String)mfrc.uid.uidByte[1];
//  String a2 = (String)mfrc.uid.uidByte[2];
//  String a3 = (String)mfrc.uid.uidByte[3];

  String a0 = String(mfrc.uid.uidByte[0]);
  String a1 = String(mfrc.uid.uidByte[1]);
  String a2 = String(mfrc.uid.uidByte[2]);
  String a3 = String(mfrc.uid.uidByte[3]);

  if(a0.equals(ary1[0]) && a1.equals(ary1[1]) && a2.equals(ary1[2]) && a3.equals(ary1[3])) {
    lcd.clear();
    lcd.print("OPEN");
    delay(500);
    flag = 3;
  }
  else {
    lcd.clear();
    lcd.print("RE-Enter the Key");
    delay(500);
    flag = 2;
  }
}
