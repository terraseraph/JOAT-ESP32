
painlessMesh mesh; // mesh

void mesh_init()
{
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION | COMMUNICATION); // set before init() so that you can see startup messages
    // mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
    mesh.onReceive(&mesh_receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.setContainsRoot(true);
}
void meshBridgeInit()
{
    mesh.stationManual(STATION_SSID, STATION_PASSWORD);
    mesh.setHostname(HOSTNAME);
    mesh.setRoot(true);
}

void processMeshLoop()
{
    mesh.update();
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
        mesh_sendSingle(nodeId, msg);
        // mesh.sendSingle(nodeId, msg);
    }
    return;
}

//==============================//
//=====Received callback=======//
//============================//
void mesh_receivedCallback(uint32_t from, String &msg)
{
    parseReceivedPacket(from, msg);
    return;
}

//=================================//
//====== Helpers ==================//
//=================================//

String mesh_getStationIp()
{
    return mesh.getStationIP().toString();
}

bool mesh_sendSingle(uint32_t nodeId, String msg)
{
    return mesh.sendSingle(nodeId, msg);
}

String mesh_getNodeId()
{
    return String(mesh.getNodeId());
}

bool mesh_sendBroadcast(String msg)
{
    return mesh.sendBroadcast(msg, false);
}

void mesh_restartMesh()
{
    mesh.stop();
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
}
