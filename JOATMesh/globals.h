//#define ARDUINOJSON_ENABLE_PROGMEM 0
//Global vars
#define MESH_PREFIX "seraphimMesh"
#define MESH_PASSWORD "somepassword"
#define MESH_PORT 5555

#define STATION_SSID "NFC"
#define STATION_PASSWORD "somepassword"

#define HOSTNAME "MQTT_Bridge"
#define MQTTENABLE
#define BRIDGE // to use when files are combined
uint32_t BRIDGE_ID;

int GLOBAL_PIN;
String nodeName = "logNode"; // Name needs to be unique
String MY_ID = "11";
String NODE_TYPE;
bool MQTT_ENABLED = true;

//Prototypes
void createJsonPacket(String fromId, String event, String eventType, String action, String actionType, String data);
