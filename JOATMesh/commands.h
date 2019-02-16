/**
 * Command types
 * bridgeId       = sets the bridge id for communicating out of the mesh
 * functionChange = change node type i.e. useRelay
 * admin          = specific commands i.e. getMeshNodes
 * setId          = change the device idChange
 * setName        = Unsure, probalbly just going to use ID for now
 * branchAddress  = set the address of the branch server
 * mqttAddress    = Set the broker address (usually same as branch address)
 * mqttMode       = enable or disbale mqtt
 *
 * EXAMPLE COMMAND: 
 *  {"command" : {
      "type" : "branchAddress",
      "message" : {
        "address" : [192,168,0,50],
        "port" : 1883
      }
    },
      "toId" : "10"}
 * 
 * 
 */

// Prototypes
String cmd_create_bridgeId(uint32_t nodeId);

void cmd_query_nodeList();
void cmd_query_subConnections();

void cmd_bridgeId(JsonObject &cmd);
void cmd_functionChange(JsonObject &cmd);
void cmd_admin(JsonObject &cmd);
void cmd_setId(JsonObject &cmd);
void cmd_setName(JsonObject &cmd);
void cmd_branchAddress(JsonObject &cmd);
void cmd_mqttMode(JsonObject &cmd);
bool validateFunctionChange(String msg);

//==============================//
//=====Parse Command ==========//
//============================//

/**
 * Parses the command type, assumes the command is for this device/node
 */
void cmd_parseCommand(JsonObject &root)
{
  JsonObject &command = root["command"];
  if (command["type"] == "bridgeId")
  {
    cmd_bridgeId(command);
  }

  if (command["type"] == "functionChange")
  {
    cmd_functionChange(command);
  }
  if (command["type"] == "admin")
  {
    cmd_admin(command);
  }
  if (command["type"] == "setId")
  {
    cmd_setId(command);
  }
  if (command["type"] == "setName")
  {
    cmd_setName(command);
  }
  if (command["type"] == "branchAddress")
  {
    cmd_branchAddress(command);
  }
  if (command["type"] == "mqttMode")
  {
    cmd_mqttMode(command);
  }
}

// Parse Broadcast commands
void cmd_broadcast(JsonObject &root)
{
  JsonObject &command = root["command"];
  if (command["type"] == "bridgeId")
  {
    cmd_bridgeId(command);
  }
}

// Parse bridge queries
void cmd_bridge_query(JsonObject &root)
{
  JsonObject &query = root["query"];
  if (query["type"] == "nodes")
  {
    cmd_query_nodeList();
  }
  if (query["type"] == "subconnections")
  {
    cmd_query_subConnections();
  }
}

// =============================================
// =========== Create queries to send =========
// =============================================
void cmd_query_subConnections()
{
  String msg = mesh.subConnectionJson();
  Serial.println(msg);
  sendMqttPacket(msg);
}

void cmd_query_nodeList()
{
  SimpleList<uint32_t> nodes;
  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end())
  {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
}

// =============================================
// =========== Create Commands to send =========
// =============================================

String cmd_create_bridgeId(uint32_t nodeId, bool broadcast)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &command = root.createNestedObject("command");

  if (broadcast)
  {
    root["toId"] = "broadcast"; //broadcast
  }
  else
  {
    root["toId"] = nodeId; // send to this id
  }

  command["type"] = "bridgeId";          // command type
  command["message"] = mesh.getNodeId(); // set this node as the id
  String msg;
  root.printTo(msg);
  return msg;
}

// =============================================
// =========== Execute commands ================
// =============================================
//TODO: do error checking!!!

void cmd_bridgeId(JsonObject &cmd)
{
  BRIDGE_ID = cmd["message"];
  Serial.print("===== Bridge ID set : ");
  Serial.println(BRIDGE_ID);
}

/**
 * Change the type of node this is
 */
void cmd_functionChange(JsonObject &cmd)
{
  String msg = cmd["message"];
  if (validateFunctionChange(msg))
  {
    setNodeType(msg);
    ESP.restart();
  }
}

void cmd_admin(JsonObject &cmd)
{
  String msg = cmd["message"];
  if (msg == "getMeshNodes")
  {
    printNodeList();
    // send mesh nodes back in a packet here??!?!
  }
}

void cmd_setId(JsonObject &cmd)
{
  String msg = cmd["message"];
  setNodeId(msg);
  ESP.restart();
}

void cmd_setName(JsonObject &cmd)
{
  //nothing yet
}

void cmd_branchAddress(JsonObject &cmd)
{
  JsonArray &command_message_address = cmd["message"]["address"];
  int cmd_addr[4];
  cmd_addr[0] = command_message_address[0]; // 192
  cmd_addr[1] = command_message_address[1]; // 168
  cmd_addr[2] = command_message_address[2]; // 0
  cmd_addr[3] = command_message_address[3]; // 50
  setBrokerAddress(cmd_addr);
  ESP.restart();
}

void cmd_mqttMode(JsonObject &cmd)
{
}

/**
 * Node type changes validation
 */
bool validateFunctionChange(String msg)
{
  if (
      msg == "bridge" ||
      msg == "button" ||
      msg == "keypad" ||
      msg == "relay" ||
      msg == "reedSwitch" ||
      msg == "magSwitch" ||
      msg == "rfid" ||
      msg == "button")
  {
    return true;
  }
  else
  {
    return false;
  }
}