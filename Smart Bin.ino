#include <SPI.h>                             // FOR UART COMMUNICATION
#include <DMD2.h>                            // FOR P10 DISPLAY
#include <fonts/SystemFont5x7.h>             // FONT STYLE IN P10
#include <Servo.h>                           // FOR OPERATING SERVO (here 360* SERVO MOTOR)
#include <SoftwareSerial.h>                  // FOR SERIAL COMUNICATION B/W ARDUINO & RASPBERRY PI

#define DISPLAYS_ACROSS 1                   //>  FOR P10 DISPLAY'S
#define DISPLAYS_DOWN 1                     //>               MAX & MIN ROW TO PRINT LETTER

volatile bool IRSFlag = false;              // FOR IRS PROTOCOL

Servo MG996R1;                              //>  BIO & NONBIO 
Servo MG996R2;                              //>             BIN'S SERVO MOTOR CLASS OBJECT

const int SerMid = 90, BioC = 34, BioAC = 35, NbioC = 41, NbioAC = 42; // C,AC=> CLOCK & ANTICLOCK WISE MOTION DETECT PIN 
const int S1flag = 1, S2flag = 2;  // SFlag => AT THE POINT SERVO SELECTOR

const int trigPin_b = 2;                        //> ULTRA 
const int echoPin_b = 4;                        //>       SONIC SENSOR 
const int trigPin_n = 10;                       //>                    FOR BIO & NONBIO 
const int echoPin_n = 12;                       //>                                      BIN'S WASTE DROP DETECTION

int WASTE_RADIUS = 30;         // IF WASTE DROP UNDER THIS RADIUS RESPECTED BIN CLOSED (CHANGE IF NEEDED)

SoftDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);  //>  P10 DISPLAY
DMD_TextBox box(dmd, 0, 4, 34, 0);            //>              CONFIGURATION (CHANGE IF NEEDED)

void scrollMessage(const char *msg, int delayTime = 200) {    // FUNCTION TO DISPLAY SENTANCE IN P10   
  box.clear();    // RESET P10 SCREEN 
  while (*msg) {  
    box.print(*msg);     
    Serial.print(*msg);  // LOGIC TO DISPLAY CHAR ONE BY ONE
    delay(delayTime);    
    msg++;
  }
  delay(1000);
}

void setup() {
  Serial.begin(9600);               // SERIAL MONITOR ACCESSING 
  pinMode(2,INPUT_PULLUP);          // FOR SAY IF 2 PIN GIVES 1 THEN IRS PROTOCOL ACTIVATED
  attachInterrupt(digitalPinToInterrupt(2), IRSprotocal, FALLING);   // ACCESS THE INTERRUPT PIN IN ARDUINO & CHANGE DIGITAL PIN TO INTERRUPT PIN MODE

  MG996R1.attach(9);              //>   SERVO MOTOR IN BIO AND NON-BIO BIN'S
  MG996R1.write(SerMid);          //>         SET TO 9 TO 11 th PIN
  MG996R2.attach(11);             //>        SET BOTH MOTOR IN SERVOS
  MG996R2.write(SerMid);          //>            MIDDLE POSITION     (CHANGE IF NEEDED)

  dmd.setBrightness(255);              // |> P10 BRIGTNESS (CHANGE IF NEEDED)
  dmd.selectFont(SystemFont5x7);       // |> P10 DISPLAY FONT
  dmd.begin();                         // |> P10 START DISPLAYING

  pinMode(trigPin_b, OUTPUT);    //>  ULTRA SONIC 1 TRIGGER PIN 
  pinMode(echoPin_b, INPUT);     //>      ""        ECHO PIN
  pinMode(trigPin_n, OUTPUT);    //>  ULTRA SONIC 2 TRIGGER PIN 
  pinMode(echoPin_n, INPUT);     //>      ""        ECHO PIN

  pinMode(A0, OUTPUT);                // FOR WAKE THE RASPBERRY PI IF IT IN SLEEP MODE
  delay(500);
}

void IRSprotocal(){     // IRS PROTOCOL FUNCTION [!] DON'T WRITE MORE THAN 1 LINE CAUSE IRS PROTOCOL READ ONLY 1 LINE 
  IRSFlag = true;
}

void biobin() {      // FUNCTION OF BIO BIN WHEN BIO WASTE IS DETECTED BY THE MODEL

  clockwiseMovement(BioAC, S1flag);      // STEP 1 => OPEN BIO BIN'S SERVO 

  while(true) {
    float duration, distance;
    digitalWrite(trigPin_b, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin_b, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_b, LOW);
    if(distance > WASTE_RADIUS){ break; }         // SETP 2 => OPEN UNTIL ULTRA SONIC SENSOR DETECT WASTE DROPING
    duration = pulseIn(echoPin_b, HIGH);
    distance = (duration*.0343)/2;
    Serial.print("Distance: ");
    Serial.println(distance);
    delay(100);                  
  }
  
  anticlockwiseMovement(BioC, S1flag);    // STEP 3 => CLOSE BIO BIN'S SERVO

}

void nonbiobin() {     // FUNCTION OF NON-BIO BIN WHEN NONBIO WASTE IS DETECTED BY THE MODEL

  clockwiseMovement(NbioAC, S2flag);   // STEP 1 => OPEN NON-BIO BIN'S SERVO

  while(true) {
    float duration, distance;
    digitalWrite(trigPin_n, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin_n, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_n, LOW);
    if(distance > WASTE_RADIUS){ break; }         // SETP 2 => OPEN UNTIL ULTRA SONIC DSENSOR DETECT WASTE DROPING
    duration = pulseIn(echoPin_n, HIGH);
    distance = (duration*.0343)/2;
    Serial.print("Distance: ");
    Serial.println(distance);
    delay(100);
  }

  anticlockwiseMovement(NbioC, S2flag);   // STEP 3 => CLOSE NON-BIO BIN'S SERVO

}

void clockwiseMovement(int p, int s) {         // FUNCTION TO MOVE SERVO IN CLOCKWISE PARAMETERS F(C , Sflag)                                          
  
    if (s == 1) { // FOR BIO BIN TO OPERATE CLOCKWISE MOTION
      Serial.println("Starting 360-degree clockwise rotating...(bio)");
      MG996R1.write(SerMid);   // SET SERVO AT MIDDLE POTISION INIALLY
      delay(250);             // HERE DELAY WORKS AS SPEED OF SERVO   
      MG996R1.write(70);     // ROTATE THE SERVO IN CLOCKWISED BY 90*(MIDDLE)-70*(CHANGE IF NEEDED)) = 20(DEGREE)
      while (1) {           // OPENING... UNTIL 
        int indicator = digitalRead(p);
        if (indicator == HIGH)   // C PIN GET HITTED BY STICK & THE AC PIN GET HIGH
        {
        MG996R1.write(SerMid);  
        Serial.println("360-degree Clockwise rotation completed (bio)");
        break;
        }
        delay(100);   // REST PERIOD
      }
    }
     
    else if (s == 2) {  // FOR NON-BIO BIN TO OPERATE CLOCKWISE MOTION
      Serial.println("clockwise rotating...(non-bio)");
      MG996R2.write(SerMid);    // SET SERVO AT MIDDLE POTISION INIALLY
      delay(250);              // HERE DELAY WORKS AS SPEED OF SERVO 
      MG996R2.write(70);      // ROTATE THE SERVO IN CLOCKWISED BY 90*(MIDDLE)-70*(CHANGE IF NEEDED)) = 20(DEGREE)
      while (1) {            // OPENING... UNTIL 
        int indicator = digitalRead(p);
        if (indicator == HIGH)    // C PIN GET HITTED BY STICK & THE AC PIN GET HIGH
        {    
        MG996R2.write(SerMid);  
        Serial.println("Clockwise rotation completed (non-bio)");
        break;
        }
        delay(100);   // REST PERIOD
      }
    } 
}

void anticlockwiseMovement(int p, int s) {
  
    if (s == 1) {  // FOR BIO BIN TO OPERATE ANTICLOCKWISE MOTION
      Serial.println("anticlockwise rotating...(bio)");      
      MG996R1.write(SerMid);      // SET SERVO AT MIDDLE POTISION INIALLY
      delay(250);                // HERE DELAY WORKS AS SPEED OF SERVO 
      MG996R1.write(130);       // ROTATE THE SERVO IN ANTICLOCKWISED BY 130*(MIDDLE)-90*(CHANGE IF NEEDED)) = 40(DEGREE)
      while (1) {              // CLOSING... UNTIL 
        int indicator = digitalRead(p);
        if (indicator == HIGH)      // AC PIN GET HITTED BY STICK & THE C PIN GET HIGH
        {
        MG996R1.write(SerMid);  
        Serial.println("antiClockwise rotation completed (bio)"); 
        break;
        }
        delay(100);   // REST PERIOD
      }
    } 
    else if (s == 2) { // FOR NON-BIO BIN TO OPERATE ANTICLOCKWISE MOTION
      Serial.println("anticlockwise rotation...(non-bio)");   
      MG996R2.write(SerMid);       // SET SERVO AT MIDDLE POTISION INIALLY
      delay(250);                 // HERE DELAY WORKS AS SPEED OF SERVO 
      MG996R2.write(130);        // ROTATE THE SERVO IN ANTICLOCKWISED BY 130*(MIDDLE)-90*(CHANGE IF NEEDED)) = 40(DEGREE)
      while (1) {               // CLOSING... UNTIL 
        int indicator = digitalRead(p);
        if (indicator == HIGH)       // AC PIN GET HITTED BY STICK & THE C PIN GET HIGH
        {
        MG996R2.write(SerMid);  
        Serial.println("antiClockwise rotation completed (non-bio)"); 
        break;
        }
        delay(100);   // REST PERIOD
      }
    }
}

String receiveData() {                                      //>   RECIVE DATA FROM RASPBERRY ML CODE     
    String receivedData = Serial.readStringUntil('\n');     //>       THAT IT TELL WHEATHER 
    receivedData.trim();                                    //>     BIO OR NON-BIO BIN TO PROCESS
    return receivedData;                                    //>          Range < 0 , 1 >
}

void loop() {                                                                                

  if(IRSFlag){         // IRS PROTOCOL IF PROCESS STUCK AT ANY POINT BY CLICKING THE INTERRUPT BOTTOM...
    IRSFlag = false;
    clockwiseMovement(BioAC, S1flag);            // DEFAULT OPEN THE BIO PIN 
    unsigned long IRSbuffer = millis();  // WAIT FOR 16 SEC
    while (millis() - IRSbuffer < 15000) {
        delay(50);
    }
    anticlockwiseMovement(BioC, S1flag);   // THEN CLOSE THE BIN
  }
          
  while (!Serial.available()) {   // DISPLAY "SMART BIN" IN THE P10 DISPLAY UNTIL BAR CODE SEND DATA TO SERIAL MONITOR
    box.clear();
    scrollMessage("SMART BIN", 200);
    delay(100);
  }


  String received = "HI, " + Serial.readStringUntil('\n');  // DISPLAY HI , + { BAR CODE SCANNED INFO }
  received.trim();
  delay(3000); 

  digitalWrite(A0, HIGH); // WAKEUP THE RASPBERRY IF IT IS IN SLEEP MODE

  scrollMessage(received.c_str(), 200);  //>  DISPLAY BAR SCANNED INFO IN P10         
  scrollMessage(received.c_str(), 200);  //>       2 TIMES

  scrollMessage("SHOW THE WASTE", 200);  // OPTIONAL PART => DISPLAY "SHOW THE WASTE" IN P10 UNTIL RASPBERRY SEND DATA TO SERIAL MONITOR
  delay(2000);

  while (!Serial.available()) {  // IF RASPBEERY SEND'S DATA TO SERIAL 
    delay(100);   // REST PERIOD
  }

  if (receiveData() == "0")  // RASPBEERY PY.SCRIPT DETECT BIO WASTE SEND "0"
  {
    scrollMessage("BIO WASTE", 200);
    Serial.println("BIO bin sequence");
    biobin();  // BIO BIN PROCESS START
  }
  else if (receiveData() == "1")  // RASPBEERY PY.SCRIPT DETECT NON-BIO WASTE SEND "1"
  {
    scrollMessage("NON-BIO WASTE", 200);
    Serial.println("NON-BIO bin sequence");
    nonbiobin();  // NON-BIO BIN PROCESS START
  }
  else 
  {
    Serial.println("INVALID RESPONSE");  // ELSE NO RESPONSE FOUND
  }

  digitalWrite(A0, HIGH);  // // WAKEUP THE RASPBERRY IF IT IS IN SLEEP MODE

  scrollMessage("THANKS, YOUR CREDIT HAS BEEN TRANSFERED", 150);  // DISPLAY THANKS
  scrollMessage("DONE", 200); // THEN FINALLY DISPLAY DONE

  
  unsigned long cooldownStart = millis();
  while (millis() - cooldownStart < 4000) {    // GIVE 4 SEC REST TO AURDINO
    while (Serial.available()) Serial.read(); 
    delay(50);
  }
}
