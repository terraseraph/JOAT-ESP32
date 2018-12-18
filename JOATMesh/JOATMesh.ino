//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a another network and relay messages from a MQTT broker to the nodes of the mesh network.
// To send a message to a mesh node, you can publish it to "painlessMesh/to/12345678" where 12345678 equals the nodeId.
// To broadcast a message to all nodes in the mesh you can publish it to "painlessMesh/to/broadcast".
// When you publish "getNodes" to "painlessMesh/to/gateway" you receive the mesh topology as JSON
// Every message from the mesh which is send to the gateway node will be published to "painlessMesh/from/12345678" where 12345678
// is the nodeId from which the packet was send.
//************************************************************

#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "globals.h"
#include "joatEEPROM.h"
#include "joatKeypad.h"
#include "relay.h"
#include "button.h"
#include "magSwitch.h"

#define   MESH_PREFIX     "seraphimMesh"
#define   MESH_PASSWORD   "somepassword"
#define   MESH_PORT       5555

#define   STATION_SSID     "NFC"
#define   STATION_PASSWORD "somepassword"

#define HOSTNAME "MQTT_Bridge"
#define BRIDGE // to use when files are combined
uint32_t BRIDGE_ID;

//Create list of all connected nodes and names
DynamicJsonBuffer jsonNodeListBuffer;
JsonObject& nodeList = jsonNodeListBuffer.createObject();

void addNodeToList(uint32_t nodeId, String nodeName) {
  nodeList[nodeName] = nodeId;
}

void removeNodeFromList(String nodeName) {
  nodeList.remove(nodeName);
}

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);

//temp for a better name
size_t logServerId = 0;

IPAddress getlocalIP();

IPAddress myIP(0, 0, 0, 0);
IPAddress mqttBroker(192, 168, 1, 1);

painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);

String inputString = "";

void setup() {
  Serial.begin(115200);
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);

  startupInitType();
}

void loop() {
  mesh.update();
  mqttClient.loop();
  serialEvent();
  processEventLoop();
  relayScheduler.execute(); //put elsewhere
  if (myIP != getlocalIP()) {
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("painlessMeshClient")) {
      mqttClient.publish("painlessMesh/from/gateway", "Ready!");
      mqttClient.subscribe("painlessMesh/to/#");
    }
  }
}

void processEventLoop() {
  if (NODE_TYPE == "button") {
    processButtonEvent();
  };
  if (NODE_TYPE == "keypad") {
    ProcessKeyPad();
  };
  if (NODE_TYPE == "magSwitch") {
    processMagSwitchEvent();
  };
}

void startupInitType() {
  NODE_TYPE = getNodeType();
  if (NODE_TYPE == "relay") {
    relay_init();
  };
  if (NODE_TYPE == "button") {
    button_init();
  };
  if (NODE_TYPE == "keypad") {
    keypad_init();
  };
  if (NODE_TYPE == "magSwitch") {
    magSwitch_init();
  };
  if (NODE_TYPE == "bridge") {
    bridge_init();
  };
  Serial.println("======  Using " + NODE_TYPE + " =====");
}

void bridge_init() {
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
}

void newConnectionCallback( uint32_t nodeId ) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["command"] = "setBridgeId";
  root["bridgeId"] = mesh.getNodeId();
  String msg;
  root.printTo(msg);
  mesh.sendSingle(nodeId, msg);
}


void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    Serial.print(inChar);
    inputString += inChar;
    if (inChar == '\n') {
      Serial.println(inputString);
      preparePacketForMesh(mesh.getNodeId(), inputString);
      inputString = "";
    }
  }
}

//prepare packet from serial to send
void preparePacketForMesh( uint32_t from, String &msg ) {
  // Saving logServer
  Serial.println("Perparing packet");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msg);
  //  uint32_t target = root["toId"];
  if (root.success()) {
    if (root.containsKey("command") && root.containsKey("toId")) {
      if (root["toId"] == MY_ID) {
        parseCommand(root);
      }
      else {
        String buffer;
        root.printTo(buffer);
        mesh.sendBroadcast(buffer);
      }
    }
    if (root.containsKey("toId")) {
      forwardEventActionPacket(root);
    }
    else {
      String buffer;
      root.printTo(buffer);
      mesh.sendBroadcast(buffer);
    }
  }
  else {
    Serial.printf("JSON parsing failed");
    mesh.sendBroadcast(msg, false);
  }
  Serial.printf("Sending message from %u msg=%s\n", from, msg.c_str());
  //mesh.sendSingle(target, msg);  //TODO: setup the send single later
}


void forwardEventActionPacket(JsonObject &root)
{
  /* This is an event or action to send to the network */
  const char *toId = root["toId"];
  const char *wait = root["wait"];
  const char *event = root["event"];
  const char *eventType = root["eventType"];
  const char *action = root["action"];
  const char *actionType = root["actionType"];
  const char *data = root["data"];
  String buffer;
  root.printTo(buffer);
  Serial.print(buffer);
  mesh.sendBroadcast(buffer);
#ifdef DEV_DEBUG
  /* for debugging messages */
  String msg = toId;
  msg += wait;
  msg += event;
  msg += eventType;
  msg += action;
  msg += actionType;
  msg += data;
  Serial.printf(msg);
#endif
}

//==============================//
//=====Received callback=======//
//============================//
void receivedCallback( const uint32_t &from, const String &msg ) {
  //  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str()); //Remove this later
  String topic = "painlessMesh/from/" + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
  Serial.println(msg.c_str());
  parseReceivedPacket(msg);
}

//==============================//
//=====Parse message ==========//
//============================//
void parseReceivedPacket(String msg) {
  char cMsg[msg.length() + 1];
  msg.toCharArray(cMsg, sizeof(cMsg));
  Serial.println(cMsg);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(cMsg);
  if (root.success()) {
    if (root.containsKey("command") && root["toId"] == MY_ID) {
      parseCommand(root);
    }
    else {
      parseEventActionPacket(root);
    }
  }
}

//==============================//
//=====Parse Command ==========//
//============================//
void parseCommand(JsonObject &root) {
  if (root["command"] == "setBridgeId") {
    uint32_t id = root["bridgeId"];
    BRIDGE_ID = id;
    return;
  }
  if (root["command"] == "useBridge") {
    //    button_init();
    setNodeType("bridge");
    ESP.restart();
  }
  if (root["command"] == "useButton") {
    //    button_init();
    setNodeType("button");
    ESP.restart();
  }
  if (root["command"] == "useRelay") {
    //    relay_init();
    setNodeType("relay");
    ESP.restart();
  }
  if (root["command"] == "useKeypad") {
    //    keypad_init();
    setNodeType("keypad");
    ESP.restart();
  }
  if (root["command"] == "useMagSwitch") {
    //    magSwitch_init();
    setNodeType("magSwitch");
    ESP.restart();
  }
}


//=========================================//
//==== Parse Event action Packet =========//
//=======================================//
void parseEventActionPacket(JsonObject &root) {
  String toId = root["toId"];
  const char *wait = root["wait"];
  const char *event = root["event"];
  const char *eventType = root["eventType"];
  String action = root["action"];
  String actionType = root["actionType"];

  //Switch data type depending on action to be completed
  if (toId != MY_ID) {
    Serial.printf("Not for me");
    return;
  }

  if (actionType == "relay") {
    Serial.println("Packet for me");
    int data = root["data"];
    processRelayAction(action, data);
  }
}

//=========================================//
//==== Create JSON Packet ================//
//=======================================//
void createJsonPacket(String fromId, String event, String eventType, String action, String actionType, String data) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  object["toId"] = "master";
  object["fromId"] = fromId;
  object["event"] = event;
  object["eventType"] = eventType;
  object["action"] = action;
  object["actionType"] = actionType;
  object["data"] = data;
  String buffer;
  object.printTo(buffer);
  Serial.println(buffer);
  mesh.sendBroadcast(buffer);
}



void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length + 1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length + 1);
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  if (targetStr == "gateway")
  {
    if (msg == "getNodes")
    {
      mqttClient.publish("painlessMesh/from/gateway", mesh.subConnectionJson().c_str());
    }
  }
  else if (targetStr == "broadcast")
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if (mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
      mqttClient.publish("painlessMesh/from/gateway", "Client not connected!");
    }
  }
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
