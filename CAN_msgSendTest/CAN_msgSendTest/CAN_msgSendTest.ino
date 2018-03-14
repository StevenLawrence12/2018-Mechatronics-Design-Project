#include<mcp_can.h>
#include<SPI.h>
#include<Wire.h>
#include<I2CEncoder.h>

const int SPI_CS_PIN=10;

I2CEncoder leftEncoder;
I2CEncoder rightEncoder;

double leftEncoderSpeed; 
double rightEncoderSpeed;

int leftDriveMotorSpeed=150;
int rightDriveMotorSpeed=150;

//create MCP_CAN object
MCP_CAN CAN(SPI_CS_PIN);

//buffer to store drive motor commands
byte motorDriveBuf[8]; //[0]=left motor speed(0-255); [1]=left motor forward or reverse (1 or 0); [2] right motor speed (0-255); [3] right motor forward or reverse (1 or 0) 
unsigned long motorDriveId=0x01;


void setup() {
Serial.begin(9600);
Wire.begin();

//initialize can bus
while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

leftEncoder.init((10.3)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
leftEncoder.setReversed(false);
rightEncoder.init((10.3)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
rightEncoder.setReversed(true);

}

void loop() {
leftEncoderSpeed=leftEncoder.getSpeed();
rightEncoderSpeed=rightEncoder.getSpeed();

Serial.print("Left encoder reading: ");
Serial.print((leftEncoderSpeed/60));
Serial.println("cm/s");

Serial.print("Right encoder reading: ");
Serial.print((rightEncoderSpeed/60));
Serial.println("cm/s");
 if((leftEncoderSpeed-rightEncoderSpeed)>.5) 
  {
    leftDriveMotorSpeed--;
  }
  if((rightEncoderSpeed-leftEncoderSpeed)>.5)  
  {
    leftDriveMotorSpeed++;
  }

motorDriveBuf[0]=leftDriveMotorSpeed;
motorDriveBuf[1]=1;
motorDriveBuf[2]=rightDriveMotorSpeed;  
motorDriveBuf[3]=1;

CAN.sendMsgBuf(motorDriveId,0,8,motorDriveBuf);

}
