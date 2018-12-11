#include "EEPROM.h"

int addr = 0;
#define EEPROM_SIZE 64

#define TYPE_RELAY 0
#define TYPE_BUTTON 1
#define TYPE_KEYPAD 2
#define TYPE_MAGSWITCH 3
//enum NodeType {
//  relay,
//  button,
//  keypad,
//  magSwitch
//}

void setNodeType(String type) {
  //  NodeType nt;
  int val;
  NODE_TYPE = type;
  if (type == "relay") {
    val = TYPE_RELAY;
  };
  if (type == "button") {
    val = TYPE_BUTTON;
  };
  if (type == "keypad") {
    val = TYPE_KEYPAD;
  };
  if (type == "magSwitch") {
    val = TYPE_MAGSWITCH;
  };
  Serial.print("====== Setting " + type + " type =====");
  Serial.println(val);
  EEPROM.writeInt(0, val);
  EEPROM.commit();
}

String getNodeType() {
//  if (!EEPROM.begin(EEPROM.length())) {
//    Serial.println("Failed to initialise EEPROM");
//    Serial.println("Restarting...");
//    delay(1000);
//    ESP.restart();
//  }
  int type = (byte(EEPROM.readInt(0)));
  //  NodeType nt;
  //  nt = type;
  String nt;
  if (type == TYPE_RELAY) {
    nt = "relay";
  };
  if (type == TYPE_BUTTON) {
    nt = "button";
  };
  if (type == TYPE_KEYPAD) {
    nt = "keypad";
  };
  if (type == TYPE_MAGSWITCH) {
    nt = "magSwitch";
  };
  return nt;
}
