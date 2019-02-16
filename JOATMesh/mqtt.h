// #include "globals.h"
#define MQTT_MAX_PACKET_SIZE 2048

#include <PubSubClient.h> // mqtt
#include <WiFiClient.h>   //mqtt

#define MQTT_MAX_PACKET_SIZE 2048

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
    // _pubSubClient->publish(MQTT_TOPIC, string2char(packet));
    _pubSubClient->publish(MQTT_TOPIC, const_cast<char *>(packet.c_str()));
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
    Serial.print("clean msg: ");
    Serial.print(msg);
    Serial.print("target str: ");
    Serial.println(targetStr);

    // This is where all messages to this device belong
    preparePacketForMesh(mesh.getNodeId(), msg);
    sendMqttPacket(readyMessage);
    return;
}

String createMqttConnectionPacket()
{
    DynamicJsonBuffer jsonBuffer;

    JsonObject &root = jsonBuffer.createObject();
    root["JOAT_CONNECT"] = true;
    root["id"] = MY_ID;
    root["ipAddress"] = mqttMyIP.toString();
    root["name"] = MY_ID;
    String msg;
    root.printTo(msg);

    return msg;
}

void mqttConnect()
{
    if (_pubSubClient->connect(string2char(MY_ID)))
    {
        Serial.print("Attempting MQTT connection...");
        sendMqttConnectionPayload();
    }
}

// TODO: send connection packet here
void sendMqttConnectionPayload()
{
    String connectionPacket = createMqttConnectionPacket();
    char* msg = string2char(connectionPacket);

    _pubSubClient->publish(MQTT_TOPIC, msg);
    _pubSubClient->subscribe(string2char(MY_ID));
    Serial.print("MQTT connected!");
}

IPAddress mqttGetlocalIP()
{
    return IPAddress(mesh.getStationIP());
}
