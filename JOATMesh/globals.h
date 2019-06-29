//#define ARDUINOJSON_ENABLE_PROGMEM 0
//Global vars
#define MESH_PREFIX "meshssid"
#define MESH_PASSWORD "pass"
#define MESH_PORT 5555
#define MESH_CHANNEL 11

#define STATION_SSID "ssid"
#define STATION_PASSWORD "pass"

#define HOSTNAME "JoatServer"
#define MQTTENABLE
#define BRIDGE // to use when files are combined
uint32_t BRIDGE_ID;

int GLOBAL_PIN;
String nodeName = "logNode"; // Name needs to be unique
String MY_ID = "50";
String NODE_TYPE;

char *MY_ID_CHAR;

int MQTT_BROKER_ADDRESS[4] = {192, 168, 0, 51};
int MQTT_BROKER_PORT = 1883;
const char *MQTT_TOPIC = "root";
bool MQTT_ENABLED = true;

bool HTTP_ENABLED = true;

// Scheduler scheduler; //scheduler
// painlessMesh mesh;   // mesh

struct DEVICE
{
    String HARDWARE = "ESP32";
    String NODE_TYPE = "update";
    String MY_ID = "50";
    String BRIDGE_ID = "10";
    String SSID;
    String PASSWORD;
    String md5;
    int BROKER_ADDRESS[4];
    size_t noPart;
    size_t partNo = 0;
};

//Prototypes
void createJsonPacket(String fromId, String event, String eventType, String action, String actionType, String data);
void preparePacketForMesh(uint32_t from, String &msg);
void taskPrepareMeshReconnect();
void taskPrepareHeartbeat();
void taskBroadcastBridgeId();
void printBridgeStatus();
uint32_t getNodeHardwareId(String id);

char *IPAddressToString(uint8_t ip); //webServer

void processRelayAction(String action, int pinNo);                      //relay
void processMp3Action(String action, uint8_t folderId, uint8_t fileId); //mp3

void mqttCallback(char *topic, byte *payload, unsigned int length); // mqtt
void sendMqttPacket(String packet);                                 //mqtt

void newConnectionCallback(uint32_t nodeId);            //mesh
void mesh_receivedCallback(uint32_t from, String &msg); //mesh
bool mesh_sendSingle(uint32_t nodeId, String msg);
String mesh_getStationIp();
String mesh_getNodeId();
bool mesh_sendBroadcast(String msg);

void parseReceivedPacket(uint32_t from, String msg); //main
void printBridgeStatus();

// Commands
String cmd_create_bridgeId(uint32_t nodeId);
void cmd_query_nodeList();
void cmd_query_subConnections();
void cmd_bridgeId(JsonObject cmd);
void cmd_functionChange(JsonObject cmd);
void cmd_admin(JsonObject cmd);
void cmd_setId(JsonObject cmd);
void cmd_setName(JsonObject cmd);
void cmd_branchAddress(JsonObject cmd);
void cmd_mqttMode(JsonObject cmd);
bool validateFunctionChange(String msg);
void cmd_otaUpdate(JsonObject cmd);
String cmd_create_bridgeId(uint32_t nodeId, bool broadcast);
void cmd_customPinInit(JsonObject cmd);
void cmd_customPinToggle(JsonObject cmd);

//OTA
void otaReceiveUpdate(JsonObject root);

// Custom pins
void customPin_init(uint8_t pinNo, bool input);            //command
void customPin_toggle(uint8_t pinNo, bool active);         //command
void processCustomPinAction(uint8_t pinNo, String action); //EventAction

char *getMyIdChar()
{
    MY_ID.toCharArray(MY_ID_CHAR, MY_ID.length() + 1);
    return MY_ID_CHAR;
}

//Global strings
const PROGMEM String readyMessage = {"{\"ready\":\"true\"}"};