
#include <WiFiClient.h>
#include <AsyncMqttClient.h>

WiFiClient wifiClient;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

IPAddress mqttGetlocalIP();
IPAddress mqttMyIP(0, 0, 0, 0);

// Prototypes
void sendMqttConnectionPayload();
char *string2char(String command);
void WiFiEvent(WiFiEvent_t event);
void connectToMqtt();
void asyncMqttSetup();

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

    Serial.print("Mqtt broker address:");
    Serial.print(mqttBroker);
    Serial.print(":");
    Serial.println(MQTT_BROKER_PORT);

    asyncMqttSetup();
    WiFi.onEvent(WiFiEvent); // this is where the mqtt events fire from
}

/**
 * Mqtt loop
 */
void processMqtt()
{
    mqttMyIP = mqttGetlocalIP();
}

/**
 * Send packet to Mqtt broker
 */
void sendMqttPacket(String packet)
{
    if (!mqttClient.connected())
    {
        return;
    }
    mqttClient.publish(MQTT_TOPIC, 0, false, string2char(packet));
    Serial.println("====Sending mqtt now======");
    Serial.println(packet);
}

/**
 * Listening to WIFI events to only attempt connection with broker when on a network
 */
void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        if (MQTT_ENABLED)
        {
            connectToMqtt();
        }
        Serial.println(WiFi.localIP());
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        if (MQTT_ENABLED)
        {
            xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
            xTimerStart(wifiReconnectTimer, 0);
        }
        break;
    }
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

// send connection packet here
void sendMqttConnectionPayload()
{
    String connectionPacket = createMqttConnectionPacket();
    sendMqttPacket(connectionPacket);
}

IPAddress mqttGetlocalIP()
{
    return IPAddress(mesh.getStationIP());
}

void connectToWifi()
{
    Serial.println("Connecting to Wi-Fi...not really already done!");
}

void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
    uint16_t packetIdSub = mqttClient.subscribe(string2char(MY_ID), 1);
    Serial.print("Subscribing at QoS 2, packetId: ");
    Serial.println(packetIdSub);

    sendMqttConnectionPayload();
    // mqttClient.publish("test/lol", 0, true, "test 1");
    //uint16_t mqttClient.publish(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0)
}

//TODO: async mqtt
void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("Disconnected from MQTT.");

    if (WiFi.isConnected())
    {
        xTimerStart(mqttReconnectTimer, 0);
    }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId)
{
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

/**
 * Process incoming MQTT packets here
 */
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    // if (topic != string2char(MY_ID))
    // { //Was able to get messages i was not subbed to..
    //     return;
    // }
    Serial.println("Publish received.");
    Serial.print("  topic: ");
    Serial.println(topic);
    Serial.print("  qos: ");
    Serial.println(properties.qos);
    Serial.print("  dup: ");
    Serial.println(properties.dup);
    Serial.print("  retain: ");
    Serial.println(properties.retain);
    Serial.print("  len: ");
    Serial.println(len);
    Serial.print("  index: ");
    Serial.println(index);
    Serial.print("  total: ");
    Serial.println(total);
    String msg = String(payload);
    Serial.print("===== payload =====");
    Serial.println(msg);
    preparePacketForMesh(mesh.getNodeId(), msg);
    sendMqttPacket(readyMessage);
}

void onMqttPublish(uint16_t packetId)
{
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

/**
 * Setup callbacks
 */
void asyncMqttSetup()
{
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

    IPAddress mqttBroker(192, 168, 0, 180);
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(mqttBroker, MQTT_BROKER_PORT);
    mqttClient.setClientId(string2char(MY_ID));
    mqttClient.setMaxTopicLength(1024);
}