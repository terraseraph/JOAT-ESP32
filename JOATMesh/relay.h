///////////////////////////////
// relay pins
//////////////////////////////
#define REL_PIN1 34 // relay pin
#define REL_PIN2 35
#define REL_PIN3 32
#define REL_PIN4 33
#define REL_PIN5 25
#define REL_PIN6 26
#define REL_PIN7 27
#define REL_PIN8 14

#define RELAY_TOGGLE_DELAY 100

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

  pinMode(REL_PIN1, INPUT_PULLUP);
  pinMode(REL_PIN1, OUTPUT);

  pinMode(REL_PIN2, INPUT_PULLUP);
  pinMode(REL_PIN2, OUTPUT);

  pinMode(REL_PIN3, INPUT_PULLUP);
  pinMode(REL_PIN3, OUTPUT);

  pinMode(REL_PIN4, INPUT_PULLUP);
  pinMode(REL_PIN4, OUTPUT);

  pinMode(REL_PIN5, INPUT_PULLUP);
  pinMode(REL_PIN5, OUTPUT);

  pinMode(REL_PIN6, INPUT_PULLUP);
  pinMode(REL_PIN6, OUTPUT);

  pinMode(REL_PIN7, INPUT_PULLUP);
  pinMode(REL_PIN7, OUTPUT);

  pinMode(REL_PIN8, INPUT_PULLUP);
  pinMode(REL_PIN8, OUTPUT);

  NODE_TYPE = "relay";
  setNodeType("relay");
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
    digitalWrite(pinNo, HIGH);
    Serial.print(pinNo);
    Serial.println(": Relay on");
  }
  if (action == "stop")
  {
    digitalWrite(pinNo, LOW);
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
