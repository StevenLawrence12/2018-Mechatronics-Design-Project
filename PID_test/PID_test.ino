
#include<mcp_can.h>
#include<SPI.h>
#include<PID_v1.h>

const int sideTrigPin1=4;
const int sideEchoPin1=5;
const int SPI_CS_PIN=10; 

MCP_CAN CAN(SPI_CS_PIN);

int leftMotorDrive=145.7;
int rightMotorDrive=150;

double setPoint,input,output;
  double Kp=, Ki=9, Kd=;

PID myPID(&input,&output,&setPoint,Kp,Ki,Kd,REVERSE);

unsigned long motorDriveId=0x01;
byte motorDriveBuf[8];

unsigned int ultrasonicRead(const int trigPin, const int echoPin);
void sendCANMsg(unsigned long *msgId,byte *msgBuf);

void setup() {
 Serial.begin(9600);

input=ultrasonicRead(sideTrigPin1,sideEchoPin1);
setPoint=8;

myPID.SetMode(AUTOMATIC);
myPID.SetOutputLimits(-100,100);
pinMode(sideTrigPin1,OUTPUT);
pinMode(sideEchoPin1,INPUT);

//init CAN
while(CAN_OK!=CAN.begin(CAN_500KBPS)){
  Serial.println("CAN BUS init fail");
  Serial.println("Init CAN BUS fail again");
  delay(100);
}Serial.println("CAN BUS init ok!");

}

void loop() {

  
input=ultrasonicRead(sideTrigPin1,sideEchoPin1);
Serial.print("Input: ");
Serial.println(input);
myPID.Compute();
Serial.print("Output: ");
Serial.println(output);
leftMotorDrive=145.7+output;
Serial.println(leftMotorDrive);
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



