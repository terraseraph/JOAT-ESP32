/**
 * EXAMPLE OTA: 
 *  {"command" : {
      "type" : "ota",
      "message" : {
        "data": "<massive base64 string>"
      }
    },
      "toId" : "10"}
 * 
 * 
 */

#include <Update.h>
#include <SPIFFS.h>
#include "base64.h"
#define OTA_FN "/ota_fw.json"

struct firmware_ota_t
{
    String hardware = "ESP32";
    String nodeType = "update";
    String md5;
    size_t noPart;
    size_t partNo = 0;
};

// Prototypes
void sendMessage();
void firmwareFromJSON(firmware_ota_t &fw, JsonObject &req);
void createDataRequest(JsonObject &req, firmware_ota_t updateFW);
void otaUpdate(uint32_t from, String &msg);
void otaReceiveUpdate(JsonObject root);
void otaSendUpdate();

void createDataRequest(JsonObject &req, firmware_ota_t updateFW)
{
    req["plugin"] = "ota";
    req["type"] = "request";
    req["hardware"] = updateFW.hardware;
    req["nodeType"] = updateFW.nodeType;
    req["md5"] = updateFW.md5;
    req["noPart"] = updateFW.noPart;
    req["partNo"] = updateFW.partNo;
}

void firmwareFromJSON(firmware_ota_t &fw, JsonObject &req)
{
    fw.hardware = req["hardware"].as<String>();
    fw.nodeType = req["nodeType"].as<String>();
    fw.md5 = req["md5"].as<String>();
}

firmware_ota_t currentFW;
firmware_ota_t updateFW;
Task otaDataRequestTask;
Scheduler otaScheduler;

void otaSendUpdate()
{
}

void otaReceiveUpdate(JsonObject root)
{
    //    write data
    auto b64data = root["message"]["data"].as<std::string>();
    auto b64Data = base64_decode(b64data);

    if (Update.write((uint8_t *)b64Data.c_str(),
                     b64Data.length()) != b64Data.length())
    {
        Serial.println("handleOTA(): OTA write failed!");
        Update.printError(Serial);
        Update.end();
        return;
    }

    //       check md5, reboot
    if (Update.end(true))
    {
        Serial.printf("Update.MD5: %s\n",
                      Update.md5String().c_str());
        Serial.println("handleOTA(): OTA Success!");
        ESP.restart();
    }
    else
    {
        Serial.println("handleOTA(): OTA failed!");
        Update.printError(Serial);
    }
}

// Replaces received callback
void otaUpdate(uint32_t from, String &msg)
{
    // bool isJSON = false;
    // DynamicJsonDocument jsonBuffer(1024 + msg.length());
    // DeserializationError error = deserializeJson(jsonBuffer, msg);
    // if (!error)
    // {
    //     isJSON = true;
    // }
    // JsonObject root = jsonBuffer.as<JsonObject>();

    // if (isJSON && root.containsKey("plugin") &&
    //     String("ota").equals(root["plugin"].as<String>()))
    // {
    //     Serial.printf("startHere: Received OTA msg from %u msg=%s\n", from,
    //                   msg.c_str());

    //     if (currentFW.nodeType.equals(root["nodeType"].as<String>()) &&
    //         currentFW.hardware.equals(root["hardware"].as<String>()))
    //     {
    //         if (String("version").equals(root["type"].as<String>()))
    //         {
    //             if (currentFW.md5.equals(root["md5"].as<String>()) ||
    //                 updateFW.md5.equals(root["md5"].as<String>()))
    //                 return; // Announced version already known
    //             else
    //             {
    //                 // Setup new updatedFW
    //                 updateFW = currentFW;
    //                 updateFW.md5 = root["md5"].as<String>();
    //                 updateFW.partNo = 0;
    //                 updateFW.noPart = root["noPart"].as<size_t>();

    //                 // Setup otaDataRequestTask (enableIfNot)
    //                 otaDataRequestTask.set(30 * TASK_SECOND, 5, [from]() {
    //                     DynamicJsonDocument jsonBuffer(256);
    //                     auto req = jsonBuffer.to<JsonObject>();
    //                     createDataRequest(req, updateFW);
    //                     String msg;
    //                     serializeJson(req, msg);
    //                     uint32_t cpyFrom = from;
    //                     mesh.sendSingle(cpyFrom, msg);
    //                 });

    //                 otaDataRequestTask.enableIfNot();
    //                 otaDataRequestTask.forceNextIteration();
    //                 Serial.printf("Requesting firmware update\n");
    //             }
    //         }
    //         else if (String("data").equals(root["type"].as<String>()) &&
    //                  updateFW.partNo == root["partNo"].as<size_t>())
    //         {
    //             size_t partNo = root["partNo"];
    //             size_t noPart = root["noPart"];
    //             if (partNo == 0)
    //             {
    //                 String otaMD5 = root["md5"].as<String>();
    //                 uint32_t maxSketchSpace = UPDATE_SIZE_UNKNOWN;

    //                 Serial.printf("Sketch size %d\n", maxSketchSpace);
    //                 if (Update.isRunning())
    //                 {
    //                     Update.end(false);
    //                 }
    //                 if (!Update.begin(
    //                         maxSketchSpace))
    //                 { // start with max available size
    //                     Serial.println("handleOTA(): OTA start failed!");
    //                     Update.printError(Serial);
    //                     Update.end();
    //                 }
    //                 else
    //                 {
    //                     Update.setMD5(otaMD5.c_str());
    //                 }
    //             }
    //             //    write data
    //             auto b64data = root["data"].as<std::string>();
    //             auto b64Data = base64_decode(b64data);

    //             if (Update.write((uint8_t *)b64Data.c_str(),
    //                              b64Data.length()) != b64Data.length())
    //             {
    //                 Serial.println("handleOTA(): OTA write failed!");
    //                 Update.printError(Serial);
    //                 Update.end();
    //                 return;
    //             }

    //             if (partNo == noPart - 1)
    //             {
    //                 //       check md5, reboot
    //                 if (Update.end(true))
    //                 { // true to set the size to the
    //                     // current progress
    //                     Serial.printf("Update.MD5: %s\n",
    //                                   Update.md5String().c_str());
    //                     auto file = SPIFFS.open(OTA_FN, "w");
    //                     DynamicJsonDocument jsonBuffer(1024);
    //                     auto req = jsonBuffer.to<JsonObject>();

    //                     createDataRequest(req, updateFW);
    //                     String msg;

    //                     serializeJson(req, msg);

    //                     file.print(msg);
    //                     file.close();

    //                     Serial.println("handleOTA(): OTA Success!");
    //                     ESP.restart();
    //                 }
    //                 else
    //                 {
    //                     Serial.println("handleOTA(): OTA failed!");
    //                     Update.printError(Serial);
    //                 }
    //                 otaDataRequestTask.disable();
    //             }
    //             else
    //             {
    //                 ++updateFW.partNo;
    //                 otaDataRequestTask.setIterations(5);
    //                 otaDataRequestTask.forceNextIteration();
    //             }
    //         }
    //     }
    // }

    // Serial.printf("ota: Received from %u msg=%s\n", from, msg.c_str());
}

void otaSetup()
{

    // otaScheduler.addTask(otaDataRequestTask);

    // SPIFFS.begin(true); // Start the SPI Flash Files System

    // if (SPIFFS.exists(OTA_FN))
    // {
    //     auto file = SPIFFS.open(OTA_FN, "r");
    //     String msg = "";
    //     while (file.available())
    //     {
    //         msg += (char)file.read();
    //     }

    //     DynamicJsonDocument jsonBuffer(1024);
    //     DeserializationError error = deserializeJson(jsonBuffer, msg);
    //     if (error)
    //     {
    //         Serial.printf("JSON DeserializationError\n");
    //     }
    //     JsonObject root = jsonBuffer.as<JsonObject>();

    //     firmwareFromJSON(currentFW, root);

    //     Serial.printf("Current firmware MD5: %s, type = %s, hardware = %s\n",
    //                   currentFW.md5.c_str(), currentFW.nodeType.c_str(),
    //                   currentFW.hardware.c_str());
    //     file.close();
    // }
    // else
    // {
    //     Serial.printf("No OTA_FN found!\n");
    // }
}