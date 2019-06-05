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
// #define ESP32
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <Arduino.h>

#include <painlessMesh.h>
#include <HardwareSerial.h>
#include "ota.h"
#include "libs/LinkedList.h"
#include "globals.h"
#include "joatEEPROM.h"
#include "commands.h"
#include "mp3Player.h"
#include "stateEventAction.h"
#include "webServer.h"
#include "mqtt.h"
#include "joatKeypad.h"
#include "relay.h"
#include "button.h"
#include "magSwitch.h"
#include "rfid.h"
//------ Custom ------//
#include "custom/phone.h"

String inputString = ""; //for serial input

//==============================//
//===== Scheduler Init ========//
//============================//

// Scheduler reconnectScheduler;
Task taskReconnect(TASK_SECOND * 180, TASK_FOREVER, &taskPrepareMeshReconnect);

// Scheduler heartbeatScheduler;
Task taskHeartbeat(TASK_SECOND * 30, TASK_FOREVER, &taskPrepareHeartbeat);

// Scheduler nodeListScheduler;
Task printNodeListTask(TASK_SECOND * 60, TASK_FOREVER, &printNodeList);

// Scheduler bridgeIdScheduler;
Task taskBridgeId(TASK_SECOND * 20, TASK_FOREVER, &taskBroadcastBridgeId);

//==============================//
//===== Node list print =======//
//============================//

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
    scheduler.addTask(taskReconnect);
    taskReconnect.enable();
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
  if (NODE_TYPE == "mp3")
  {
    MP3_init();
  };
  //======= Custom ====================//
  if (NODE_TYPE == "phone")
  {
    phone_init();
  };
  //==================================//
  if (NODE_TYPE == "bridge")
  {
    bridge_init();
    scheduler.addTask(printNodeListTask);
    scheduler.addTask(taskBridgeId);
    printNodeListTask.enable();
    taskBridgeId.enable();
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
  if (NODE_TYPE == "mp3")
  {
    processMp3Loop();
  };
  //======= Custom ====================//
  if (NODE_TYPE == "phone")
  {
    processPhoneLoop();
  };
  //==================================//
  if (NODE_TYPE != "bridge") //run heartbeat if not the bridge
  {
    scheduler.execute();
  }
  if (NODE_TYPE == "bridge") //if it is the bridge
  {
    scheduler.execute();
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
  if (MQTT_ENABLED)
  {
    mqtt_init();
  }
  if (HTTP_ENABLED)
  {
    webServer_init();
  }
}

//==========================================//
//===== List of connected nodes ===========//
//========================================//

class MeshNode
{
public:
  uint32_t hardwareId;
  String id;
  String nodeType;
  String memory;
};
NLinkedList<MeshNode *> meshNodeList = NLinkedList<MeshNode *>();

uint32_t getNodeHardwareId(String id)
{
  MeshNode *node;
  for (int i = 0; i < meshNodeList.size(); i++)
  {
    node = meshNodeList.get(i);
    if (node->id == id)
    {
      return node->hardwareId;
    }
  }
  return 0;
}

void addNodeToList(uint32_t nodeId, String myId, String nodeType, String memory)
{
  MeshNode *node;
  bool nodeExists = false;
  for (int i = 0; i < meshNodeList.size(); i++)
  {
    node = meshNodeList.get(i);
    if (node->hardwareId == nodeId)
    {
      nodeExists = true;

      node->id = myId;
      node->nodeType = nodeType;
      node->memory = memory;
    }
  }
  if (!nodeExists)
  {
    node = new MeshNode();
    node->hardwareId = nodeId;
    node->id = myId;
    node->nodeType = nodeType;
    node->memory = memory;
    meshNodeList.add(node);
  }

  String nodeMeshId = String(nodeId); //convert node name to string
  String msg = "{\"heartbeat\":{\"hardwareId\":" + nodeMeshId + ",\"id\":\"" + myId + "\",\"type\":\"" + nodeType + "\",\"memory\":\"" + memory + "\"}}";
  Serial.println(msg);
  if (MQTT_ENABLED)
  {
    sendMqttPacket(msg);
  }
}

/**
 * Print node list
 */
void printNodeList()
{
  String msgIp = mesh.getStationIP().toString();
  String mem = String(ESP.getFreeHeap());

  Serial.println("============= Nodes ==========");
  String msg = "{\"bridgeMemory\":" + mem + ",\"ipAddress\":\"" + msgIp + "\"}";
  Serial.println(msg);
  if (MQTT_ENABLED && NODE_TYPE == "bridge")
  {
    sendMqttPacket(msg);
  }
  return;
}

void removeNodeFromList(String nodeName)
{
  // nodeList.remove(nodeName);
}

//==============================//
//=====On connection callback===//
//============================//
void newConnectionCallback(uint32_t nodeId)
{
  if (NODE_TYPE == "bridge")
  {
    String msg = cmd_create_bridgeId(nodeId, false);
    Serial.print("==== Sending bridge setting id to: ");
    Serial.println(nodeId);
    Serial.println(msg);
    mesh.sendSingle(nodeId, msg);
  }
  return;
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
      Serial.println(readyMessage); //tells the server we are ready for another message
    }
  }
  return;
}

//==============================//
//=====Prepare packet=======//
//============================//
//Used for direct input from serial/http/mqtt
void preparePacketForMesh(uint32_t from, String &msg)
{
  // Saving logServer
  Serial.println("Perparing packet");
  DynamicJsonDocument jsonBuffer(sizeof(msg) + 100);
  deserializeJson(jsonBuffer, msg);
  JsonObject root = jsonBuffer.as<JsonObject>();
  String buffer;
  // root.printTo(buffer);
  serializeJson(root, buffer);
  if (!root.isNull())
  {

    if (root.containsKey("query"))
    {
      cmd_bridge_query(root);
    }
    // if is a command packet
    if (root.containsKey("command") && root.containsKey("toId"))
    {
      if (root["toId"] == MY_ID || root["toId"] == String(mesh.getNodeId()))
      {
        cmd_parseCommand(root);
      }
      else //broadcast command to the mesh
      {
        state_forwardPacketToMesh(buffer, root["toId"]);
        // mesh.sendBroadcast(buffer);
      }
    }

    // If is event action packet
    if (root.containsKey("state"))
    {
      if (root["toId"] == MY_ID || root["toId"] == String(mesh.getNodeId()))
      {
        state_parsePacket(root);
      }
      else
      {
        state_forwardPacketToMesh(buffer, root["toId"]);
      }
    }
    else //just send it anyway......
    {
      state_forwardPacketToMesh(buffer, root["toId"]);
      // mesh.sendBroadcast(buffer);
    }
  }
  else //not a json message
  {
    Serial.println("JSON parsing failed");
    mesh.sendBroadcast(msg, false);
  }
  Serial.print("Sending message from : ");
  Serial.print(from);
  Serial.print(" message: ");
  Serial.print(msg);
  Serial.flush();
  return;
}

//====================================//
//===== Scheduled Task functions ====//
//==================================//
void taskPrepareHeartbeat()
{
  DynamicJsonDocument root(1024);
  // JsonObject &root = jsonBuffer.createObject();
  JsonObject heartbeat = root.createNestedObject("heartbeat");

  String hwId = String(mesh.getNodeId());
  String mem = String(ESP.getFreeHeap());
  String msgBuffer;
  heartbeat["hardwareId"] = hwId;
  heartbeat["id"] = MY_ID;
  heartbeat["type"] = NODE_TYPE;
  heartbeat["memory"] = mem;

  // root.printTo(msgBuffer);
  serializeJson(root, msgBuffer);
  Serial.println("====Sending heartbeat to: " + String(BRIDGE_ID) + "=====");
  Serial.println(msgBuffer);
  mesh.sendSingle(BRIDGE_ID, msgBuffer);
}

void taskBroadcastBridgeId()
{
  String msg = cmd_create_bridgeId(0, true);
  Serial.println("==== Broadcasting bridge id ===== ");
  mesh.sendBroadcast(msg);
}

void taskPrepareMeshReconnect()
{
  if (BRIDGE_ID == 0)
  {
    Serial.println("=== Bridge unset, restarting mesh connection ===");
    mesh.stop();
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  }
}

//==============================//
//=====Received callback=======//
//============================//
void mesh_receivedCallback(uint32_t from, String &msg)
{
  parseReceivedPacket(from, msg);
  return;
}

//==============================//
//=====Parse message ==========//
//============================//
void parseReceivedPacket(uint32_t from, String msg)
{
  Serial.println("==== parsing Received packet =======");

  DynamicJsonDocument jsonBuffer(sizeof(msg) + 100);
  // JsonObject &root = jsonBuffer.parseObject(msg);
  deserializeJson(jsonBuffer, msg);
  JsonObject root = jsonBuffer.as<JsonObject>();
  String serialBuffer;
  // root.printTo(serialBuffer);
  serializeJson(root, serialBuffer);
  Serial.println(serialBuffer);
  if (!root.isNull())
  {

    if (root.containsKey("command") && (root["toId"] == MY_ID || root["toId"] == String(mesh.getNodeId())))
    {
      cmd_parseCommand(root);
    }

    if (root.containsKey("command") && root["toId"] == "broadcast")
    {
      cmd_broadcast(root);
    }

    if (root.containsKey("heartbeat"))
    {
      addNodeToList(from, root["heartbeat"]["id"], root["heartbeat"]["type"], root["heartbeat"]["memory"]);
    }

    // Event Action
    if (root.containsKey("state"))
    {
      state_parsePacket(root);
      if (NODE_TYPE == "bridge")
      {
        sendMqttPacket(msg);
        //TODO: maybe serial here too
      }
    }

    else
    {
    }
  }
  return;
}
