void setup() {
   // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  //analogReference(INTERNAL);
 
}

void loop() {
 Serial.println("C++");
 delay(1000); 
}
