#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);
SoftwareSerial finger(2, 3);

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);
  finger.begin(57600);
  
}

void loop() {
  if (mySerial.available()) {
    Serial.println(mySerial.readString());
  }
  
  mySerial.println("11111@1@1@3$4%5^0");

  delay(2000);
}
