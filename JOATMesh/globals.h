//#define ARDUINOJSON_ENABLE_PROGMEM 0
//Global vars
#define MESH_PREFIX "seraphimMesh"
#define MESH_PASSWORD "somepassword"
#define MESH_PORT 5555

#define STATION_SSID "someNetwork"
#define STATION_PASSWORD "somepassword"

#define HOSTNAME "JoatServer"
#define MQTTENABLE
#define BRIDGE // to use when files are combined
uint32_t BRIDGE_ID;

int GLOBAL_PIN;
String nodeName = "logNode"; // Name needs to be unique
String MY_ID = "16";
String NODE_TYPE;

char *MY_ID_CHAR;

int MQTT_BROKER_ADDRESS[4] = {192, 168, 0, 180};
int MQTT_BROKER_PORT = 1883;
const char *MQTT_TOPIC = "root";
bool MQTT_ENABLED = true;

bool HTTP_ENABLED = true;

Scheduler scheduler; //scheduler
painlessMesh mesh;   // mesh

//Prototypes
void createJsonPacket(String fromId, String event, String eventType, String action, String actionType, String data);
void preparePacketForMesh(uint32_t from, String &msg);
void taskPrepareMeshReconnect();
void taskPrepareHeartbeat();
void taskBroadcastBridgeId();
void printNodeList();
uint32_t getNodeHardwareId(String id);

char *IPAddressToString(uint8_t ip); //webServer

void processRelayAction(String action, int pinNo); //relay

void mqttCallback(char *topic, byte *payload, unsigned int length); // mqtt
void sendMqttPacket(String packet);                                 //mqtt

//Json objects
DynamicJsonBuffer jsonNodeListBuffer;
JsonObject &nodeList = jsonNodeListBuffer.createObject();

char *getMyIdChar()
{
    MY_ID.toCharArray(MY_ID_CHAR, MY_ID.length() + 1);
    return MY_ID_CHAR;
}

//Global strings
const PROGMEM String readyMessage = {"{\"ready\":\"true\"}"};