#include "EEPROM.h"

int addr = 0;
#define EEPROM_SIZE 64

EEPROMClass  EENODEID("eeprom0", 0x1000);
EEPROMClass  EENODETYPE("eeprom1", 0x500);

#define TYPE_BRIDGE 0
#define TYPE_RELAY 1
#define TYPE_BUTTON 2
#define TYPE_KEYPAD 3
#define TYPE_MAGSWITCH 4
//enum NodeType {
//  relay,
//  button,
//  keypad,
//  magSwitch
//}
void setNodeId(String val){
  Serial.print("====== Setting " + val + " Node ID =====");
  Serial.println(val);
  EENODEID.begin(EENODEID.length());
  EENODEID.writeString(0, val);
  EENODEID.commit();
}

String getNodeId() {
  EENODEID.begin(EENODEID.length());
  String type = (EENODEID.readString(0));  
  return type;
}

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
  if (type == "bridge") {
    val = TYPE_BRIDGE;
  };
  Serial.print("====== Setting " + type + " type =====");
  Serial.println(val);
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.writeInt(0, val);
  EEPROM.commit();
}

String getNodeType() {
  EENODETYPE.begin(EENODETYPE.length());
  int type = (byte(EENODETYPE.readInt(0)));
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
  if (type == TYPE_BRIDGE) {
    nt = "bridge";
  };
  return nt;
}
