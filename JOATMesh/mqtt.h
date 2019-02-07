// #include "globals.h"
#include <PubSubClient.h> // mqtt
#include <WiFiClient.h>   //mqtt

WiFiClient wifiClient;
PubSubClient *_pubSubClient;

IPAddress mqttGetlocalIP();
IPAddress mqttMyIP(0, 0, 0, 0);

// Prototypes
void sendMqttConnectionPayload();
char *string2char(String command);
void reconnect();
void mqttConnect();


void mqtt_init()
{
    Serial.println("Init MQTT");
    getMqttBrokerAddress();
    int zeroAddr[4];
    // If the address is set in the sketch as default use that
    if (tempMqttAddr != zeroAddr)
    {
        memcpy(MQTT_BROKER_ADDRESS, tempMqttAddr, 4);
    }
    else
    {
        setBrokerAddress(MQTT_BROKER_ADDRESS);
    }

    // IPAddress mqttBroker(MQTT_BROKER_ADDRESS[0], MQTT_BROKER_ADDRESS[1], MQTT_BROKER_ADDRESS[2], MQTT_BROKER_ADDRESS[3]);
    IPAddress mqttBroker(192, 168, 0, 180);
    _pubSubClient = new PubSubClient(mqttBroker, MQTT_BROKER_PORT, mqttCallback, wifiClient);
    Serial.print("Mqtt broker address:");
    Serial.print(mqttBroker);
    Serial.print(":");
    Serial.println(MQTT_BROKER_PORT);
}

// TODO: unfinished from example
void processMqtt()
{

    // loop the mqtt client logic
    _pubSubClient->loop();

    if (mqttMyIP != mqttGetlocalIP())
    {
        mqttMyIP = mqttGetlocalIP();
        Serial.println("MQTT: My IP is " + mqttMyIP.toString());

        mqttConnect();
    }
    // reconnect if not connected
    if (!_pubSubClient->connected())
    {
        mqttConnect();
    }
}



void sendMqttPacket(String packet)
{
    _pubSubClient->publish("root", string2char(packet));
    Serial.println("====Sending mqtt now======");
    Serial.println(string2char(packet));
}

/**
 * Helper to give a const char* instaead of string
 */
char *string2char(String command)
{
    if (command.length() != 0)
    {
        char *p = const_cast<char *>(command.c_str());
        return p;
    }
}

/**
 * Process incoming MQTT packets here
 */
void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
    char *cleanPayload = (char *)malloc(length + 1);
    payload[length] = '\0';
    memcpy(cleanPayload, payload, length + 1);
    String msg = String(cleanPayload);
    free(cleanPayload);
    String targetStr = String(topic).substring(16);

    
    
    Serial.print(" ====== MQTT callback: ============");
    Serial.print("topic: ");
    Serial.print(topic);
    // Serial.print("payload: ");
    // Serial.print(payload);
    Serial.print("clean msg: ");
    Serial.print(msg);
    Serial.print("target str: ");
    Serial.println(targetStr);


    // This is where all messages to this device belong
    preparePacketForMesh(mesh.getNodeId(), msg);

    //msg to json
    // target == myid
    // if msg.command -> parseCommand
    // if msg.eventAction -> parseEventAction


    // if (targetStr == "gateway")
    // {
    //     if (msg == "getNodes")
    //     {
    //         _pubSubClient->publish(MQTT_TOPIC, mesh.subConnectionJson().c_str());
    //     }
    // }
    // else if (targetStr == "broadcast")
    // {
    //     mesh.sendBroadcast(msg);
    // }
    // else
    // {
    //     uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    //     if (mesh.isConnected(target))
    //     {
    //         mesh.sendSingle(target, msg);
    //     }
    //     else
    //     {
    //         _pubSubClient->publish(MQTT_TOPIC, "Client not connected!");
    //     }
    // }
}

void mqttConnect(){
    if (_pubSubClient->connect(string2char(MY_ID)))
        {
            Serial.print("Attempting MQTT connection...");
            sendMqttConnectionPayload();
            // TODO: send connection info
        }
}

// TODO: send connection packet here
void sendMqttConnectionPayload()
{
    _pubSubClient->publish(MQTT_TOPIC, "Ready!");
    _pubSubClient->subscribe(string2char(MY_ID));
}

IPAddress mqttGetlocalIP()
{
    return IPAddress(mesh.getStationIP());
}
