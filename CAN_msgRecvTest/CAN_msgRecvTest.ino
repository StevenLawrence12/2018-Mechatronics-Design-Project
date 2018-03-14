//include librarys
#include<mcp_can.h>
#include<SPI.h>
#include<Servo.h>

//declare pins
const int SPI_CS_PIN=10;
const int servo1Pin=8;
const int servo2Pin=9;

//init objects
Servo servo1;
Servo servo2;
MCP_CAN CAN(SPI_CS_PIN);

//init variables 
byte len;
byte receiveBuf[8];
unsigned long canId;
byte driveBuf[8];
int servo1Speed;
int servo2Speed;

void setup() { 
Serial.begin(9600);

while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

pinMode(servo1Pin,OUTPUT);
servo1.attach(servo1Pin);
pinMode(servo2Pin,OUTPUT);
servo2.attach(servo2Pin);
}

void loop() {

if(CAN_MSGAVAIL==CAN.checkReceive()){
  CAN.readMsgBuf(&len, receiveBuf);
  canId=CAN.getCanId();

  if(canId==0x01){
    for(int i=0;i<len;i++)
    driveBuf[i]=receiveBuf[i];
  }

if(driveBuf[1]==0) servo1Speed=1500-driveBuf[0];
else servo1Speed=1500+driveBuf[0];

if(driveBuf[3]==0) servo2Speed=1500-driveBuf[2];
else servo2Speed=1500+driveBuf[2];

Serial.print("Motor 1 speed: ");
Serial.println(servo1Speed);

Serial.print("Motor 2 speed: ");
Serial.println(servo2Speed);

servo1.writeMicroseconds(servo1Speed);
servo2.writeMicroseconds(servo2Speed);
}




}
