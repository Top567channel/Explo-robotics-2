#include <IRremote.hpp>

// ---------------- Pins ----------------

#define IR_PIN A0

// 74HC595 pins
const int DATA_PIN  = 4;  // 74HC595 pin 14 (SER)
const int CLOCK_PIN = 5;  // 74HC595 pin 11 (SRCLK)
const int LATCH_PIN = 6;  // 74HC595 pin 12 (RCLK)

// Digit pins: left -> right
const int digits[] = {9, 10, 11, 12};


// ---------------- 7-segment numbers ----------------

// A B C D E F G
// 1 = ON
// Common cathode

const byte numbers[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};


// ---------------- Remote codes ----------------

const uint8_t CMD_PLAY_PAUSE = 67;

const uint8_t CMD_EDIT = 70;
const uint8_t CMD_LEFT = 69;
const uint8_t CMD_RIGHT = 71;

const uint8_t CMD_MINUS = 7;
const uint8_t CMD_PLUS = 21;

const uint8_t CMD_ADD1 = 25;
const uint8_t CMD_ADD2 = 13;

const uint8_t CMD_RESET = 9;

const uint8_t CMD_FASTER = 64;
const uint8_t CMD_SLOWER = 68;


// index = digit it writes
const uint8_t DIGIT_CODES[10] = {
  22, 12, 24, 94, 8,
  28, 90, 66, 82, 74
};


// ---------------- State ----------------

int dig[4] = {0, 0, 0, 0};

bool running = false;

unsigned long lastTickUs = 0;


// Speed ladder
const int8_t SPEEDS[] = {
  -64, -32, -16, -8, -4, -2, -1,
   1,   2,   4,   8, 16, 32, 64
};

const int NUM_SPEEDS =
  sizeof(SPEEDS) / sizeof(SPEEDS[0]);

int speedIdx = 7;


// Editing
bool editing = false;

int editPos = 3;

bool blinkVisible = true;

unsigned long lastBlink = 0;


// Display multiplexing
unsigned long lastMux = 0;

int muxPos = 0;


// ======================================================
// 74HC595 DISPLAY
// ======================================================

void writeSegments(int num, bool dp) {

  byte pattern = 0;

  // Build the 8-bit pattern.
  //
  // Bit 0 = A
  // Bit 1 = B
  // Bit 2 = C
  // Bit 3 = D
  // Bit 4 = E
  // Bit 5 = F
  // Bit 6 = G
  // Bit 7 = DP

  for (int i = 0; i < 7; i++) {

    if (numbers[num][i]) {
      pattern |= (1 << i);
    }

  }

  // Decimal point
  if (dp) {
    pattern |= (1 << 7);
  }

  // Send the 8 bits to the 74HC595

  digitalWrite(LATCH_PIN, LOW);

  shiftOut(
    DATA_PIN,
    CLOCK_PIN,
    MSBFIRST,
    pattern
  );

  digitalWrite(LATCH_PIN, HIGH);
}


// Turn all four digits off
void allDigitsOff() {

  for (int i = 0; i < 4; i++) {
    digitalWrite(digits[i], HIGH);
  }

}


// Show one digit
void showDigit(int pos, int num, bool dp) {

  // First turn every digit off.
  // This prevents ghosting.

  allDigitsOff();

  // Tell the 74HC595 what segments to illuminate.

  writeSegments(num, dp);

  // Select this particular digit.

  digitalWrite(digits[pos], LOW);
}


// Called continuously.
// One digit is displayed every 2 ms.
// Full display refresh = ~125 Hz.

void refreshDisplay() {

  if (micros() - lastMux < 2000)
    return;

  lastMux = micros();

  bool hide =
    editing &&
    muxPos == editPos &&
    !blinkVisible;


  if (hide) {

    allDigitsOff();

  } else {

    // Decimal point after third digit

    showDigit(
      muxPos,
      dig[muxPos],
      muxPos == 2
    );

  }

  muxPos++;

  if (muxPos >= 4)
    muxPos = 0;
}


// ======================================================
// STOPWATCH LOGIC
// ======================================================

void stepTenth(int dir) {

  int v =
    dig[0] * 1000 +
    dig[1] * 100 +
    dig[2] * 10 +
    dig[3];


  if (dir > 0) {

    v = (v + 1) % 10000;

  } else {

    v = (v == 0) ? 9999 : v - 1;

  }


  dig[0] = v / 1000;
  dig[1] = (v / 100) % 10;
  dig[2] = (v / 10) % 10;
  dig[3] = v % 10;
}


void resetBlink() {

  blinkVisible = true;
  lastBlink = millis();

}


// ======================================================
// IR COMMANDS
// ======================================================

void handleCommand(uint8_t cmd) {

  // ----- Play / Pause -----

  if (cmd == CMD_PLAY_PAUSE) {

    if (editing) {

      editing = false;
      running = true;

    } else {

      running = !running;

    }

    lastTickUs = micros();

    return;
  }


  // ----- Enter / Exit edit mode -----

  if (cmd == CMD_EDIT) {

    if (!editing) {

      editing = true;
      running = false;

      editPos = 3;

      resetBlink();

    } else {

      editing = false;

    }

    return;
  }


  // ----- Speed -----

  if (cmd == CMD_FASTER ||
      cmd == CMD_SLOWER) {

    if (
      cmd == CMD_FASTER &&
      speedIdx < NUM_SPEEDS - 1
    ) {
      speedIdx++;
    }

    if (
      cmd == CMD_SLOWER &&
      speedIdx > 0
    ) {
      speedIdx--;
    }

    lastTickUs = micros();

    return;
  }


  // ----- Reset -----

  if (cmd == CMD_RESET) {

    for (int i = 0; i < 4; i++) {
      dig[i] = 0;
    }

    speedIdx = 7;

    lastTickUs = micros();

    resetBlink();

    return;
  }


  // Everything below only works in edit mode.

  if (!editing)
    return;


  // Move left

  if (cmd == CMD_LEFT) {

    editPos =
      (editPos + 3) % 4;

  }


  // Move right

  else if (cmd == CMD_RIGHT) {

    editPos =
      (editPos + 1) % 4;

  }


  // Decrease selected digit

  else if (cmd == CMD_MINUS) {

    dig[editPos] =
      (dig[editPos] + 9) % 10;

  }


  // Increase selected digit

  else if (cmd == CMD_PLUS) {

    dig[editPos] =
      (dig[editPos] + 1) % 10;

  }


  // 100+

  else if (cmd == CMD_ADD1) {

    dig[0] =
      (dig[0] + 1) % 10;

  }


  // 200+

  else if (cmd == CMD_ADD2) {

    dig[0] =
      (dig[0] + 2) % 10;

  }


  // Number buttons

  else {

    for (int n = 0; n < 10; n++) {

      if (cmd == DIGIT_CODES[n]) {

        dig[editPos] = n;

        break;

      }

    }

  }


  resetBlink();
}


// ======================================================
// ARDUINO SETUP
// ======================================================

void setup() {

  // 74HC595 control pins

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);


  // Four digit-select pins

  for (int i = 0; i < 4; i++) {
    pinMode(digits[i], OUTPUT);
  }


  // Start with all digits off

  allDigitsOff();


  // Start IR receiver

  IrReceiver.begin(
    IR_PIN,
    DISABLE_LED_FEEDBACK
  );

}


// ======================================================
// MAIN LOOP
// ======================================================

void loop() {

  refreshDisplay();


  // ----- IR -----

  if (IrReceiver.decode()) {

    bool isRepeat =
      IrReceiver.decodedIRData.flags &
      IRDATA_FLAGS_IS_REPEAT;

    bool isKnown =
      IrReceiver.decodedIRData.protocol !=
      UNKNOWN;


    if (isKnown && !isRepeat) {

      handleCommand(
        IrReceiver.decodedIRData.command
      );

    }


    IrReceiver.resume();
  }


  // ----- Counting -----

  if (running) {

    int speed =
      SPEEDS[speedIdx];


    unsigned long interval =
      100000UL /
      (speed > 0 ? speed : -speed);


    if (
      micros() - lastTickUs >=
      interval
    ) {

      lastTickUs += interval;

      stepTenth(
        speed > 0 ? 1 : -1
      );

    }

  }


  // ----- Blinking -----

  if (
    editing &&
    millis() - lastBlink >= 500
  ) {

    lastBlink = millis();

    blinkVisible =
      !blinkVisible;

  }

}