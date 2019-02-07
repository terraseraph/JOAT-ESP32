//************************************************************
// Switch between being a bridge devie or just a mesh node
// Example commands to send via mesh or serial.
// {"command" : "useMagSwitch", "toId" : "10"} -- tells device 10 to use mag switch
// {"command" : "getMeshNodes", "toId":"1"} -- Gets all mesh nodes and prints fre memory
// {"command" : "setId", "toId" : "10", "nodeId" : "11"} (toId also hardware id) -- sets the easy id
// {"toId": "11", "wait": "0", "event":"noneE", "eventType":"noneET", "action":"start", "actionType":"relay", "data":"1"} -- example action
// {"toId":"3213429781","fromId":"10","event":"code","eventType":"keypad","action":"noneA", "actionType" : "noneAT", "data" : "333333"} -- example event
//
// https://randomnerdtutorials.com/esp32-pinout-reference-gpios/ <- for the pins
//************************************************************

#include <Arduino.h>

#include <painlessMesh.h>


#include "globals.h"
#include "joatEEPROM.h"
#include "commands.h"
#include "stateEventAction.h"
#include "webServer.h"
#include "mqtt.h"
#include "joatKeypad.h"
#include "relay.h"
#include "button.h"
#include "magSwitch.h"
#include "rfid.h"

String inputString = ""; //for serial input

//==============================//
//=====Heartbeat Init =========//
//============================//
Scheduler scheduler;
Task taskHeartbeat(TASK_SECOND * 30, TASK_FOREVER, []() {
  String hwId = String(mesh.getNodeId());
  String mem = String(ESP.getFreeHeap());
  String msg = "{\"heartbeat\":{\"hardwareId\":" + hwId + ",\"id\":\"" + MY_ID + "\",\"type\":\"" + NODE_TYPE + "\",\"memory\":\"" + mem + "\"}}";
  Serial.println("====Sending heartbeat to: " + String(BRIDGE_ID) + "=====");
  Serial.println(msg);
  mesh.sendSingle(BRIDGE_ID, msg);
  //  mesh.sendBroadcast(msg);
});

//==============================//
//===== Node list print =======//
//============================//
Scheduler nodeListScheduler;
Task printNodeListTask(TASK_SECOND * 60, TASK_FOREVER, &printNodeList);

void setup()
{
  Serial.begin(115200);
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&mesh_receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);

  if (getMyId() != "0")
  {
    MY_ID = getMyId();
  }
  else
  {
    setNodeId(MY_ID);
  }
  Serial.println("======= NodeId " + MY_ID + "=========");
  startupInitType();
  if (NODE_TYPE != "bridge")
  {
    scheduler.addTask(taskHeartbeat);
    taskHeartbeat.enable();
  }
  mesh.setContainsRoot(true);
}

void loop()
{
  mesh.update();
  serialEvent();
  processEventLoop();
}

//==============================//
//=====Startup Init ===========//
//============================//
void startupInitType()
{
  NODE_TYPE = getNodeType();
  if (NODE_TYPE == "relay")
  {
    relay_init();
  };
  if (NODE_TYPE == "button")
  {
    button_init();
  };
  if (NODE_TYPE == "keypad")
  {
    keypad_init();
  };
  if (NODE_TYPE == "magSwitch")
  {
    magSwitch_init();
  };
  if (NODE_TYPE == "rfid")
  {
    rfid_init();
  };
  if (NODE_TYPE == "bridge")
  {
    bridge_init();
    webServer_init();
    mqtt_init();
    nodeListScheduler.init();
    nodeListScheduler.addTask(printNodeListTask);
    printNodeListTask.enable();
  };
  Serial.println("======  Using " + NODE_TYPE + " =====");
}

//==============================//
//=====Event check loop =======//
//============================//
void processEventLoop()
{
  if (NODE_TYPE == "button")
  {
    processButtonEvent();
  };
  if (NODE_TYPE == "keypad")
  {
    ProcessKeyPad();
  };
  if (NODE_TYPE == "magSwitch")
  {
    processMagSwitchEvent();
  };
  if (NODE_TYPE == "relay")
  {
    relayScheduler.execute(); //for relay delays
  };
  if (NODE_TYPE == "rfid")
  {
    rdidScheduler.execute(); //for relay delays
    processRfid();
  };
  if (NODE_TYPE != "bridge") //run heartbeat if not the bridge
  {
    scheduler.execute();
  }
  if (NODE_TYPE == "bridge") //if it is the bridge
  {
    nodeListScheduler.execute();
    processMqtt();
  }
}

//==============================//
//=====Bridge Init ============//
//============================//
void bridge_init()
{
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  mesh.setRoot(true);
}

//==========================================//
//===== List of connected nodes ===========//
//========================================//
// DynamicJsonBuffer jsonNodeListBuffer;
// JsonObject &nodeList = jsonNodeListBuffer.createObject();

void addNodeToList(uint32_t nodeId, String myId, String nodeType, String memory)
{
  String nodeMeshId = String(nodeId); //convert node name to string
  if (!nodeList.containsKey(nodeMeshId))
  {
    JsonObject &nodeObj = nodeList.createNestedObject(nodeMeshId);
    nodeObj["id"] = myId;
    nodeObj["type"] = nodeType;
    nodeObj["memory"] = memory;
    nodeObj["lastAlive"] = 0;
    nodeObj["lastAliveMillis"] = millis();

  }
  else
  {
    long t = nodeList[nodeMeshId]["lastAliveMillis"];


    nodeList[nodeMeshId]["id"] = myId;
    nodeList[nodeMeshId]["type"] = nodeType;
    nodeList[nodeMeshId]["memory"] = memory;
    nodeList[nodeMeshId]["lastAlive"] = abs(millis() - t);
    nodeList[nodeMeshId]["lastAliveMillis"] = millis();
  }
}

/**
 * Print node list
 */
void printNodeList()
{
  Serial.println("============= Nodes ==========");

  Serial.print("{\"IpAddress\":\"");
  Serial.print(mesh.getStationIP());
  Serial.println("\"}");

  String list;
  nodeList.printTo(list);
  String msg = "{\"nodes\":[ " + list + "]}";
  Serial.println(msg);
  if(MQTT_ENABLED && NODE_TYPE == "bridge"){
    sendMqttPacket(msg);
  }
  // Also add in a time since last heard from device


  String mem = String(ESP.getFreeHeap());
  Serial.println("{\"memory\":" + mem + "}");
}

void removeNodeFromList(String nodeName)
{
  nodeList.remove(nodeName);
}

//==============================//
//=====On connection callback===//
//============================//
void newConnectionCallback(uint32_t nodeId)
{
  if (NODE_TYPE == "bridge")
  {
    // DynamicJsonBuffer jsonBuffer;
    // JsonObject &root = jsonBuffer.createObject();
    // root["command"] = "setBridgeId";
    // root["bridgeId"] = mesh.getNodeId();
    // root["toId"] = nodeId;
    // String msg;
    // root.printTo(msg);
    String msg = cmd_create_bridgeId(nodeId);
    Serial.print("==== Sending bridge setting id to: ");
    Serial.println(nodeId);
    mesh.sendSingle(nodeId, msg);
  }
}

//==============================//
//=====Parse serial=======//
//============================//
void serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n')
    {
      preparePacketForMesh(mesh.getNodeId(), inputString);
      inputString = "";
      Serial.println("{\"ready\":\"true\"}"); //tells the server we are ready for another message
    }
  }
}

//==============================//
//=====Prepare packet=======//
//============================//
//Used for direct input from serial (later for http messages)
void preparePacketForMesh(uint32_t from, String &msg)
{
  // Saving logServer
  Serial.println("Perparing packet");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(msg);
  String buffer;
  root.printTo(buffer);
  //  uint32_t target = root["toId"];
  if (root.success())
  {

    // if is a command packet
    if (root.containsKey("command") && root.containsKey("toId"))
    {
      if (root["toId"] == MY_ID || root["toId"] == String(mesh.getNodeId()))
      {
        // parseCommand(root);
        cmd_parseCommand(root);
      }
      else //broadcast command to the mesh
      {
        mesh.sendBroadcast(buffer);
      }
    }

    // If is event action packet
    if (root.containsKey("state"))
    {
      if (root["toId"] == MY_ID || root["toId"] == String(mesh.getNodeId()))
      {
        // parseEventActionPacket(root);
        state_parsePacket(root);
      }
      else
      {
        // forwardEventActionPacket(root);
        state_forwardPacketToMesh(root);
      }
    }
    else //just send it anyway......
    {
      mesh.sendBroadcast(buffer);
    }
  }
  else //not a json message
  {
    Serial.printf("JSON parsing failed");
    mesh.sendBroadcast(msg, false);
  }
  Serial.printf("Sending message from %u msg=%s\n", from, msg.c_str());
  //mesh.sendSingle(target, msg);  //TODO: setup the send single later
}

//==============================//
//=====Forward event action=====//
//============================//

// pretty sure this should just skip all those lines and broadcast anyway
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
  mesh.sendBroadcast(buffer); //TODO: change to sendSingle()
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
void mesh_receivedCallback(uint32_t from, String &msg)
{
  parseReceivedPacket(from, msg);
}

//==============================//
//=====Parse message ==========//
//============================//
void parseReceivedPacket(uint32_t from, String msg)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(msg);
  if (root.success())
  {
    if (root.containsKey("command") && (root["toId"] == MY_ID || root["toId"] == String(mesh.getNodeId())))
    {
      // parseCommand(root);
      cmd_parseCommand(root);
    }
    if (root.containsKey("heartbeat"))
    {
      addNodeToList(from, root["heartbeat"]["id"], root["heartbeat"]["type"], root["heartbeat"]["memory"]);
    }
    if(root.containsKey("state")){
      state_parsePacket(root);
      if(NODE_TYPE == "bridge"){
        sendMqttPacket(msg);
      }
    }
    else
    {
      // parseEventActionPacket(root);
    }
  }
}


//TODO: Parse the commands IAW the model written on notion

//==============================//
//=====Parse Command ==========//
//============================//
void parseCommand(JsonObject &root)
{
  if (root["command"] == "setBridgeId")
  {
    BRIDGE_ID = root["bridgeId"];
    Serial.print("===== Bridge ID set : ");
    Serial.println(BRIDGE_ID);
    return;
  }
  if (root["command"] == "setId")
  {
    setNodeId(root["nodeId"]);
    ESP.restart();
  }
  if (root["command"] == "useBridge")
  {
    setNodeType("bridge");
    ESP.restart();
  }
  if (root["command"] == "useButton")
  {
    setNodeType("button");
    ESP.restart();
  }
  if (root["command"] == "useRelay")
  {
    setNodeType("relay");
    ESP.restart();
  }
  if (root["command"] == "useKeypad")
  {
    setNodeType("keypad");
    ESP.restart();
  }
  if (root["command"] == "useMagSwitch")
  {
    setNodeType("magSwitch");
    ESP.restart();
  }
  if (root["command"] == "useRfid")
  {
    setNodeType("rfid");
    ESP.restart();
  }
  if (root["command"] == "getMeshNodes")
  {
    printNodeList();
  }
}

//=========================================//
//==== Parse Event action Packet =========//
//=======================================//
void parseEventActionPacket(JsonObject &root)
{
  String toId = root["toId"];
  const char *wait = root["wait"];
  const char *event = root["event"];
  const char *eventType = root["eventType"];
  String action = root["action"];
  String actionType = root["actionType"];

  String msg;
  root.printTo(msg);
  //Switch data type depending on action to be completed
  if (root["toId"] == MY_ID || root["toId"] == String(mesh.getNodeId()))
  {
    if (actionType == "relay")
    {
      Serial.println("Packet for me");
      int data = root["data"];
      processRelayAction(action, data);
    }
    else //goes to serial
    {
      Serial.println(msg);
    }
  }
  else
  {
    Serial.printf("Not for me");
    return;
  }
}

//=========================================//
//==== Create JSON Packet ================//
//=======================================//
void createJsonPacket(String fromId, String event, String eventType, String action, String actionType, String data)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &object = jsonBuffer.createObject();
  object["toId"] = String(BRIDGE_ID);
  object["fromId"] = fromId;
  object["event"] = event;
  object["eventType"] = eventType;
  object["action"] = action;
  object["actionType"] = actionType;
  object["data"] = data;
  String buffer;
  object.printTo(buffer);
  Serial.println(buffer);
  // mesh.sendBroadcast(buffer);
  mesh.sendSingle(BRIDGE_ID, buffer);
}
