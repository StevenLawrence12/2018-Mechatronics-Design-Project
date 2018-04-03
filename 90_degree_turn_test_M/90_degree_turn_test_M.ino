#include<Servo.h>
#include<mcp_can.h>
#include<SPI.h>

const int leftServoPin=8;
const int rightServoPin=9;
const int SPI_CS_PIN=10;

MCP_CAN CAN(SPI_CS_PIN); 
Servo leftServo;
Servo rightServo;

byte len=0;
byte receiveBuf[8];

int leftMotorSpeed;
int rightMotorSpeed;

void setup() {

Serial.begin(9600);

Serial.println("Begin");
pinMode(leftServoPin,OUTPUT);
pinMode(rightServoPin,OUTPUT);

leftServo.attach(leftServoPin);
rightServo.attach(rightServoPin);
Serial.println("Begin");

while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");
}

void loop() {

if(CAN_MSGAVAIL==CAN.checkReceive()) //checks if there is anything to read in CAN buffers
  {
    Serial.println("receive");
    CAN.readMsgBuf(&len, receiveBuf); //reading CAN message
    
  for(int i=0; i<8;i++){
    Serial.print(receiveBuf[i]);
    Serial.print(", ");
  }Serial.println();
  }
if(receiveBuf[2]==1) leftMotorSpeed=1500-receiveBuf[0];
  else leftMotorSpeed=1500+receiveBuf[0];
  if(receiveBuf[3]==1) rightMotorSpeed=1500-receiveBuf[1];
  else rightMotorSpeed=1500+receiveBuf[1];

  leftServo.writeMicroseconds(leftMotorSpeed);
  rightServo.writeMicroseconds(rightMotorSpeed);
}
