#include <IRremote.hpp>

#define IR_PIN 2
#define LIGHT_PIN 5

void setup() {
  Serial.begin(9600);

  pinMode(LIGHT_PIN, OUTPUT);

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  Serial.println("IR Receiver Ready");
}

void loop() {

  if (IrReceiver.decode()) {

    int command = IrReceiver.decodedIRData.command;

    Serial.print("Command received: ");
    Serial.println(command);

    if (command == 24) {
      digitalWrite(LIGHT_PIN, HIGH);
      Serial.println("LIGHT ON");
    }
    else {
      digitalWrite(LIGHT_PIN, LOW);
      Serial.println("LIGHT OFF");
    }

    IrReceiver.resume();
  }
}