#include <IRremote.hpp>

// ---------------- Pins ----------------
#define IR_PIN A0   // A0 = digital pin 14. Pin 13 (onboard LED) did not work with IRremote here

// Segment pins: A B C D E F G DP
const int segments[] = {1, 2, 3, 4, 5, 6, 7, 8};

// Digit pins: left -> right (index 0..3)
const int digits[] = {9, 10, 11, 12};

// A B C D E F G  (1 = ON, common cathode)
const byte numbers[10][7] = {
  {1, 1, 1, 1, 1, 1, 0},  // 0
  {0, 1, 1, 0, 0, 0, 0},  // 1
  {1, 1, 0, 1, 1, 0, 1},  // 2
  {1, 1, 1, 1, 0, 0, 1},  // 3
  {0, 1, 1, 0, 0, 1, 1},  // 4
  {1, 0, 1, 1, 0, 1, 1},  // 5
  {1, 0, 1, 1, 1, 1, 1},  // 6
  {1, 1, 1, 0, 0, 0, 0},  // 7
  {1, 1, 1, 1, 1, 1, 1},  // 8
  {1, 1, 1, 1, 0, 1, 1}   // 9
};

// ---------------- Remote codes ----------------
const uint8_t CMD_PLAY_PAUSE = 67;
const uint8_t CMD_EDIT       = 70;  // CH
const uint8_t CMD_LEFT       = 69;  // CH-
const uint8_t CMD_RIGHT      = 71;  // CH+
const uint8_t CMD_MINUS      = 7;   // VOL-
const uint8_t CMD_PLUS       = 21;  // VOL+
const uint8_t CMD_ADD1       = 25;  // 100+
const uint8_t CMD_ADD2       = 13;  // 200+
const uint8_t CMD_RESET      = 9;   // EQ
const uint8_t CMD_FASTER     = 64;  // NEXT (>>)
const uint8_t CMD_SLOWER     = 68;  // PREV (<<)

// index = the digit it writes (0..9)
const uint8_t DIGIT_CODES[10] = {22, 12, 24, 94, 8, 28, 90, 66, 82, 74};

// ---------------- State ----------------
int dig[4] = {0, 0, 0, 0};      // what is shown: SSS.T  (dig[3] = tenths)

bool running = false;
unsigned long lastTickUs = 0;   // for counting (micros, so we can tick faster than 0.1 s)

// Speed ladder: negative = counting backward. 1 = real time (1 s per second).
const int8_t SPEEDS[] = {-64, -32, -16, -8, -4, -2, -1, 1, 2, 4, 8, 16, 32, 64};
const int NUM_SPEEDS = sizeof(SPEEDS) / sizeof(SPEEDS[0]);
int speedIdx = 7;               // index of +1x

bool editing = false;
int editPos = 3;                // which digit is being edited
bool blinkVisible = true;
unsigned long lastBlink = 0;

unsigned long lastMux = 0;      // for display multiplexing
int muxPos = 0;

// ---------------- Display ----------------
void allDigitsOff() {
  for (int i = 0; i < 4; i++) digitalWrite(digits[i], HIGH);  // HIGH = off (common cathode)
}

void showDigit(int pos, int num, bool dp) {
  allDigitsOff();                                   // off first -> prevents ghosting
  for (int i = 0; i < 7; i++) digitalWrite(segments[i], numbers[num][i]);
  digitalWrite(segments[7], dp ? HIGH : LOW);
  digitalWrite(digits[pos], LOW);                   // LOW = select this digit
}

// Call as often as possible. Shows one digit per 2 ms (~125 Hz full refresh).
void refreshDisplay() {
  if (micros() - lastMux < 2000) return;
  lastMux = micros();

  bool hide = editing && muxPos == editPos && !blinkVisible;
  if (hide) {
    allDigitsOff();
  } else {
    showDigit(muxPos, dig[muxPos], muxPos == 2);    // dot after 3rd digit
  }
  muxPos = (muxPos + 1) % 4;
}

// ---------------- Logic ----------------
void stepTenth(int dir) {
  int v = dig[0] * 1000 + dig[1] * 100 + dig[2] * 10 + dig[3];
  if (dir > 0) {
    v = (v + 1) % 10000;                            // 999.9 -> 000.0
  } else {
    v = (v == 0) ? 9999 : v - 1;                    // 000.0 -> 999.9
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

void handleCommand(uint8_t cmd) {
  // ----- Play / Pause -----
  if (cmd == CMD_PLAY_PAUSE) {
    if (editing) {                 // leave edit mode and continue counting
      editing = false;
      running = true;
    } else {
      running = !running;
    }
    lastTickUs = micros();
    return;
  }

  // ----- Enter / exit edit mode -----
  if (cmd == CMD_EDIT) {
    if (!editing) {
      editing = true;
      running = false;
      editPos = 3;                 // start at rightmost digit
      resetBlink();
    } else {
      editing = false;             // stay paused
    }
    return;
  }

  // ----- Speed: NEXT = faster forward / slower backward, PREV = the opposite -----
  // Only changes the speed setting; it does not start or stop counting.
  if (cmd == CMD_FASTER || cmd == CMD_SLOWER) {
    if (cmd == CMD_FASTER && speedIdx < NUM_SPEEDS - 1) speedIdx++;
    if (cmd == CMD_SLOWER && speedIdx > 0) speedIdx--;
    lastTickUs = micros();
    return;
  }

  // ----- Reset to 000.0 (works in any mode; mode itself is unchanged) -----
  if (cmd == CMD_RESET) {
    for (int i = 0; i < 4; i++) dig[i] = 0;
    speedIdx = 7;                  // back to +1x (index 7 in SPEEDS)
    lastTickUs = micros();         // next tick starts fresh after reset
    resetBlink();
    return;
  }

  if (!editing) return;            // everything below only works in edit mode

  if (cmd == CMD_LEFT)       editPos = (editPos + 3) % 4;
  else if (cmd == CMD_RIGHT) editPos = (editPos + 1) % 4;
  else if (cmd == CMD_MINUS) dig[editPos] = (dig[editPos] + 9) % 10;
  else if (cmd == CMD_PLUS)  dig[editPos] = (dig[editPos] + 1) % 10;
  else if (cmd == CMD_ADD1)  dig[0] = (dig[0] + 1) % 10;
  else if (cmd == CMD_ADD2)  dig[0] = (dig[0] + 2) % 10;
  else {
    for (int n = 0; n < 10; n++) {
      if (cmd == DIGIT_CODES[n]) { dig[editPos] = n; break; }
    }
  }
  resetBlink();                    // show the edited digit immediately
}

// ---------------- Arduino ----------------
void setup() {
  for (int i = 0; i < 8; i++) pinMode(segments[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(digits[i], OUTPUT);
  allDigitsOff();

  // No Serial on purpose: pin 1 (TX) drives segment A.
  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);
}

void loop() {
  refreshDisplay();

  // ----- IR -----
  if (IrReceiver.decode()) {
    bool isRepeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;
    bool isKnown  = IrReceiver.decodedIRData.protocol != UNKNOWN;
    if (isKnown && !isRepeat) {
      handleCommand(IrReceiver.decodedIRData.command);
    }
    IrReceiver.resume();           // ready for next signal
  }

  // ----- Counting: one step = 0.1 s of display time; real interval = 100 ms / |speed| -----
  if (running) {
    int speed = SPEEDS[speedIdx];
    unsigned long interval = 100000UL / (speed > 0 ? speed : -speed);
    if (micros() - lastTickUs >= interval) {
      lastTickUs += interval;      // += keeps long-term accuracy (no drift)
      stepTenth(speed > 0 ? 1 : -1);
    }
  }

  // ----- Blinking (0.5 s on / 0.5 s off) -----
  if (editing && millis() - lastBlink >= 500) {
    lastBlink = millis();
    blinkVisible = !blinkVisible;
  }
}
