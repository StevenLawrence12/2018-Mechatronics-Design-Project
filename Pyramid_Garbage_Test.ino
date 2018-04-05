
void setup() {
  // put your setup code here, to run once:

}

int checkGarbage(int reading, int readingReference, int countNum)
{
  // Check to see if the ultrasonic reading is fluctuating 
  if (reading > (readingReferance + 10) || reading < (readingReference - 10))
  {
    countNum++;
  }
  
  else {
    countNum = 0;
  }

  return countNum;
}

void loop() {
  // put your main code here, to run repeatedly:

  // Initialize ultrasonic distance readings 
  int ultrasonicDReading; 
  int prevUltrasonicReading;
  // Set the reading of the ultrasonic to prev that we will check 2 seconds later 

  prevUltrasonicReading = ultrasonicDReading;
  delay(2);

  // Initalize count variable (if count exceeds 3 then pyramid is there and proceed with finding it)
  int count = 0;
  Serial.println("UltrasonicDReading is: ")
  Serial.println(ultrasonicDReading);
  Serial.println("Previous UltrasonicDReading is: ")
  Serial.println(prevUltrasonicReading);
  count = checkGarbage(ultrasonicDReading, prevUltrasonicReading, count);
  Serial.println("Count is: ")
  Serial.println(count);

  if (count > 3)
  {
    // Reset the count for future cases
    Serial.println("Garbage found!")
    count = 0;
    // Call pyramid finding function
  }
}
