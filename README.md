# JOAT API

# JOAT command structure

---

    Sending a message to the branch server to forward to a node device

    {
    	"id":"10",
    	"message": {--state/command--}
    }


    //===================================
    {
    	"toId" : "10",
    	"command" : {
    		"type" : "functionChange",
    		"toId": "10", // - Redundant id?
    		"message" : "keypad"
    	}
    }

    {"command" : {
    	"type" : "admin",
    	"to_id": "1",
    	"message" : "getMeshNodes"
    },
    	"toId" : "10"}

    {"command" : {
    	"type" : "setId",
    	"toId": "10",
    	"message" : "11"
    },
    	"toId" : "10"}


    //=========================================
    {"command" : {
    	"type" : "branchAddress",
    	"message" : {
    		"address" : [192,168,0,50],
    		"port" : 1883
    	}
    },
    	"toId" : "10"}


    {"command" : {
    	"type" : "mqtt",
    	"message" : "enable"
    },
    	"toId" : "10"}

    {"command" : {
    	"type" : "name",
    	"message" : "new name to have or just use the id"
    },
    	"toId" : "10"}

    //=========================================
    Event/Action packet
    {
    	"toId":"10",
      "state": {
        "type": "action",
        "message": {
          "toId": "11",
          "wait": "0",
          "event": "noneE",
          "eventType": "noneET",
          "action": "start",
          "actionType": "relay",
          "data": "1"
        }
      }
    }

# Libraries

The JOATESP32 uses many different libs these are:

- painlessMesh
  - ArduinoJSon
  - AsyncTCP
  - TaskScheduler
- LinkedList
  - Note: Painless mesh uses an older intergrated version of this, therefore there is an updated version with renamed members in the repo.
- MFRC522
  - This is used for the RFID
- ESPAsyncWebServer
  - Used for the web server
- AsyncMqttClient
  - Used for mqtt messaging in a non blocking capacity

# EEPROM settings

- SSID - pending
- Wifi password - pending
- ID - Done - (as string atm)
- Device type - done

- Mesh SSID
- Mesh password

# Debugging

Add the arduino debugging lib and tools (TODO)

[https://github.com/JoaoLopesF/SerialDebugConverter](https://github.com/JoaoLopesF/SerialDebugConverter)

[https://github.com/JoaoLopesF/SerialDebug/archive/master.zip](https://github.com/JoaoLopesF/SerialDebug/archive/master.zip)

[https://randomnerdtutorials.com/serialdebug-library-arduino-ide/](https://randomnerdtutorials.com/serialdebug-library-arduino-ide/) - HOW TO

# Relay Config(ESP32)

#define REL_PIN1 15

#define REL_PIN2 2

#define REL_PIN3 4

#define REL_PIN4 5

#define REL_PIN5 27

#define REL_PIN6 14

#define REL_PIN7 12

#define REL_PIN8 13

This is designed to handle an 8-channel relay. (Note:; the relay needs a 5V power source, as the ESP32 cannot handle this).

    {
    	"toId":"10",
      "state": {
        "type": "action",
        "message": {
          "toId": "11"
          "action": "start",
          "actionType": "relay",
          "data": "1"
        }
      }
    }

# Button Config(ESP32)

#define BTN_PIN0 15
#define BTN_PIN1 2
#define BTN_PIN2 0
#define BTN_PIN3 4
#define BTN_PIN4 16
#define BTN_PIN5 17
#define BTN_PIN6 5
#define BTN_PIN7 18
#define BTN_PIN8 19
#define BTN_PIN9 21

These buttons have a debounce of 125ms ensuring that the button is not double triggered in the same press

    {
    	"toId":"10",
      "state": {
        "type": "action",
        "message": {
          "toId": "11",
          "event": "toggle",
          "eventType": "button",
          "data": "1"
        }
      }
    }

# ReedSwitch Config(ESP32)

These pins correspond to an array that is returned as the data parameter.

0 = 15
1 = 2
2 = 0
3 = 4
4 = 16
5 = 17
6 = 5
7 = 18
8 = 19
9 = 21

Example is pins(15, 2, 0, 4) are active, the rest have no magnet near them:

the data returned = [1,1,1,1,0,0,0,0,0,0]

    {
    	"toId":"10",
      "state": {
        "type": "action",
        "message": {
          "toId": "11",
          "event": "toggle",
          "eventType": "magSwitch",
          "data": "[1,1,1,1,0,0,0,0,0,0]"
        }
      }
    }

# RFID Config(ESP32)

- SDA = 5
- SCK = 18
- MISO = 19
- MOSI = 23
- RST = 22

  {
  "toId":"10",
  "state": {
  "type": "action",
  "message": {
  "toId": "11",
  "event": "code",
  "eventType": "rfid",
  "data": "1A 23 4B"
  }
  }
  }

# Keypad Config(ESP32)

#define LEFT_PIN1 15 // Left pin on keyp pad pin1
#define LEFT_PIN2 2
#define LEFT_PIN3 0
#define LEFT_PIN4 4
#define LEFT_PIN5 27
#define LEFT_PIN6 14
#define LEFT_PIN7 12
#define LEFT_PIN8 13

============ New layout =======

#define LEFT_PIN1 15 // Left pin on keyp pad pin1

#define LEFT_PIN2 2

#define LEFT_PIN3 4

#define LEFT_PIN4 5

#define LEFT_PIN5 18

#define LEFT_PIN6 19

#define LEFT_PIN7 21

#define LEFT_PIN8 23

Key pad has optional buzzer on pin 6

    {
    	"toId":"10",
      "state": {
        "type": "action",
        "message": {
          "toId": "11",
          "event": "code",
          "eventType": "keypad",
          "data": "112234"
        }
      }
    }

# Command message structure

    {
      "toId" : "<NODE_BRIDGE_ID - String>",
      "command":{
        "type" : "<COMMAND_TYPE - String>",
        "message" : "<MESSAGE - String>"
      }
    }

# Event Action message structure

    {
    	"toId":"<NODE_ID - String>",
      "state": {
        "type": "<EVENT OR ACTION - String",
        "message": {
          "toId": "<NODE_ID - String>",
          "wait": "<WAIT_TIME_SECONDS - string>",
          "event": "<EVENT - String>",
          "eventType": "<EVENT_TYPE - String>",
          "action": "<ACTION - string>",
          "actionType": "<ACTION_TYPE>",
          "data": "<DATA_TO_SEND - Various>"
        }
      }
    }

## Event/Event Types

[Events](https://www.notion.so/585ed7d423d2476b918542a9558e230b)

## Action/ActionTypes

[Actions](https://www.notion.so/91dbbbb3c0cd47268d9d9248f2cfe885)

## Event Example

Events are emitted from the node device. A node will not take actions on receiving an event.

    {
    	"toId":"10",
      "state": {
        "type": "event",
        "message": {
          "toId": "10",
          "event": "toggle",
          "eventType": "button",
          "data": "1"
        }
      }
    }

## Action Example

Actions are sent to the node devices to perform the specified action on the action type

    // This example starts a relay on pin 1 on the node device.
    {
    	"toId":"10",
      "state": {
        "type": "action",
        "message": {
          "toId": "10",
          "action": "start",
          "actionType": "relay",
          "data": "1"
        }
      }
    }
