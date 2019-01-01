#include "IPAddress.h"

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

IPAddress getlocalIP();

AsyncWebServer server(80);
IPAddress myIP(0, 0, 0, 0);
IPAddress myAPIP(0, 0, 0, 0);

void webServer_init()
{
    myAPIP = IPAddress(mesh.getAPIP());
    Serial.println("My AP IP is " + myAPIP.toString());
    mesh.setHostname(HOSTNAME);

    //=========================================//
    //==== Web server routes ================//
    //=======================================//

    //TODO: make a form here to update nodes

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
        String list;
        nodeList.printTo(list);
        request->send(200, "text/html", "{\"nodes\":[ " + list + "]}");
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String mem = String(ESP.getFreeHeap());
        request->send(200, "text/html", "{\"memory\":" + mem + "}");
    });

    server.begin();
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