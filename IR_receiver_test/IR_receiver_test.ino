#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 11); // RX, TX
char reed;

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("MSE 2202 IR tester");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(2400);
  //mySerial.println("Hello, world?");
  
}

 

void loop() { // run over and over

  if (mySerial.available())
  {
    reed=mySerial.read();
    /*Serial.write(reed);
    Serial.println();*/
    if((reed=='A')||(reed=='E'))Serial.println("Pyramid found!");
    else Serial.println("Coder not correct!");
  }
   //Serial.println("Pyramid not found!");
}
