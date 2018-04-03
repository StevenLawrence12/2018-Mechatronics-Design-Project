#include<mcp_can.h>
#include<SPI.h>
#include<I2CEncoder.h>
#include<Wire.h>

const int SPI_CS_PIN=10;

MCP_CAN CAN(SPI_CS_PIN);
I2CEncoder leftEncoder;
I2CEncoder rightEncoder;

unsigned long driveMotorsId=0x01;
byte driveMotorsBuf[8]={0,0,0,0,0,0,0,0};

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

void setup() {
Serial.begin(9600);
Wire.begin();

leftEncoder.init((31.9186)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
leftEncoder.setReversed(false);
rightEncoder.init((31.9186)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
rightEncoder.setReversed(true);

while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

leftEncoder.zero();
rightEncoder.zero();
delay(5000);
driveMotorsBuf[0]=100;
driveMotorsBuf[1]=100;
driveMotorsBuf[2]=1;
}

void loop() {

  if(rightEncoder.getRawPosition()>305)
  driveMotorsBuf[1]=0;
  if(leftEncoder.getRawPosition()<-305)
  driveMotorsBuf[0]=0;

  send_CAN_Msg(&driveMotorsId,driveMotorsBuf);
}
