// ==========================================
// 1. กำหนดขาอุปกรณ์ (ตามที่คุณตั้งค่าไว้ล่าสุด)
// ==========================================
// Motor A (มอเตอร์ฝั่งซ้าย)
const int enA = 3;
const int in1 = 2;
const int in2 = 4;

// Motor B (มอเตอร์ฝั่งขวา)
const int enB = 6;
const int in3 = 5;
const int in4 = 7;

// IR Sensors (เซ็นเซอร์อินฟราเรด)
const int irLeft = 11;  // เซ็นเซอร์ตัวซ้าย
const int irRight = 12; // เซ็นเซอร์ตัวขวา

// กำหนดความเร็วของรถ (0-255)
// **แนะนำให้เริ่มที่ 100-150** ถ้ารถวิ่งเร็วไป เซ็นเซอร์จะอ่านเส้นไม่ทันและหลุดโค้ง
int speed = 130; 
int turnSpeed = 190;

void setup() {
  // ตั้งค่าขามอเตอร์เป็น OUTPUT
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // ตั้งค่าขาเซ็นเซอร์เป็น INPUT
  pinMode(irLeft, INPUT);
  pinMode(irRight, INPUT);

  // เริ่มต้นโปรแกรมให้รถหยุดนิ่งก่อน
  stopRobot();
  delay(2000); // หน่วงเวลา 2 วินาทีก่อนเริ่มวิ่ง เพื่อให้เราวางรถลงบนพื้นได้ทัน
}

void loop() {
  // อ่านค่าจากเซ็นเซอร์
  int leftValue = digitalRead(irLeft);
  int rightValue = digitalRead(irRight);

  // ==========================================
  // 2. ลอจิกการตัดสินใจ (เดินเส้น)
  // ==========================================
  
  if (leftValue == 0 && rightValue == 0) {
    // 2.1 ไม่เจอเส้นทั้งคู่ (อยู่บนพื้นขาว) -> วิ่งตรงไป
    moveForward();
  } 
  else if (leftValue == 1 && rightValue == 0) {
    // 2.2 เซ็นเซอร์ซ้ายเจอเส้นดำ -> รถต้องเลี้ยวซ้าย
    turnLeft();
  } 
  else if (leftValue == 0 && rightValue == 1) {
    // 2.3 เซ็นเซอร์ขวาเจอเส้นดำ -> รถต้องเลี้ยวขวา
    turnRight();
  } 
  else if (leftValue == 1 && rightValue == 1) {
    // 2.4 เจอเส้นดำทั้งคู่ (ทางขวาง/จุดตัด) -> หยุดรถ
    stopRobot();
  }
}
// ==========================================
// 3. ฟังก์ชันควบคุมการเคลื่อนที่ของรถ
// ==========================================
// หมายเหตุ: เนื่องจากคุณเคยแจ้งว่าต่อสายมอเตอร์สลับทิศกับในรูป 
// หากรถวิ่งผิดทิศ (เช่น สั่งเดินหน้าแต่มันถอยหลัง) ให้สลับคำว่า HIGH กับ LOW ในฟังก์ชันนั้นๆ ครับ

void moveForward() {
  // เดินหน้า (มอเตอร์หมุนไปข้างหน้าทั้ง 2 ตัว)
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enA, speed);
  analogWrite(enB, speed);
}

void turnLeft() {
  // เลี้ยวซ้าย (ล้อซ้ายถอยหลัง, ล้อขวาเดินหน้า)
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enA, turnSpeed);
  analogWrite(enB, turnSpeed);
}

void turnRight() {
  // เลี้ยวขวา (ล้อซ้ายเดินหน้า, ล้อขวาถอยหลัง)
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enA, turnSpeed);
  analogWrite(enB, turnSpeed);
}

void stopRobot() {
  // หยุดรถ
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, 0);
}