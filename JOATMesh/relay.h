///////////////////////////////
// relay pins
// Note:
// For 8 channel relay - Cannot power from esp32 alone
// Relay VCC & GND -> 5V external supply
// COM -> ESP32 3v3/Vin
// Pins then corrospond.
//
// Alternate 8 channel relay
// relay Jvcc & gnd -> external supply
// vcc & pins -> esp32
//////////////////////////////
#include "HardwareSerial.h"

#define REL_PIN1 15 // relay pin
#define REL_PIN2 2
#define REL_PIN3 4
#define REL_PIN4 5
#define REL_PIN5 27
#define REL_PIN6 14
#define REL_PIN7 12
#define REL_PIN8 13

#define RELAY_TOGGLE_DELAY 300

void relayToggleOff();
void relayToggleOn();

Scheduler relayScheduler;
Task toggleOn(RELAY_TOGGLE_DELAY, 1, &relayToggleOn);
Task toggleOff(RELAY_TOGGLE_DELAY, 1, &relayToggleOff);

void relay_init()
{
  relayScheduler.init();
  relayScheduler.addTask(toggleOn);
  relayScheduler.addTask(toggleOff);

  // pinMode(REL_PIN1, INPUT_PULLUP);
  pinMode(REL_PIN1, OUTPUT);
  digitalWrite(REL_PIN1, HIGH);

  // pinMode(REL_PIN2, INPUT_PULLUP);
  pinMode(REL_PIN2, OUTPUT);
  digitalWrite(REL_PIN2, HIGH);

  // pinMode(REL_PIN3, INPUT_PULLUP);
  pinMode(REL_PIN3, OUTPUT);
  digitalWrite(REL_PIN3, HIGH);

  // pinMode(REL_PIN4, INPUT_PULLUP);
  pinMode(REL_PIN4, OUTPUT);
  digitalWrite(REL_PIN4, HIGH);

  // pinMode(REL_PIN5, INPUT_PULLUP);
  pinMode(REL_PIN5, OUTPUT);
  digitalWrite(REL_PIN5, HIGH);

  // pinMode(REL_PIN6, INPUT_PULLUP);
  pinMode(REL_PIN6, OUTPUT);
  digitalWrite(REL_PIN6, HIGH);

  // pinMode(REL_PIN7, INPUT_PULLUP);
  pinMode(REL_PIN7, OUTPUT);
  digitalWrite(REL_PIN7, HIGH);

  // pinMode(REL_PIN8, INPUT_PULLUP);
  pinMode(REL_PIN8, OUTPUT);
  digitalWrite(REL_PIN8, HIGH);

  NODE_TYPE = "relay";
  // setNodeType("relay");
}

void relayToggleOn()
{
  int pinNo = GLOBAL_PIN;
  digitalWrite(pinNo, HIGH);
  Serial.print(pinNo);
  Serial.println(": Relay toggle on");
}

void relayToggleOff()
{
  int pinNo = GLOBAL_PIN;
  digitalWrite(pinNo, LOW);
  Serial.print(pinNo);
  Serial.println(": Relay toggle off");
}

void processRelayAction(String action, int pinNo)
{
  GLOBAL_PIN = pinNo;

  if (action == "start")
  {
    digitalWrite(pinNo, LOW);
    Serial.print(pinNo);
    Serial.println(": Relay on");
  }
  if (action == "stop")
  {
    digitalWrite(pinNo, HIGH);
    Serial.print(pinNo);
    Serial.printf(": Relay off");
  }
  if (action == "toggleA")
  {
    if (digitalRead(pinNo) == HIGH)
    {
      digitalWrite(pinNo, LOW);
      toggleOn.setIterations(1); //ensures the task re runs
      toggleOn.enable();
    }
    else
    {
      digitalWrite(pinNo, HIGH);
      toggleOff.setIterations(1);
      toggleOff.enable();
    }

    Serial.printf("Relay toggle");
  }
}
