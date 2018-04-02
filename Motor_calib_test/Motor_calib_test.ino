#include <EEPROM.h>
#include<Servo.h>

const int left_Motor_Pin = 8;
const int right_Motor_Pin = 9;


Servo servo_RightMotor;
Servo servo_LeftMotor;

byte b_LowByte;
byte b_HighByte;

unsigned int ui_Motors_Speed=150;
unsigned int ui_Left_Motor_Speed;
unsigned int ui_Right_Motor_Speed;
unsigned long ui_Left_Motor_Offset;
unsigned long ui_Right_Motor_Offset;

void setup() {
  
Serial.begin(9600);

pinMode(right_Motor_Pin, OUTPUT);
  servo_RightMotor.attach(right_Motor_Pin);
  pinMode(left_Motor_Pin, OUTPUT);
  servo_LeftMotor.attach(left_Motor_Pin);

  b_LowByte = EEPROM.read(12);
  b_HighByte = EEPROM.read(13);
  ui_Left_Motor_Offset = word(b_HighByte, b_LowByte);
  b_LowByte = EEPROM.read(14);
  b_HighByte = EEPROM.read(15);
  ui_Right_Motor_Offset = word(b_HighByte, b_LowByte);

  Serial.print("left offset: ");
  Serial.println(ui_Left_Motor_Offset);
  Serial.print("right offset: ");
  Serial.println(ui_Right_Motor_Offset);
  

  ui_Left_Motor_Speed = constrain(ui_Motors_Speed + ui_Left_Motor_Offset+1500, 1300, 2100);
  ui_Right_Motor_Speed = constrain(ui_Motors_Speed + ui_Right_Motor_Offset+1500, 1300, 2100);

  Serial.print("left speed: ");
  Serial.println(ui_Left_Motor_Speed);
  Serial.print("right speed: ");
  Serial.println(ui_Right_Motor_Speed);
}

void loop() {
  servo_RightMotor.writeMicroseconds(ui_Right_Motor_Speed);
  servo_LeftMotor.writeMicroseconds(ui_Left_Motor_Speed);

}
