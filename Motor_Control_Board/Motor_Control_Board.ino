/*
 *Motor Control V1.0
 *
 */
//include librarys
#include<Servo.h>
#include<mcp_can.h>
#include<SPI.h>

//setup CAN 
const int SPI_CS_PIN=10;
MCP_CAN CAN(SPI_CS_PIN);

//init servo objects
Servo servo_LeftMotor;
Servo servo_RightMotor;

//init pins
const int LeftMotorPin=8;
const int RightMotorPin=9;

//init variables
unsigned int leftMotorSpeed;
unsigned int rightMotorSpeed;

//drive mode; 0=stop, 1=straight, 2=left turn, 3=right turn, 4=reverse
unsigned int driveMode=0;

//CAN stuff
byte len=0;
byte receiveBuf[8];
int canId;

byte driveBuf[8];

//drive function
void drive(int leftSpeed,int rightSpeed)
{
  servo_LeftMotor.writeMicroseconds(leftSpeed);
  servo_RightMotor.writeMicroseconds(rightSpeed);
}

void setup() 
{
  Serial.begin(9600);
  pinMode(LeftMotorPin,OUTPUT);
  servo_LeftMotor.attach(LeftMotorPin);
  pinMode(RightMotorPin,OUTPUT);
  servo_RightMotor.attach(RightMotorPin);

  //init CAN
  while(CAN_OK!=CAN.begin(CAN_500KBPS))
  {
    Serial.println("CAN BUS init fail");
    Serial.println("Init CAN BUS fail again");
    delay(100);
  }
  Serial.println("CAN BUS init ok!");
  driveMode=1;
}

void loop() 
{

  //receiving CAN message
  if(CAN_MSGAVAIL==CAN.checkReceive()) //checks if there is anything to read in CAN buffers
  {
    CAN.readMsgBuf(&len, receiveBuf); //reading CAN message
    canId=CAN.getCanId();  //read the message id associated with this CAN message
    Serial.println("message recieved");
    Serial.print("CAN ID: ");
    Serial.println(canId);
    if(canId==0x01)
    {
    for(int i=0;i<8;i++)
    {
      driveBuf[i]=receiveBuf[i];
    }
    }
  }


  if(driveBuf[1]==0x00)
    rightMotorSpeed=1500-driveBuf[0];
  else if(driveBuf[1]==0x01)
    rightMotorSpeed=1500+driveBuf[0];

  if(driveBuf[3]==0x00)
  leftMotorSpeed=1500-driveBuf[2];
  else if(driveBuf[3]==0x01);
  leftMotorSpeed=1500+driveBuf[2];

  
  
  if(Serial.available()>0)
  driveMode=Serial.read()-48;
  
  /*switch(driveMode)
  {
    case 0:
    {
      leftMotorSpeed=1500;
      rightMotorSpeed=1500;
      Serial.println("Stopped");
      break;
    }
    case 1: 
    {
      leftMotorSpeed=1650;
      rightMotorSpeed=1640;
      Serial.println("Driving Straight");
      break;
    }
     case 2: 
    {
      leftMotorSpeed=1450;
      rightMotorSpeed=1650;
      Serial.println("Turning Left");
      break;
    }
     case 3: 
    {
      leftMotorSpeed=1650;
      rightMotorSpeed=1450;
      Serial.println("Turning Right");
      break;
    }
    case 4:
    {
      leftMotorSpeed=1350;
      rightMotorSpeed=1360;
      Serial.println("Reverse");
      break;
    }
  }*/
 drive(leftMotorSpeed, rightMotorSpeed);
}
