#include "globals.h"
#include <PubSubClient.h> // mqtt
#include <WiFiClient.h>   //mqtt

WiFiClient wifiClient;
PubSubClient *_pubSubClient = NULL;

IPAddress mqttGetlocalIP();
IPAddress mqttMyIP(0, 0, 0, 0);

void mqtt_init()
{
    // If the address is set in the sketch as default use that
    if (getMqttBrokerAddress() != [ 0, 0, 0, 0 ])
    {
        MQTT_BROKER_ADDRESS = getMqttBrokerAddress();
    }
    else
    {
        getMqttBrokerAddress(MQTT_BROKER_ADDRESS);
    }

    IPAddress mqttBroker(MQTT_BROKER_ADDRESS[0], MQTT_BROKER_ADDRESS[1], MQTT_BROKER_ADDRESS[2], MQTT_BROKER_ADDRESS[3]);
    _pubSubClient = new mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);
}

// TODO: unfinished from example
void processMqtt()
{
    mqttClient.loop();

    if (mqttMyIP != mqttGetlocalIP())
    {
        mqttMyIP = mqttGetlocalIP();
        Serial.println("MQTT: My IP is " + mqttMyIP.toString());

        if (mqttClient.connect(MY_ID))
        {
            sendMqttConnectionPayload();
        }
    }
}

// TODO: send connection packet here
void sendMqttConnectionPayload()
{
    mqttClient.publish("painlessMesh/from/gateway", "Ready!");
    mqttClient.subscribe("painlessMesh/to/#");
}

void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
    char *cleanPayload = (char *)malloc(length + 1);
    payload[length] = '\0';
    memcpy(cleanPayload, payload, length + 1);
    String msg = String(cleanPayload);
    free(cleanPayload);

    String targetStr = String(topic).substring(16);

    if (targetStr == "gateway")
    {
        if (msg == "getNodes")
        {
            mqttClient.publish("painlessMesh/from/gateway", mesh.subConnectionJson().c_str());
        }
    }
    else if (targetStr == "broadcast")
    {
        mesh.sendBroadcast(msg);
    }
    else
    {
        uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
        if (mesh.isConnected(target))
        {
            mesh.sendSingle(target, msg);
        }
        else
        {
            mqttClient.publish("painlessMesh/from/gateway", "Client not connected!");
        }
    }
}

IPAddress mqttGetlocalIP()
{
    return IPAddress(mesh.getStationIP());
}