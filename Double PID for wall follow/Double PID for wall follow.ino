#include<mcp_can.h>
#include<SPI.h>
#include<PID_v1.h>

const int sideTrigPin1=4;
const int sideEchoPin1=5;
const int sideTrigPin2=6;
const int sideEchoPin2=7;
const int SPI_CS_PIN=10; 

MCP_CAN CAN(SPI_CS_PIN);

int leftMotorDrive=145.7;
int rightMotorDrive=150;

double targSideDist,sideDist1,output1;
//int angleError,frontError,leftFactor,rightFactor;
//int factor1=20,factor2=12;
//double sideDist2;
double Kp1=10,Ki1=0,Kd1=16.5;
double sideDist2,output2;
double Kp2=3,Ki2=0,Kd2=5;

PID sideUlt1(&sideDist1,&output1,&targSideDist,Kp1,Ki1,Kd1,REVERSE);
PID sideUlt2(&sideDist2,&output2,&sideDist1,Kp2,Ki2,Kd2,P_ON_M,REVERSE);

unsigned long motorDriveId=0x01;
byte motorDriveBuf[8];

unsigned int ultrasonicRead(const int trigPin, const int echoPin);
void sendCANMsg(unsigned long *msgId,byte *msgBuf);

void setup() {
Serial.begin(9600);
sideDist1=ultrasonicRead(sideTrigPin1,sideEchoPin1);
sideDist2=ultrasonicRead(sideTrigPin2,sideEchoPin2);

targSideDist=8;

sideUlt1.SetMode(AUTOMATIC);
sideUlt2.SetMode(AUTOMATIC);
sideUlt1.SetOutputLimits(-50,50);
sideUlt2.SetOutputLimits(-50,50);
/*sideUlt1.SetSampleTime(10);
sideUlt2.SetSampleTime(10);*/

pinMode(sideTrigPin1,OUTPUT);
pinMode(sideEchoPin1,INPUT);
pinMode(sideTrigPin2,OUTPUT);
pinMode(sideEchoPin2,INPUT);

//init CAN
while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");
}

void loop() {
sideDist1=ultrasonicRead(sideTrigPin1,sideEchoPin1);
sideDist2=ultrasonicRead(sideTrigPin2,sideEchoPin2);
bool one=sideUlt1.Compute();
bool two=sideUlt2.Compute();
Serial.println(one);
Serial.println(two);

/*angleError=sideDist1-sideDist2;
frontError=sideDist1-targSideDist;*/

/*if(angleError>0)leftFactor+=angleError*factor1;
if(angleError<0)rightFactor-=angleError*factor2;
if(frontError>0)leftFactor+=frontError*factor1;
if(frontError<0)rightFactor-=frontError*factor2;*/

leftMotorDrive=100+output1;
rightMotorDrive=100+output2;

motorDriveBuf[0]=leftMotorDrive;
motorDriveBuf[2]=rightMotorDrive;

sendCANMsg(&motorDriveId,motorDriveBuf);
}

unsigned int ultrasonicRead(const int trigPin, const int echoPin){
  //clears the TrigPin
digitalWrite(trigPin,LOW);
delayMicroseconds(2);

//set TrigPin high for 10 microseconds to send pulse out
digitalWrite(trigPin,HIGH);
delayMicroseconds(10);
digitalWrite(trigPin,LOW);

//read the EchoPin
long duration=pulseIn(echoPin,HIGH);

//calculate the distance 0.034cm/us
double distance=duration*0.034/2;
return distance;
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



