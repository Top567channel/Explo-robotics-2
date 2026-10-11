// RGB LED color disk: joystick moves a cursor on the color wheel (rate control).
// Cursor angle = hue, distance from center = saturation, center = white.
// Wiring: R=5, G=6, B=9 (PWM)  |  SW=8  |  VRX=A1, VRY=A0
// LED module "-" -> GND (common cathode)

const uint8_t PIN_R   = 5;
const uint8_t PIN_G   = 6;
const uint8_t PIN_B   = 9;      // must be PWM pin (3,5,6,9,10,11)
const uint8_t PIN_SW  = 8;
const uint8_t PIN_VRX = A1;
const uint8_t PIN_VRY = A0;

const bool  COMMON_ANODE = false;
const bool  USE_GAMMA    = true;

const int   DEADZONE     = 120;     // stick ignored inside this radius
const int   MAX_DEFLECT  = 480;     // full push of the stick
const float CURSOR_SPEED = 0.8;     // disk radii per second at full push (tune!)
const float WHITE_ZONE   = 0.10;    // cursor radius (0..1) that is pure white

// White balance: scale each channel (0..1). Lower blue because white looked bluish.
const float WB_R = 1.0;
const float WB_G = 1.0;
const float WB_B = 0.75;

const unsigned long DEBOUNCE_MS = 30;
const unsigned long PRINT_MS    = 100;

int centerX, centerY;
float px = 0, py = 0;     // cursor on the color disk, |p| <= 1. Starts at white.
bool ledOn = true;

unsigned long lastLoopTime;
unsigned long lastPrintTime = 0;

bool lastReading = HIGH;
bool stableState = HIGH;
unsigned long lastChangeTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_SW, INPUT_PULLUP);

  delay(200);
  centerX = analogRead(PIN_VRX);
  centerY = analogRead(PIN_VRY);
  lastLoopTime = millis();
}

void loop() {
  handleButton();

  unsigned long now = millis();
  float dt = (now - lastLoopTime) / 1000.0;
  lastLoopTime = now;
  if (dt > 0.1) dt = 0.1;           // protect against a long hiccup

  int x = analogRead(PIN_VRX) - centerX;
  int y = centerY - analogRead(PIN_VRY);
  float dist = sqrt((float)x * x + (float)y * y);

  // ---- Stick -> velocity of the cursor (direction * strength) ----
  float vx = 0, vy = 0;
  if (dist > DEADZONE) {
    float push = constrain((dist - DEADZONE) / (float)(MAX_DEFLECT - DEADZONE), 0.0, 1.0);
    vx = (x / dist) * push;
    vy = (y / dist) * push;
  }

  // ---- Integrate velocity into position ----
  px += vx * CURSOR_SPEED * dt;
  py += vy * CURSOR_SPEED * dt;

  // keep the cursor inside the disk (slides along the rim if pushed outward)
  float radius = sqrt(px * px + py * py);
  if (radius > 1.0) {
    px /= radius;
    py /= radius;
    radius = 1.0;
  }

  // ---- Cursor position -> color ----
  float hue = atan2(py, px) * 180.0 / PI;
  if (hue < 0) hue += 360;
  float sat = constrain((radius - WHITE_ZONE) / (1.0 - WHITE_ZONE), 0.0, 1.0);

  uint8_t r = 0, g = 0, b = 0;
  if (ledOn) {
    hsvToRgb(hue, sat, 1.0, r, g, b);
    writeColor(r, g, b);
  } else {
    writeColor(0, 0, 0);
  }

  if (now - lastPrintTime >= PRINT_MS) {
    lastPrintTime = now;
    Serial.print("p=("); Serial.print(px); Serial.print(",");
    Serial.print(py); Serial.print(")");
    Serial.print(" hue="); Serial.print(hue);
    Serial.print(" sat="); Serial.print(sat);
    Serial.print(" RGB="); Serial.print(r);
    Serial.print(","); Serial.print(g);
    Serial.print(","); Serial.println(b);
  }

  delay(10);
}

void handleButton() {
  bool reading = digitalRead(PIN_SW);
  if (reading != lastReading) lastChangeTime = millis();
  if ((millis() - lastChangeTime) > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) ledOn = !ledOn;
  }
  lastReading = reading;
}

uint8_t gammaCorrect(uint8_t v) {
  return (uint8_t)(pow(v / 255.0, 2.2) * 255.0 + 0.5);
}

void writeColor(uint8_t r, uint8_t g, uint8_t b) {
  if (USE_GAMMA) {
    r = gammaCorrect(r);
    g = gammaCorrect(g);
    b = gammaCorrect(b);
  }
  r = r * WB_R;
  g = g * WB_G;
  b = b * WB_B;
  if (COMMON_ANODE) { r = 255 - r; g = 255 - g; b = 255 - b; }
  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
  float c  = v * s;
  float hp = h / 60.0;
  float x  = c * (1 - fabs(fmod(hp, 2) - 1));
  float m  = v - c;
  float rf = 0, gf = 0, bf = 0;

  switch ((int)hp % 6) {
    case 0: rf = c; gf = x; bf = 0; break;
    case 1: rf = x; gf = c; bf = 0; break;
    case 2: rf = 0; gf = c; bf = x; break;
    case 3: rf = 0; gf = x; bf = c; break;
    case 4: rf = x; gf = 0; bf = c; break;
    case 5: rf = c; gf = 0; bf = x; break;
  }
  r = (rf + m) * 255;
  g = (gf + m) * 255;
  b = (bf + m) * 255;
}