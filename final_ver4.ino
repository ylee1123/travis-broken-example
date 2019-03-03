//#include <Servo.h>
#include <IRremote.h>
#include <SoftwareSerial.h> // 시리얼 통신을 위한 라이브러리 선언
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define rx 2
#define tx 3                          // TX, RX Pin for GPS Module

int RECV_PIN = A0;                    // 적외선 수신센서 핀(아날로그 입력 A0)
IRrecv irrecv(RECV_PIN);             // 적외선 송수신 통신을 위한 객체
decode_results decodedSignal;       // 적외선 수신값 해석을 위한 객체
TinyGPS gps;
SoftwareSerial gpsSerial(tx, rx);

int trigPin = 4;
int echoPin = 7;
long duration, distance, vib_measurement;

int RightMotor_E_pin = 5;
int RightMotor_1_pin = 8;
int RightMotor_2_pin = 9;
int LeftMotor_E_pin = 6;
int LeftMotor_3_pin = 10;
int LeftMotor_4_pin = 11;
int shock = 13;
int button = 12;
int fw = 0;

int state;
int isClicked = 0;
unsigned long prev_time = 0;
int buttonState;
int mode = 0;
int c;
float lat;
float lon;

int CarSpeed = 153;
int prev_speed = 0;

# define DIR_FW 1 // Forward
# define DIR_BW 2 // Backward
# define DIR_LF 3 // Left
# define DIR_RF 4 // Right
# define DIR_ST 5 // Stop
char CarDirection = 0;

void CarGo();
void CarBack();
void CarStop();
void CarLeft();
void CarRight();
void CarUpdate();
void Obstacle_Check();
void Distance_Measurement();
void getgps(TinyGPS &gps);
String decode_IRvalue(unsigned long irValue);
void controllerByIRCommand(String& szIRCmd);

struct IRvalueData
{
  String name;              // 적외선 리모컨 버튼 이름
  unsigned long value;      // 버튼 이름의 고유값
};

// 적외선 리모컨의 명령코드
IRvalueData irData[21] =
{
  { "0", 0xC101E57B },
  { "1", 0x9716BE3F },
  { "2", 0x3D9AE3F7 },
  { "3", 0x6182021B },
  { "4", 0x8C22657B },
  { "5", 0x488F3CBB },
  { "6", 0x449E79F },
  { "7", 0x32C6FDF7 },
  { "8", 0x1BC0157B },
  { "9", 0x3EC3FC1B },
  { "100+", 0x97483BFB },
  { "200+", 0xF0C41643 },
  { "-", 0xF076C13B },
  { "+", 0xA3C8EDDB },
  { "EQ", 0xE5CFBD7F },
  { "<<", 0x52A3D41F },
  { ">>", 0xD7E84B1B },
  { ">|", 0x20FE4DBB },
  { "CH-", 0xE318261B },
  { "CH", 0x511DBB },
  { "CH+", 0xEE886D7F }
};

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  irrecv.enableIRIn(); // 적외선 통신 수신 시작

  pinMode(echoPin, INPUT);
  pinMode(shock, INPUT);
  pinMode(button, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(RightMotor_E_pin, OUTPUT);
  pinMode(RightMotor_1_pin, OUTPUT);
  pinMode(RightMotor_2_pin, OUTPUT);
  pinMode(LeftMotor_E_pin, OUTPUT);
  pinMode(LeftMotor_3_pin, OUTPUT);
  pinMode(LeftMotor_4_pin, OUTPUT);

  digitalWrite(RightMotor_E_pin, HIGH); // enabling Right Motor
  digitalWrite(LeftMotor_E_pin, HIGH); // enabling Left Motor
}

void loop() {

  buttonState = digitalRead(button);
  vib_measurement = pulseIn(shock, HIGH);

  if (fw == 0)
  {
    getgps(gps);
    delay(30000);
    fw = 1;
    Serial.print(" init done ");
  }

  while (gpsSerial.available())    // While there is data on the RX pin...//출처: https://deneb21.tistory.com/331 [Do It Yourself!]
  {
    int c = gpsSerial.read();    // load the data into a variable...
    if (gps.encode(c))     // if there is a new valid sentence...
    {
      Serial.print("outside ");
      getgps(gps);
      Serial.print(" output= ");
      Serial.println(vib_measurement);
    }
  }

  Serial.print("inside ");

  if (buttonState == HIGH)
  {
    if (isClicked == 0)
    {
      delay(10);
      isClicked = 1;
    }
  }

  if (buttonState == LOW) {
    if (isClicked == 1) {
      mode += 1;
      delay(10);
      isClicked = 0;
    }
  }

  state = mode % 2;

  if (state == 0) {
    //Serial.println("Turning into Manual Mode!");
    if (irrecv.decode(&decodedSignal) == true) {
      //Serial.println(decodedSignal.value, HEX);
      String szRecvCmd = decode_IRvalue(decodedSignal.value); // IR 수신값 해석
      controllerByIRCommand(szRecvCmd);
      irrecv.resume(); // Receive the next valuㄷ
      CarUpdate(); // 스마트카 상태 업데이트
    }
  }

  if (state == 1) {
    //Serial.println("Turning into Auto Mode!");
    CarGo();
    delay(100);
    Obstacle_Check();
    delay(100);
  }

  Serial.print("output= ");
  Serial.println(vib_measurement);

}

String decode_IRvalue(unsigned long irValue)  // IR 수신값을 해석하는 함수
{
  for (int i = 0; i < 21; i++)
  {
    if (irData[i].value == irValue)
    {
      return irData[i].name;            // 해당 수신값의 이름을 반환
    }
  }
}

void controllerByIRCommand(String& szIRCmd)
{
  if (szIRCmd == "+") // 모터의 speed up
  {
    prev_speed = CarSpeed;
    CarSpeed += 5;
    CarSpeed = min(CarSpeed, 255);
  }
  else if (szIRCmd == "-") // 모터의 speed down
  {
    prev_speed = CarSpeed;
    CarSpeed -= 5;
    CarSpeed = max(CarSpeed, 50);
  }
  else if (szIRCmd == "2") {// 전진
    CarDirection = DIR_FW;
  }
  else if (szIRCmd == "5") { // 정지
    CarDirection = DIR_ST;
  }
  else if (szIRCmd == "8") { // 후진
    CarDirection = DIR_BW;
  }
  else if (szIRCmd == "4") {  // 좌회전
    CarDirection = DIR_LF;
  }
  else if (szIRCmd == "6") { // 우회전
    CarDirection = DIR_RF;
  }
}

void Obstacle_Check() {
  int val = random(2);
  Distance_Measurement();

  while (distance < 200) {
    if (distance < 100) {
      CarBack();
      delay(1000);
      CarStop();
      delay(200);
      Distance_Measurement();
    }
    else {
      if (val == 0) {
        CarStop();
        delay(700);
        CarBack();
        delay(1000);
        CarRight();
        delay(800);
      }
      else if (val == 1) {
        CarStop();
        delay(700);
        CarBack();
        delay(1000);
        CarLeft();
        delay(800);
      }
      Distance_Measurement();
    }
  }
}

void Distance_Measurement() {
  digitalWrite(trigPin, LOW);
  delay(2);
  digitalWrite(trigPin, HIGH); // trigPin에서 초음파 발생
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = ((float)(340 * duration) / 1000) / 2;
  delay(10);
}

void CarGo() {
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);

  for (int i = prev_speed; i <= CarSpeed; i = i + 10) {
    analogWrite(RightMotor_E_pin, i);
    analogWrite(LeftMotor_E_pin, i);
    delay(20);
  }
  CarDirection == DIR_FW;
  prev_speed = CarSpeed;
}

void CarBack() {
  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, HIGH);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH);

  for (int i = prev_speed; i <= CarSpeed; i = i + 10) {
    analogWrite(RightMotor_E_pin, i);
    analogWrite(LeftMotor_E_pin, i);
    delay(20);
  }
  CarDirection == DIR_BW;
  prev_speed = CarSpeed;
}

void CarStop() {
  if (CarDirection == DIR_FW || CarDirection == DIR_LF || CarDirection == DIR_RF) {
    for (int i = CarSpeed; i >= 0; i = i - 5) {
      analogWrite(RightMotor_E_pin, i);
      analogWrite(LeftMotor_E_pin, i);
      delay(20);
    }
  } else if (CarDirection == DIR_BW) {
    for (int i = CarSpeed; i >= 0; i = i - 5) {
      analogWrite(RightMotor_E_pin, i);
      analogWrite(LeftMotor_E_pin, i);
      delay(20);
    }
  }
  digitalWrite(RightMotor_E_pin, LOW); // 정지
  digitalWrite(LeftMotor_E_pin, LOW);
  CarDirection == DIR_ST;
}

void CarLeft() {
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);

  for (int i = prev_speed; i <= CarSpeed; i = i + 5) {
    analogWrite(RightMotor_E_pin, i * 1.4);           // 140%
    analogWrite(LeftMotor_E_pin, i * 0.2);            // 20%
    delay(50);
  }
  CarDirection == DIR_LF;
  prev_speed = CarSpeed;
}

void CarRight() {
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);

  for (int i = prev_speed; i <= CarSpeed; i = i + 5) {
    analogWrite(RightMotor_E_pin, i * 0.2);           // 20%
    analogWrite(LeftMotor_E_pin, i * 1.4);            // 140%
    delay(50);
  }
  CarDirection == DIR_RF;
  prev_speed = CarSpeed;
}

void CarUpdate()
{
  if (CarDirection == DIR_FW) // 전진
    CarGo();
  else if (CarDirection == DIR_BW) // 후진.
    CarBack();
  else if (CarDirection == DIR_LF) // 좌회전
    CarLeft();
  else if (CarDirection == DIR_RF) // 우회전
    CarRight();
  else if (CarDirection == DIR_ST) // 정지.
    CarStop();
}

void getgps(TinyGPS &gps)
{
  gps.f_get_position(&lat, &lon);
  Serial.print("Lat/Lon: ");
  Serial.print(lat, 5);
  Serial.print(" ");
  Serial.print(lon, 5);

}

