#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <IRremote.hpp>
#include <MFRC522.h>
#include <SoftwareSerial.h>


#define TX 3
#define RX 4

#define RST_PIN 9
#define SS_PIN 10

SoftwareSerial soft_Serial(3,4);
MFRC522 mfrc(SS_PIN, RST_PIN);

LiquidCrystal_I2C lcd(0x27,16,2);

int RECV_PIN=2; //IR 수신
IRrecv ir(RECV_PIN);
decode_results results;

//전역변수

int flag=0;

// card id
int str1[5];
int str2[5];
int str3[5];
int str4[5];

int str1_List;
int str2_List;
int str3_List;
int str4_List;

int trig=8;
int echo=5;

// 메인 홈
void H_main(){
    lcd.clear();
    lcd.print("Wel Come!");
    lcd.setCursor(0,1);
    lcd.print("-.Jion");
    lcd.setCursor(8,1);
    lcd.print("+.Login");
}

void setup() {
  lcd.init();

  ir.enableIRIn(); //리모컨 수신
  SPI.begin();
  mfrc.PCD_Init();
  soft_Serial.begin(9600);
  pinMode(6,OUTPUT); //LED Blue
  pinMode(7,OUTPUT); //LED Red
    
  pinMode(trig,OUTPUT); //초음파 거리
  pinMode(echo,INPUT);
}

void loop() {    
  if(flag==0){
    Light();
    Exc_masg1();
  }
  else if(flag==1){
    card_ch2();
  }
  else if(flag==2){
    join();
  }
  else if(flag==3){
    Suc_masg();
    Suc_masg2();
  }
  else if(flag==4){          
    Exc_masg2();
  }
}
//손님 확인
void Light() {
  digitalWrite(trig,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig,LOW);
  
  int dis=pulseIn(echo,HIGH)*340/2/10000;//cm단위
  if(dis<=10){
    lcd.backlight(); 
    H_main();
    contect();
    pulseIn(echo,LOW);
  }
  else{
    lcd.noBacklight();
    lcd.clear();
  }
}

//가입 및 로그인 선택 창
void contect() {
  if(ir.decode(&results)){     
    int results_N= (int)results.value;
    if(results_N==-22441){
      lcd.clear();
      lcd.print("LOGIN");
      lcd.setCursor(0,1); 
      lcd.print("Card Please!");
      flag=1;       
    }
    else if(results_N==-8161){
      lcd.clear();
      lcd.print("JOIN");
      lcd.setCursor(0,1);
      lcd.print("Card Please!");
      flag=2;           
    }
    else if(results_N==-28561){
      flag=4;           
    }
    ir.resume();
  }
}
//카드 등록
void join(){
  if(!mfrc.PICC_IsNewCardPresent())
  return;
  
  if(!mfrc.PICC_ReadCardSerial())
  return;
  
  for(byte i=0;i<4;i++){ //카드 번호 저장
    str1_List=mfrc.uid.uidByte[0];       
    str2_List=mfrc.uid.uidByte[1];
    str3_List=mfrc.uid.uidByte[2];
    str4_List=mfrc.uid.uidByte[3];          
    Serial.print(" ");
  } 
  join2();
  lcd.clear();
  lcd.print("Sucess");
  lcd.setCursor(0,1);
  lcd.print("Login please!");
  delay(1000);
  flag=0;
}
// 카드 아이디 등록
void join2(){  
  int size= sizeof(str1) / sizeof(str1[0]);

  for(int i=0; i<size;i++){ 
    if(str1[i]==0){
      str1[i]=str1_List;
      str2[i]=str2_List;
      str3[i]=str3_List;
      str4[i]=str4_List;            
      break;                
    }    
  }
  digitalWrite(6,HIGH);
  delay(1000);
  digitalWrite(6,LOW);
}

//카드 확인
void card_ch2(){
  int size=sizeof(str1)/sizeof(str1[0]);
  if(!mfrc.PICC_IsNewCardPresent())
  return;
    
  if(!mfrc.PICC_ReadCardSerial())
  return;
    
  for(int i=0;i<size;i++){
    if(mfrc.uid.uidByte[0]== str1[i] && //카드 조회
       mfrc.uid.uidByte[1]== str2[i] &&
       mfrc.uid.uidByte[2]== str3[i] &&
       mfrc.uid.uidByte[3]== str4[i]){
      soft_Serial.print(str1[i]); //arduion 카드 번호 전송
      soft_Serial.print(" ");  
      soft_Serial.print(str2[i]);
      soft_Serial.print(" "); 
      soft_Serial.print(str3[i]);
      soft_Serial.print(" "); 
      soft_Serial.print(str4[i]);
      soft_Serial.print(" "); 
      card_ch3();           
      break;
    }
    else if(str1[i]==0){              
      card_ch4();
      break;
    }
  }
}
    
//카드 확인 성공시
void card_ch3(){
  lcd.clear();
  lcd.print("Sucess");
  digitalWrite(6,HIGH);
  delay(1000);
  digitalWrite(6,LOW);
  flag=3;                                                  
}
      
//카드 확인 실패시
void card_ch4(){
  lcd.clear();
  lcd.print("Failed");  
  digitalWrite(7,HIGH);
  delay(1000);
  digitalWrite(7,LOW);   
  flag=0;                                   
}

//사용 일자 작성 요청 창
void Suc_masg(){
  lcd.clear();
  lcd.print("Write the date");
  lcd.setCursor(0,1);
  lcd.print("1~9DAY Connect");
  delay(5000);
}   
//사용 일 선정
void Suc_masg2(){
  int nu=0;
  lcd.setCursor(0,1);
  if(ir.decode(&results)){     
    int results_N= (int)results.value;
    if(results_N==12495) nu=1;
    else if(results_N==6375) nu=2;           
    else if(results_N==31365) nu=3;  
    else if(results_N==4335) nu=4;           
    else if(results_N==14535) nu=5;        
    else if(results_N==23205) nu=6;          
    else if(results_N==17085) nu=7;        
    else if(results_N==19125) nu=8;        
    else if(results_N==21165) nu=9;              
    ir.resume();
    lcd.clear();
    lcd.print("storage days :");
    lcd.setCursor(0,1);
    lcd.print(nu);
    lcd.print("DAY"); 
    delay(5000);
    Suc_masg3();
  }    
}
//이용가능한 방 안내  
void Suc_masg3(){
  char room;
  while(soft_Serial.available()){
    room= soft_Serial.read();
    delay(1);
  }
  lcd.clear();
  lcd.print("Cage Number :");
  lcd.setCursor(8,1);
  lcd.print("N0.");
  lcd.print(room);
  lcd.print(" Go");
  delay(5000);
  flag=0;
}
//퇴실 확인
void Exc_masg1(){
  if(soft_Serial.available()>0){
    int EX_nu= soft_Serial.read();
    if(EX_nu ==49){
      flag=4;
    }
  }
}
// 퇴실 안내  창
void Exc_masg2(){
  lcd.clear(); 
  lcd.print("GOOD BYE~~");   
  lcd.setCursor(0,1);
  lcd.print("Have a Nice Day!");   
  delay(5000);
  flag=0;
}
  
