
//Include librarys
#include<mcp_can.h> //CAN communication lib
#include<SPI.h> //SPI chip select pin lib
#include<Wire.h>  //I2C comm lib
#include<I2CEncoder.h> //I2C encoder lib 

const int SPI_CS_PIN=10; //CAN chip select pin

//create objects
I2CEncoder leftEncoder;
I2CEncoder rightEncoder;
MCP_CAN CAN(SPI_CS_PIN);


double leftEncoderSpeed;
double rightEncoderSpeed;
int leftMotorDrive=100;
int rightMotorDrive=100;

int SDKp=0; //straight drive proportional constant

unsigned long motorDriveId=0x01;
byte motorDriveBuf[8];

void sendCANMsg(unsigned long *msgId,byte *msgBuf);

void setup() {
Serial.begin(9600);
Wire.begin();

leftEncoder.init((1.047/60)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
leftEncoder.setReversed(false);
rightEncoder.init((1.047/60)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
rightEncoder.setReversed(true);


//init CAN
while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

}

void loop() {
leftEncoderSpeed=leftEncoder.getSpeed();
rightEncoderSpeed=rightEncoder.getSpeed();

Serial.print("Left Encoder Speed: ");
Serial.println(leftEncoderSpeed);
Serial.print("Right Encoder Speed: ");
Serial.println(rightEncoderSpeed);
double error=leftEncoderSpeed-rightEncoderSpeed;

Serial.print("Error: ");
Serial.println(error);
rightMotorDrive+=error*SDKp;

motorDriveBuf[0]=leftMotorDrive;
motorDriveBuf[2]=rightMotorDrive;

Serial.print("Left Motor Speed: ");
Serial.println(leftMotorDrive);
Serial.print("Right Motor Speed: ");
Serial.println(rightMotorDrive);
Serial.println();
sendCANMsg(&motorDriveId,motorDriveBuf);
}

void sendCANMsg(unsigned long *msgId,byte *msgBuf){
 byte CANTx=CAN.sendMsgBuf(*msgId,0,8,msgBuf);
 if(CANTx==CAN_OK)
 Serial.println("Message sent successfully");
 else
 {
 Serial.print("Message not sent. Error code: ");
 Serial.println(CANTx); 
 }
}



