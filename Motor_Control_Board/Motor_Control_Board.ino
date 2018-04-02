/*
 *Motor Control V2.0
 *
 */
//include librarys
#include<Servo.h>
#include<mcp_can.h>
#include<SPI.h>

//declare pins
const int tipMotorPin=5;
const int swingMotorPin=6;
const int hugMotorPin=7;
const int LeftMotorPin=8;
const int RightMotorPin=9;
const int SPI_CS_PIN=10;

//create objects  
MCP_CAN CAN(SPI_CS_PIN);
Servo servo_LeftMotor;
Servo servo_RightMotor;
Servo servo_Swinging;
Servo servo_Hugging;
Servo servo_Extending;

//CAN buf variables
byte leftSpeed=0;
byte rightSpeed=0;
byte leftRev=0;
byte rightRev=0;
byte hugPos=0;
byte armPos=0;
byte swingSpeed=0;
byte leftMotOn=0;
byte rightMotOn=0;
byte swingMotOn=0;
byte hugMotOn=0;
byte extMotOn=0;

//init variables
unsigned int leftMotorSpeed=0;
unsigned int rightMotorSpeed=0;
unsigned int hugArmPos=0;
unsigned int extArmPos=0;
unsigned int swingArmSpeed=0;

//CAN variables
byte len=0;
byte receiveBuf[8];
unsigned long canId;
byte driveBuf[8]={0,0,0,0,0,0,0,0}; //0=leftMotorSpeed, 1=rightMotorSpeed, 2=leftMotorRev, 3=rightMotorRev
byte miscBuf[8]={0,0,0,0,0,0,0,0}; //0=hug arm position, 1=extending arm position, 2=swinging arm speed
byte connBuf[5]={0,0,0,0,0}; //0=left motor on/off, 1=right motor on/off, 2=swinging motor on/off, 3=hugging motor on/off, 4=extending motor on/off

//drive function
void drive(int leftSpeed,int rightSpeed){
  servo_LeftMotor.writeMicroseconds(leftSpeed);
  servo_RightMotor.writeMicroseconds(rightSpeed);
}

void setup() {
  Serial.begin(9600);

  //pinmodes 
  pinMode(tipMotorPin,OUTPUT);
  pinMode(swingMotorPin,OUTPUT);
  pinMode(hugMotorPin,OUTPUT);
  pinMode(LeftMotorPin,OUTPUT);
  pinMode(RightMotorPin,OUTPUT);

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
    Serial.println("receive");
    CAN.readMsgBuf(&len, receiveBuf); //reading CAN message
    canId=CAN.getCanId();  //read the message id associated with this CAN message

  Serial.print("CAN ID: ");
  Serial.println(canId);
    if(canId==0x01){
    for(int i=0;i<8;i++){
      driveBuf[i]=receiveBuf[i];
    }
    }
    else if(canId=0x02){
      for(int i=0;i<8;i++){
      miscBuf[i]=receiveBuf[i];
    }
    }
    else if(canId==0x03){
    if(connBuf[0]==1)
    servo_LeftMotor.attach(LeftMotorPin);
    else
    servo_LeftMotor.detach(); 
    if(connBuf[1]==1)
    servo_RightMotor.attach(RightMotorPin);
    else
    servo_RightMotor.detach(); 
    if(connBuf[2]==1)
    servo_Swinging.attach(swingMotorPin);
    else
    servo_Swinging.detach(); 
    if(connBuf[3]==1)
    servo_Hugging.attach(hugMotorPin);
    else
    servo_Hugging.detach(); 
    if(connBuf[4]==1)
    servo_Extending.attach(tipMotorPin);
    else
    servo_Extending.detach(); 
    }
  }
  
  hugArmPos=miscBuf[0];
  extArmPos=miscBuf[1];
  swingArmSpeed=miscBuf[2];
  
  
  //set drive motor speeds
  if(driveBuf[1]==0) leftMotorSpeed=1500-driveBuf[0];
  else leftMotorSpeed=1500+driveBuf[0];
  if(driveBuf[3]==0) rightMotorSpeed=1500-driveBuf[2];
  else rightMotorSpeed=1500+driveBuf[2];

  Serial.println(leftMotorSpeed);
  Serial.println(rightMotorSpeed);

  //drive motors
  drive(leftMotorSpeed, rightMotorSpeed);
  servo_Hugging.write(hugArmPos);
  servo_Extending.write(extArmPos);
  servo_Swinging.writeMicroseconds(swingArmSpeed+1500);
  
}
