/*
 *Motor Control V2.0
 *
 */
//include librarys
#include<Servo.h>
#include<mcp_can.h>
#include<SPI.h>

//declare pins
const int LeftMotorPin=8;
const int RightMotorPin=9;
const int SPI_CS_PIN=10;

//create objects  
MCP_CAN CAN(SPI_CS_PIN);
Servo servo_LeftMotor;
Servo servo_RightMotor;

//init variables
unsigned int leftMotorSpeed;
unsigned int rightMotorSpeed;


//CAN variables
byte len=0;
byte receiveBuf[8];
unsigned long canId;
byte driveBuf[8];

//drive function
void drive(int leftSpeed,int rightSpeed){
  servo_LeftMotor.writeMicroseconds(leftSpeed);
  servo_RightMotor.writeMicroseconds(rightSpeed);
}

void setup() {
  Serial.begin(9600);

  //pinmodes 
  pinMode(LeftMotorPin,OUTPUT);
  pinMode(RightMotorPin,OUTPUT);

  //attach servos
  servo_LeftMotor.attach(LeftMotorPin);
  servo_RightMotor.attach(RightMotorPin);

  //init CAN
  while(CAN_OK!=CAN.begin(CAN_500KBPS)){
    Serial.println("CAN BUS init fail");
    Serial.println("Init CAN BUS fail again");
    delay(100);
  }Serial.println("CAN BUS init ok!");
  
}

void loop() {

  //receiving CAN message
  if(CAN_MSGAVAIL==CAN.checkReceive()) //checks if there is anything to read in CAN buffers
  {
    CAN.readMsgBuf(&len, receiveBuf); //reading CAN message
    canId=CAN.getCanId();  //read the message id associated with this CAN message
    
    if(canId==0x01){
    for(int i=0;i<8;i++){
      driveBuf[i]=receiveBuf[i];
    }
    }
  }

  //set drive motor speeds
  if(driveBuf[1]==0) leftMotorSpeed=1500-driveBuf[0];
  else leftMotorSpeed=1500+driveBuf[0];
  if(driveBuf[3]==0) rightMotorSpeed=1500-driveBuf[2];
  else rightMotorSpeed=1500+driveBuf[2];

  //drive motors
  drive(leftMotorSpeed, rightMotorSpeed);
}
