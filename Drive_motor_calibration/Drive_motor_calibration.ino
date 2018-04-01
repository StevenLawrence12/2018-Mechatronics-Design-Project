#include<I2CEncoder.h>
#include<Wire.h>
#include<mcp_can.h>
#include<SPI.h>


const int SPI_CS_PIN=10; 

I2CEncoder leftEncoder;                       
I2CEncoder rightEncoder;
MCP_CAN CAN(SPI_CS_PIN);


int leftMotorSpeed=150;
int rightMotorSpeed=150;
int speedChangeCount=0;
int averageSpeed;
int accLeftMotorSpeed=0;

double leftEncoderSpeed; //left motor encoder
double rightEncoderSpeed; //right motor encoder


unsigned long motorDriveId=0x01;
byte motorDriveBuf[8];


void sendCANMsg(unsigned long *msgId,byte *msgBuf);

void setup() {
 Serial.begin(9600);
Wire.begin();



leftEncoder.init((32.4)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
leftEncoder.setReversed(false);
rightEncoder.init((32.4)*MOTOR_393_SPEED_ROTATIONS,MOTOR_393_TIME_DELTA);
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

      /*Serial.print("Left Encoder Speed: ");
      Serial.println(leftEncoderSpeed);
      Serial.print("Right Encoder Speed: ");
      Serial.println(rightEncoderSpeed);*/

      if((leftEncoderSpeed-rightEncoderSpeed)>1) {
        leftMotorSpeed--;
      }
      
      if((rightEncoderSpeed-leftEncoderSpeed)>1)  {
          leftMotorSpeed++;
      }
    accLeftMotorSpeed+=leftMotorSpeed;
    speedChangeCount++;
    averageSpeed=accLeftMotorSpeed/speedChangeCount;
    
    Serial.print("Left Motor Speed: ");
    Serial.println(leftMotorSpeed);
    Serial.print("accumulated speed: ");
    Serial.println(accLeftMotorSpeed);
    
    Serial.print("speed counter: ");
    Serial.println(speedChangeCount);
    Serial.print("Average Left Motor Speed: ");
    Serial.println(averageSpeed);
  motorDriveBuf[0]=leftMotorSpeed;
  motorDriveBuf[2]=rightMotorSpeed;
sendCANMsg(&motorDriveId,motorDriveBuf);
Serial.println();
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
