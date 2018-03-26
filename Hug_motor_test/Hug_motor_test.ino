
//librarys
#include <Servo.h>

//pins
const int hugMotorPin=7;

//objects
Servo servo_hugMotor;

void setup() {
Serial.begin(9600);

//pinmodes
pinMode(hugMotorPin,OUTPUT);

//attach servos
servo_hugMotor.attach(hugMotorPin);

//setup initial starting position for arm
servo_hugMotor.writeMicroseconds(1000);
}

void loop() {

//add correct arm positions to the following:
servo_hugMotor.writeMicroseconds(1500);
servo_hugMotor.writeMicroseconds(2000);
}
