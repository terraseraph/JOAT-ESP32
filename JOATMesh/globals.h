//#define ARDUINOJSON_ENABLE_PROGMEM 0
//Global vars
#define MESH_PREFIX "seraphimMesh"
#define MESH_PASSWORD "somepassword"
#define MESH_PORT 5555

#define STATION_SSID "cerberus"
#define STATION_PASSWORD "Midgar5481"

#define HOSTNAME "JoatServer"
#define MQTTENABLE
#define BRIDGE // to use when files are combined
uint32_t BRIDGE_ID;

int GLOBAL_PIN;
String nodeName = "logNode"; // Name needs to be unique
String MY_ID = "10";
String NODE_TYPE;

char* MY_ID_CHAR;
    

int MQTT_BROKER_ADDRESS[4] = {192,168,0,180};
const char* MQTT_TOPIC = "root";
bool MQTT_ENABLED = true;





painlessMesh mesh; // mesh

//Prototypes
void createJsonPacket(String fromId, String event, String eventType, String action, String actionType, String data);
void preparePacketForMesh(uint32_t from, String &msg);
void printNodeList();

void mqttCallback(char* topic, byte* payload, unsigned int length); // mqtt
void sendMqttPacket(String packet); //mqtt

//Json objects
DynamicJsonBuffer jsonNodeListBuffer;
JsonObject &nodeList = jsonNodeListBuffer.createObject();



char* getMyIdChar(){
    MY_ID.toCharArray(MY_ID_CHAR, MY_ID.length()+1);
    return MY_ID_CHAR;
}