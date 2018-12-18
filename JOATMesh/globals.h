//#define ARDUINOJSON_ENABLE_PROGMEM 0
//Global vars
int GLOBAL_PIN;
String nodeName = "logNode"; // Name needs to be unique
String MY_ID = "10";
String NODE_TYPE;

//Prototypes
void createJsonPacket(String fromId, String event, String eventType, String action, String actionType, String data);
