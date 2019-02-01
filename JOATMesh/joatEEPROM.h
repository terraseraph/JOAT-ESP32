#include "EEPROM.h"

int addr = 0;
#define EEPROM_SIZE 64

EEPROMClass EENODEID("eeprom0", 0x1000);
EEPROMClass EENODETYPE("eeprom1", 0x500);

#define TYPE_BRIDGE 0
#define TYPE_RELAY 1
#define TYPE_BUTTON 2
#define TYPE_KEYPAD 3
#define TYPE_MAGSWITCH 4
#define TYPE_RFID 5
#define TYPE_UNSET 255

#define NODE_TYPE_START 0
#define MQTT_ADDRESS_START 4
#define NODE_ID_START 20

/*
  Generated partition that would work perfectly with this example
  #Name,   Type, SubType, Offset,   Size,    Flags
  nvs,     data, nvs,     0x9000,   0x5000,
  otadata, data, ota,     0xe000,   0x2000,
  app0,    app,  ota_0,   0x10000,  0x140000,
  app1,    app,  ota_1,   0x150000, 0x140000,
  eeprom0, data, 0x99,    0x290000, 0x1000,
  eeprom1, data, 0x9a,    0x291000, 0x500,
  eeprom2, data, 0x9b,    0x292000, 0x100,
  spiffs,  data, spiffs,  0x293000, 0x16d000,

  Created for arduino-esp32 on 25 Dec, 2017
  by Elochukwu Ifediora (fedy0)

  // Instantiate eeprom objects with parameter/argument names and size same as in the partition table
  EEPROMClass  NAMES("eeprom0", 0x1000);
  NAMES.begin(NAMES.length();
  NAMES.put(0, name);
*/

void setNodeId(String val)
{
  Serial.print("====== Setting " + val + " Node ID =====");
  Serial.println(val);
  // int x = atoi(val);
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.writeString(NODE_ID_START, val);
  EEPROM.commit();
}

String getMyId()
{
  EEPROM.begin(EEPROM_SIZE);
  String type = (EEPROM.readString(NODE_ID_START)); // the offset from the other addresses
  return type;
}

void setNodeType(String type)
{
  //  NodeType nt;
  int val;
  NODE_TYPE = type;
  if (type == "relay")
  {
    val = TYPE_RELAY;
  };
  if (type == "button")
  {
    val = TYPE_BUTTON;
  };
  if (type == "keypad")
  {
    val = TYPE_KEYPAD;
  };
  if (type == "magSwitch")
  {
    val = TYPE_MAGSWITCH;
  };
  if (type == "rfid")
  {
    val = TYPE_RFID;
  };
  if (type == "bridge")
  {
    val = TYPE_BRIDGE;
  };
  Serial.print("====== Setting " + type + " type =====");
  Serial.println(val);
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.writeInt(NODE_TYPE_START, val);
  EEPROM.commit();
}

String getNodeType()
{
  EEPROM.begin(EEPROM_SIZE);
  int type = (byte(EEPROM.readInt(0)));
  //  NodeType nt;
  //  nt = type;
  String nt;
  if (type == TYPE_UNSET)
  {
    nt = "unset";
  };
  if (type == TYPE_RELAY)
  {
    nt = "relay";
  };
  if (type == TYPE_BUTTON)
  {
    nt = "button";
  };
  if (type == TYPE_KEYPAD)
  {
    nt = "keypad";
  };
  if (type == TYPE_MAGSWITCH)
  {
    nt = "magSwitch";
  };
  if (type == TYPE_RFID)
  {
    nt = "rfid";
  };
  if (type == TYPE_BRIDGE)
  {
    nt = "bridge";
  };
  return nt;
}

// MQTT
void setBrokerAddress(int ipAddress[])
{
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.writeInt(MQTT_ADDRESS_START + 0, ipAddress[0]);
  EEPROM.writeInt(MQTT_ADDRESS_START + 4, ipAddress[1]);
  EEPROM.writeInt(MQTT_ADDRESS_START + 8, ipAddress[2]);
  EEPROM.writeInt(MQTT_ADDRESS_START + 12, ipAddress[3]);
  EEPROM.commit();
}

void getMqttBrokerAddress()
{
  int addr[4];
  EEPROM.begin(EEPROM_SIZE);
  addr[0] = (EEPROM.readInt(MQTT_ADDRESS_START + 0));
  addr[1] = (EEPROM.readInt(MQTT_ADDRESS_START + 4));
  addr[2] = (EEPROM.readInt(MQTT_ADDRESS_START + 8));
  addr[3] = (EEPROM.readInt(MQTT_ADDRESS_START + 12));
  return addr;
}
