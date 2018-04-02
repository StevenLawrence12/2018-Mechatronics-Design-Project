#include <EEPROM.h>
#include<Servo.h>
#include<mcp_can.h>
#include<SPI.h>

const int SPI_CS_PIN=10;

MCP_CAN CAN(SPI_CS_PIN);

const int left_Motor_Pin = 8;
const int right_Motor_Pin = 9;

Servo servo_LeftMotor;
Servo servo_RightMotor;

byte b_LowByte;
byte b_HighByte;

unsigned int ui_Motors_Speed;
unsigned int ui_Left_Motor_Speed;
unsigned int ui_Right_Motor_Speed;
unsigned long ui_Left_Motor_Offset;
unsigned long ui_Right_Motor_Offset;

byte len;
byte receiveBuf[8];

void setup() {

  Serial.begin(9600);
      
  while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

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

  ui_Left_Motor_Speed = constrain(ui_Motors_Speed + ui_Left_Motor_Offset+1500, 1300, 2100);
  ui_Right_Motor_Speed = constrain(ui_Motors_Speed + ui_Right_Motor_Offset+1500, 1300, 2100);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(CAN_MSGAVAIL==CAN.checkReceive()) //checks if there is anything to read in CAN buffers
  {
    CAN.readMsgBuf(&len, receiveBuf); //reading CAN message
  }

  ui_Motors_Speed=receiveBuf[0]+1500;
  ui_Left_Motor_Offset=receiveBuf[1];
  ui_Right_Motor_Offset=receiveBuf[2];

  servo_RightMotor.writeMicroseconds(ui_Motors_Speed);
  servo_LeftMotor.writeMicroseconds(ui_Motors_Speed);

  Serial.print("Speed: ");
  Serial.println(ui_Motors_Speed);

  Serial.print("Motor Offsets: Left = ");
  Serial.print(ui_Left_Motor_Offset);
  Serial.print(", Right = ");
  Serial.println(ui_Right_Motor_Offset);
  
  EEPROM.write(14, lowByte(ui_Right_Motor_Offset));
  EEPROM.write(15, highByte(ui_Right_Motor_Offset));
  EEPROM.write(12, lowByte(ui_Left_Motor_Offset));
  EEPROM.write(13, highByte(ui_Left_Motor_Offset));

}
