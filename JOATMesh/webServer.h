#include "IPAddress.h"

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

IPAddress getlocalIP();

AsyncWebServer server(80);
IPAddress myIP(0, 0, 0, 0);
IPAddress myAPIP(0, 0, 0, 0);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

String body; //for saving body messages

//prototype
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

void webServer_init()
{
    myAPIP = IPAddress(mesh.getAPIP());
    Serial.println("My AP IP is " + myAPIP.toString());
    mesh.setHostname(HOSTNAME);

    //=========================================//
    //==== Web server routes ================//
    //=======================================//

    //TODO: make a form here to update nodes

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    events.onConnect([](AsyncEventSourceClient *client) {
        client->send("hello!", NULL, millis(), 1000);
    });
    server.addHandler(&events);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
        if (request->hasArg("BROADCAST"))
        {
            String msg = request->arg("BROADCAST");
            mesh.sendBroadcast(msg);
        }
        if (request->hasArg("command"))
        {
            String msg = request->arg("command");
            preparePacketForMesh(0, msg);
        }
    });

    server.on("/nodes", HTTP_GET, [](AsyncWebServerRequest *request) {
        SimpleList<uint32_t> nodes;
        nodes = mesh.getNodeList();
        String list;
        SimpleList<uint32_t>::iterator node = nodes.begin();
        while (node != nodes.end())
        {
            list = list + (char)*node;
            Serial.printf(" %u", *node);
            node++;
        }

        // String list = (char *)mesh.getNodeList();
        // nodeList.printTo(list);
        // serializeJson(nodeList, list);
        request->send(200, "text/html", "{\"nodes\":[ " + list + "]}");
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String mem = String(ESP.getFreeHeap());
        request->send(200, "text/html", "{\"memory\":" + mem + "}");
    });

    server.on(
        "/post",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            request->send(200);
            if (!index) //new body
            {
                body = (char *)data;
            }
            else
            {
                body = body + (char *)data;
            }
            if (index + len == total)
            {
                preparePacketForMesh(0, body);
                body = "";
            }

            Serial.println("");
            Serial.println("==== post req ====");
            Serial.println("len");
            Serial.println(len);
            Serial.println("index");
            Serial.println(index);
            Serial.println("total");
            Serial.println(total);
            // free(data);
        });

    server.begin();
}

//=========================================//
//==== Web socket routes ================//
//=======================================//
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    }
    else if (type == WS_EVT_ERROR)
    {
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    }
    else if (type == WS_EVT_PONG)
    {
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len)
        {
            //the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += (char)data[i];
                }
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < info->len; i++)
                {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());

            if (info->opcode == WS_TEXT)
                client->text("I got your text message");
            else
                client->binary("I got your binary message");
        }
        else
        {
            //message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0)
            {
                if (info->num == 0)
                    Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += (char)data[i];
                }
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < info->len; i++)
                {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());

            if ((info->index + len) == info->len)
            {
                Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if (info->final)
                {
                    Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                    if (info->message_opcode == WS_TEXT)
                        client->text("I got your text message");
                    else
                        client->binary("I got your binary message");
                }
            }
        }
    }
}

IPAddress getlocalIP()
{
    return IPAddress(mesh.getStationIP());
}

char *IPAddressToString(uint8_t ip)
{
    char result[16];

    sprintf(result, "%d.%d.%d.%d",
            (ip >> 24) & 0xFF,
            (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF,
            (ip)&0xFF);

    return result;
}