#include <Servo.h>

const int potPin = A0;          
const int activeBuzzerPin = 2;  
const int passiveBuzzerPin = 3; 
const int servoPin = 9;         

Servo myServo;
const int baseFreq = 2620; 

void setup() {
  pinMode(activeBuzzerPin, OUTPUT);
  pinMode(passiveBuzzerPin, OUTPUT);
  myServo.attach(servoPin);     
  Serial.begin(9600);
}

void loop() {
  int potValue = analogRead(potPin);

  // ปิดเสียงและหมุนเซอร์โวกลับไปที่ 0 องศาเมื่อบิดลงสุด
  if (potValue < 10) {
    digitalWrite(activeBuzzerPin, LOW);
    noTone(passiveBuzzerPin);
    myServo.write(0);
    Serial.println("Status: Muted & Servo Step 0 (0°)");
  } 
  else {
    digitalWrite(activeBuzzerPin, HIGH); 
    
    int targetFreq = baseFreq;
    int servoAngle = 0;
    String harmonyName = "";

    // กำหนดมุมแบบเป็นขั้นๆ (Steps) ล็อคตามช่วงเสียงประสานทั้ง 15 ระดับ
    if (potValue < 70) {
      targetFreq = baseFreq / 8;
      servoAngle = 0;   // Step 1
      harmonyName = "Triple Sub-Octave (Deep Bass)";
    } 
    else if (potValue < 140) {
      targetFreq = baseFreq / 6;
      servoAngle = 13;  // Step 2
      harmonyName = "Sub-Minor Sixth (Low)";
    } 
    else if (potValue < 210) {
      targetFreq = baseFreq / 5;
      servoAngle = 26;  // Step 3
      harmonyName = "Sub-Major Third (Low)";
    } 
    else if (potValue < 280) {
      targetFreq = baseFreq / 4;
      servoAngle = 39;  // Step 4
      harmonyName = "Double Sub-Octave (Low Bass)";
    } 
    else if (potValue < 350) {
      targetFreq = baseFreq / 3;
      servoAngle = 51;  // Step 5
      harmonyName = "Sub-Twelfth (Low Fifth)";
    } 
    else if (potValue < 420) {
      targetFreq = baseFreq / 2;
      servoAngle = 64;  // Step 6
      harmonyName = "Sub-Octave (Low)";
    } 
    else if (potValue < 490) {
      targetFreq = baseFreq * 3 / 4;
      servoAngle = 77;  // Step 7
      harmonyName = "Low Fourth";
    } 
    else if (potValue < 560) {
      targetFreq = baseFreq;
      servoAngle = 90;  // Step 8 (ตรงกลาง 90°)
      harmonyName = "Unison (Same as Active)";
    } 
    else if (potValue < 630) {
      targetFreq = baseFreq * 5 / 4;
      servoAngle = 103; // Step 9
      harmonyName = "Major Third";
    } 
    else if (potValue < 700) {
      targetFreq = baseFreq * 4 / 3;
      servoAngle = 116; // Step 10
      harmonyName = "Perfect Fourth";
    } 
    else if (potValue < 770) {
      targetFreq = baseFreq * 3 / 2;
      servoAngle = 129; // Step 11
      harmonyName = "Perfect Fifth";
    } 
    else if (potValue < 840) {
      targetFreq = baseFreq * 5 / 3;
      servoAngle = 141; // Step 12
      harmonyName = "Major Sixth";
    } 
    else if (potValue < 910) {
      targetFreq = baseFreq * 2;
      servoAngle = 154; // Step 13
      harmonyName = "High Octave";
    } 
    else if (potValue < 970) {
      targetFreq = baseFreq * 3;
      servoAngle = 167; // Step 14
      harmonyName = "Twelfth (Very High)";
    } 
    else {
      targetFreq = baseFreq * 4;
      servoAngle = 180; // Step 15
      harmonyName = "Double Octave (Extreme High)";
    }

    // สั่งให้มอเตอร์และลำโพงทำงานตรงกัน
    myServo.write(servoAngle);
    tone(passiveBuzzerPin, targetFreq);

    // แสดงผลข้อมูลออกทาง Serial Monitor
    Serial.print("Pot: ");
    Serial.print(potValue);
    Serial.print(" \t| Servo Step Angle: ");
    Serial.print(servoAngle);
    Serial.print("° \t| Harmony: ");
    Serial.print(harmonyName);
    Serial.println();
  }

  delay(40);
}