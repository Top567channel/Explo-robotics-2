#include <IRremote.hpp>
void setup() {
  Serial.begin(9600);
  IrReceiver.begin(0, DISABLE_LED_FEEDBACK);
  Serial.println("ready");
}
void loop() {
  if (IrReceiver.decode()) {
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.resume();
  }
}