//************************************************************
// https://randomnerdtutorials.com/esp32-pinout-reference-gpios/ <- for the pins
//
//************************************************************
#include "globals.h"
#include "joatEEPROM.h"
#include "namedMesh.h"
#include "joatKeypad.h"
#include "relay.h"
#include "button.h"
#include "magSwitch.h"


#define   MESH_SSID       "seraphimMesh"
#define   MESH_PASSWORD   "somepassword"
#define   MESH_PORT       5555


Scheduler     userScheduler; // to control your personal task
uint32_t BRIDGE_ID;
namedMesh  mesh;

String inputString = "";


//send heartbeat every 30 seconds
Task taskSendMessage( TASK_SECOND * 30, TASK_FOREVER, []() {
  String msg = String("Heartbeat ") + MY_ID + String(" Alive, Type: " + NODE_TYPE);
  mesh.sendSingle(BRIDGE_ID, msg);
  mesh.sendBroadcast(msg);
}); // start with a one second interval




void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name!

  mesh.onReceive([](uint32_t from, String & msg) {
    Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  });

  mesh.onReceive([](String & from, String & msg) {
    parseReceivedPacket(msg);
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  //Init node type

  //  relay_init();
  //  button_init();
  //  keypad_init();
  //  magSwitch_init();
  startupInitType();
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
  Serial.println("======  Using " + NODE_TYPE + " =====");
}

void parseReceivedPacket(String msg) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msg);
  if (root.success()) {
    if (root.containsKey("command") && root["toId"] == MY_ID) {
      parseCommand(root);
    }
    else {
      parseEventActionPacket(root);
    }
  }
}

//Command to change the node type
void parseCommand(JsonObject &root) {
  if (root["command"] == "setBridgeId") {
    uint32_t id = root["bridgeId"];
    BRIDGE_ID = id;
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


void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  relayScheduler.execute(); //FOR RELAY TOGGLE
  mesh.update();

  //Events
  serialEvent();
  processEventLoop();
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

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      preparePacketForMesh(mesh.getNodeId(), inputString);
      inputString = "";
    }
  }
}

//prepare packet from serial to send
void preparePacketForMesh( uint32_t from, String &msg ) {
  // Saving logServer
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msg);
  if (root.success()) {
    if (root.containsKey("command") && root.containsKey("toId")) {
      if(root["toId"] == MY_ID){
        parseCommand(root);  
      }
    }
    else{
      mesh.sendBroadcast(msg, false);
    }
  } 
}

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

void createJsonPacket(String fromId, String event, String eventType, String action, String actionType, String data) {
  StaticJsonBuffer<200> jsonBuffer;
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
  mesh.sendBroadcast(buffer);
}
