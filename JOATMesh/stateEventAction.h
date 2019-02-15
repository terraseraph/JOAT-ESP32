//prototypes
String state_createAndSendPacket(String fromId, String type, String event, String eventType, String action, String actionType, String data);
void state_parsePacket(JsonObject &root);
void state_forwardPacketToMesh(JsonObject &root);
// ===================================================
// =========== Create State Packets to send ==========
// ===================================================

/**
 * Creates an event action packet to jsonBuffer
 * type = event/action
 * returns a String to send over the mesh
 */
String state_createAndSendPacket(String fromId, String type, String event, String eventType, String action, String actionType, String data)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &state = root.createNestedObject("state");
  JsonObject &state_message = state.createNestedObject("message");

  // ensure action and events are correct for opposing states
  if (type == "event")
  {
    action = "noneA";
    actionType = "noneAT";
  }
  else if (type == "action")
  {
    event = "noneE";
    eventType = "noneET";
  }

  root["toId"] = String(BRIDGE_ID);
  state["type"] = type;
  state_message["fromId"] = fromId;
  state_message["event"] = event;
  state_message["eventType"] = eventType;
  state_message["action"] = action;
  state_message["actionType"] = actionType;
  state_message["wait"] = "0";
  state_message["data"] = data;

  String buffer;
  root.printTo(buffer);
  mesh.sendSingle(BRIDGE_ID, buffer);
  return buffer;
}

// =============================================
// ================ Parse State Packets ========
// =============================================

/**
 * Parse incoming packet
 * uses new standard
 * 
 */
void state_parsePacket(JsonObject &root)
{
  // process data when the action type is known
  const char *state_type = root["state"]["type"]; // "action"
  JsonObject &state_message = root["state"]["message"];
  const char *state_message_toId = state_message["toId"];
  const char *state_message_wait = state_message["wait"];
  const char *state_message_event = state_message["event"];
  const char *state_message_eventType = state_message["eventType"];
  const char *state_message_action = state_message["action"];
  const char *state_message_actionType = state_message["actionType"];

  //Switch data type depending on action to be completed
  if (root["toId"] == MY_ID || root["toId"] == String(mesh.getNodeId()))
  {
    Serial.println("Packet for me");
    Serial.println(state_message_actionType);
    // if (state_message_actionType == "relay")
    if(strcmp(state_message_actionType, "relay") == 0)
    {
      int data = root["state"]["message"]["data"];
      processRelayAction(state_message_action, data);
    }
    else //goes to serial
    {
      String msg;
      root.printTo(msg);
      Serial.println(msg);
    }
  }
  else
  {
    Serial.printf("Not for me");
    return;
  }
}

void state_forwardPacketToMesh(JsonObject &root)
{
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