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

int lastLeft=0;
int lastRight=0;
int lastSwing=0;
int lastHug=0;
int lastExt=0;

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

  servo_Swinging.attach(swingMotorPin);
  servo_Extending.attach(tipMotorPin);

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

  for(int i=0; i<8;i++){
    Serial.print(receiveBuf[i]);
    Serial.print(", ");
  }Serial.println();
    if(canId==0x01){
    for(int i=0;i<8;i++){
      driveBuf[i]=receiveBuf[i];
    }
    }
    else if(canId==0x02){
      for(int i=0;i<8;i++){
      miscBuf[i]=receiveBuf[i];
    }
    }

    else if(canId==0x03){
      for(int i=0;i<5;i++){
      connBuf[i]=receiveBuf[i];
    }
    }
  }
if(connBuf[0]!=lastLeft){
  lastLeft=connBuf[0];
  if(connBuf[0]==1){
  servo_LeftMotor.attach(LeftMotorPin);
  Serial.println("Left motor attached");
  }
  else {
    servo_LeftMotor.detach();
    Serial.println("Left motor dettached");
  }
}
if(connBuf[1]!=lastRight){
  lastRight=connBuf[1];
   if(connBuf[1]==1){
    servo_RightMotor.attach(RightMotorPin);
    Serial.println("Right motor attached");
    }
    else{
    servo_RightMotor.detach(); 
    Serial.println("Right motor dettached");
    }
}
if(connBuf[2]!=lastSwing){
  lastSwing=connBuf[2];
  if(connBuf[2]==1){
    servo_Swinging.attach(swingMotorPin);
    Serial.println("swinging motor attached");
    }
    else{
    servo_Swinging.detach(); 
    Serial.println("swinging motor detattched");
    }
}
if(connBuf[3]!=lastHug){
  lastHug=connBuf[3];
  if(connBuf[3]==1){
    servo_Hugging.attach(hugMotorPin);
    Serial.println("hugging motor attached");
    }
    else{
    servo_Hugging.detach(); 
    Serial.println("hugging motor dettached");
    }
}
if(connBuf[4]!=lastExt){
  lastExt=connBuf[4];
  if(connBuf[4]==1){
    servo_Extending.attach(tipMotorPin);
    Serial.println("extending motor attached" );
    }
    else{
    servo_Extending.detach(); 
    Serial.println("extending motor dettched");
    }
}
  

  hugArmPos=miscBuf[0];
  extArmPos=miscBuf[1];
  /*Serial.println(hugArmPos);
  Serial.println(extArmPos);*/
  swingArmSpeed=miscBuf[2];
  
  
  //set drive motor speeds
  if(driveBuf[2]==1) leftMotorSpeed=1500-driveBuf[0];
  else leftMotorSpeed=1500+driveBuf[0];
  if(driveBuf[3]==1) rightMotorSpeed=1500-driveBuf[1];
  else rightMotorSpeed=1500+driveBuf[1];

  Serial.println(leftMotorSpeed);
  Serial.println(rightMotorSpeed);

  //drive motors
  drive(leftMotorSpeed, rightMotorSpeed);
  servo_Hugging.write(hugArmPos);
  servo_Extending.write(extArmPos);
  servo_Swinging.writeMicroseconds(swingArmSpeed+1500);
  
}
