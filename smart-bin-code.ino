#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1

const int Servo_bio = 3;
const int Servo_nonbio = 5;

const int trigPin_b = 2;
const int echoPin_b = 4;

const int trigPin_n = 10;
const int echoPin_n = 12;

const int angle = 115;         //change the angle as needed
int pos = 0;
int no_waste_dis = 30;         //change the value as needed

Servo myservo_b;
Servo myservo_n;

SoftDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
DMD_TextBox box(dmd, 0, 4, 34, 0); // Adjust size for your display

void scrollMessage(const char *msg, int delayTime = 200) {
  box.clear();
  while (*msg) {
    box.print(*msg);     // Print one character
    Serial.print(*msg);  // Echo to Serial
    delay(delayTime);    
    msg++;
  }
  delay(1000);
}

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), IRSprotocal, FALLING);
  dmd.setBrightness(255);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();
  myservo_b.attach(Servo_bio);
  myservo_n.attach(Servo_nonbio);
  pinMode(trigPin_b, OUTPUT);
  pinMode(echoPin_b, INPUT);
  pinMode(trigPin_n, OUTPUT);
  pinMode(echoPin_n, INPUT);
  pinMode(A0, OUTPUT);               //For controlling Raspberry pi
  delay(500);
}

void IRSprotocal() {
  biobin();
} 

void biobin() {
  int flag = 0;
  
  for (pos = 0; pos <= angle; pos += 1) { 
    myservo_b.write(pos);              
    delay(15);                       
  }

  while(flag!=1) {

    float duration, distance;
    digitalWrite(trigPin_b, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin_b, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_b, LOW);

    duration = pulseIn(echoPin_b, HIGH);
    distance = (duration*.0343)/2;
    Serial.print("Distance: ");
    Serial.println(distance);
    delay(100);

    if (distance > no_waste_dis){
      flag = 1;
    }     

    delay(2500);

  }
  
  for (pos = angle; pos >= 0; pos -= 1) {
    myservo_b.write(pos);              
    delay(15);                       
  }

}

void nonbiobin() {
  int flag = 0;
  
  for (pos = 0; pos <= angle; pos += 1) { 
    myservo_n.write(pos);              
    delay(15);                       
  }

  while(flag!=1) {

    float duration, distance;
    digitalWrite(trigPin_n, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin_n, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_n, LOW);

    duration = pulseIn(echoPin_n, HIGH);
    distance = (duration*.0343)/2;
    Serial.print("Distance: ");
    Serial.println(distance);
    delay(100);

    if (distance > no_waste_dis){
      flag = 1;
    }     

    delay(2500);

  }
  
  for (pos = angle; pos >= 0; pos -= 1) {
    myservo_n.write(pos);              
    delay(15);                       
  }

}

String receiveData() {
    String receivedData = Serial.readStringUntil('\n');  // Read incoming data line
    receivedData.trim(); // Remove any trailing \r or whitespace
    return receivedData;
}

void loop() {

  scrollMessage("THANKS, YOUR CREDIT HAS BEEN TRANSFERED", 150);

  // 2. Wait for input from Serial
  while (!Serial.available()) {
    // Stay in this loop displaying WAITING until data arrives
    box.clear();
    scrollMessage("SMART BIN", 200);
    delay(100);
  }

  // 3. Read and scroll the data
  String received = "HI, " + Serial.readStringUntil('\n');
  received.trim();
  delay(3000); // 3-second delay before display

  digitalWrite(A0, HIGH);

  scrollMessage(received.c_str(), 200);
  scrollMessage(received.c_str(), 200);

  scrollMessage("SHOW THE WASTE", 200);
  delay(2000);

  while (!Serial.available()) {
    // Stay in this loop displaying WAITING until data arrives
    delay(100);
  }

  if (receiveData() == '0')
  {
    scrollMessage("BIO WASTE", 200);
    Serial.println("BIO bin sequence");
    biobin();
  }
  else if (receiveData() == '1')
  {
    scrollMessage("NON-BIO WASTE", 200);
    Serial.println("NON-BIO bin sequence");
    nonbiobin();
  }
  else 
  {
    Serial.println("INVALID RESPONSE");
  }

  digitalWrite(A0, HIGH);

  // 4. Display "DONE"
  scrollMessage("DONE", 200);

  // 5. Wait for 10 seconds (ignoring any incoming data)
  unsigned long cooldownStart = millis();
  while (millis() - cooldownStart < 4000) {
    while (Serial.available()) Serial.read(); // Clear input buffer
    delay(50);
  }
}