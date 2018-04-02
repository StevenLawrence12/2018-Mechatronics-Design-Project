#include <Wire.h>
#include <I2CEncoder.h>
#include<mcp_can.h>
#include<SPI.h>

const int SPI_CS_PIN=10;

MCP_CAN CAN(SPI_CS_PIN);

I2CEncoder encoder_LeftMotor;
I2CEncoder encoder_RightMotor;


int timeStart;
bool started = false;

long rightWheelPosition;
long leftWheelPosition;

unsigned int ui_Motors_Speed=100;

unsigned long ui_Left_Motor_Offset;
unsigned long ui_Right_Motor_Offset;

unsigned long CANId=0x01;
byte buf[8];

void send_CAN_Msg(unsigned long *msgId,byte *msgBuf);
void calibratioMagico();

void setup() {
  Wire.begin(); // Wire library required for I2CEncoder library
  Serial.begin(9600);

  while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

  
  Serial.println("Setup");
  encoder_LeftMotor.init(1.0/3.0*MOTOR_393_SPEED_ROTATIONS, MOTOR_393_TIME_DELTA);
  encoder_LeftMotor.setReversed(false); // adjust for positive count when moving forward
  encoder_RightMotor.init(1.0/3.0*MOTOR_393_SPEED_ROTATIONS, MOTOR_393_TIME_DELTA);
  encoder_RightMotor.setReversed(true); // adjust for positive count when moving forward
  //Serial.println("Setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  calibratioMagico();
  //servo_LeftMotor.writeMicroseconds(ui_Left_Motor_Speed);
  //servo_RightMotor.writeMicroseconds(ui_Right_Motor_Speed);
}
void calibratioMagico()
{ 
  if(!started){
    Serial.println("Calibrating");
   started = true;
   encoder_LeftMotor.zero();
   encoder_RightMotor.zero();
   timeStart = millis();

   buf[0]=ui_Motors_Speed;
   send_CAN_Msg(&CANId,buf);
   
  }
  else if((millis() - timeStart) > 5000)
  {
    buf[0]=0;
    send_CAN_Msg(&CANId,buf);
    
    leftWheelPosition = encoder_LeftMotor.getRawPosition();
    rightWheelPosition = encoder_RightMotor.getRawPosition();

    Serial.print("Left Wheel Position: ");
    Serial.println(leftWheelPosition);
    Serial.print("Right Wheel Position:");
    Serial.println(rightWheelPosition);
    
    if (leftWheelPosition > rightWheelPosition)
    {
      ui_Left_Motor_Offset = (leftWheelPosition - rightWheelPosition)/4;
      ui_Right_Motor_Offset = 0;
    }
    else
    {
      ui_Right_Motor_Offset = (rightWheelPosition - leftWheelPosition)/4;
      ui_Left_Motor_Offset = 0;
    }
    Serial.print("Motor Offsets: Left = ");
    Serial.print(ui_Left_Motor_Offset);
    Serial.print(", Right = ");
    Serial.println(ui_Right_Motor_Offset);

    buf[1]=ui_Left_Motor_Offset;
    buf[2]=ui_Right_Motor_Offset;
    send_CAN_Msg(&CANId,buf);
  }
}

 void send_CAN_Msg(unsigned long *msgId,byte *msgBuf){
 byte CANTx=CAN.sendMsgBuf(*msgId,0,8,msgBuf);
 if(CANTx==CAN_OK)
 Serial.println("Message sent successfully");
 else
 {
 Serial.print("Message not sent. Error code: ");
 Serial.println(CANTx); 
 }
}
 

