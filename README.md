# JOAT API

# JOAT command structure

---

- **Proposed - done on esp32**
  - packet structure -
  - Commands used to get/set the functionality of a node
  - State\* (pending better naming) to parse Event/Action data
  - Commands are agnostic for MQTT, Serial and HTTP


    TODO:

    - make query examples
    - get all libs associated & versions

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
        //Yet to implement
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
        	"message" : "new name to have(used for mqtt maybeh) or just use the id"
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

    - Enable support for legacy packet type i.e. sending events and actions with a toId

    # EEPROM settings

    - SSID - pending
    - Wifi password - pending
    - ID - Done - (as string atm)
    - Device type - done

    - Mesh SSID
    - Mesh password

    # Debugging

    Add the arduino debugging lib and tools

    [https://github.com/JoaoLopesF/SerialDebugConverter](https://github.com/JoaoLopesF/SerialDebugConverter)

    [https://github.com/JoaoLopesF/SerialDebug/archive/master.zip](https://github.com/JoaoLopesF/SerialDebug/archive/master.zip)

    [https://randomnerdtutorials.com/serialdebug-library-arduino-ide/](https://randomnerdtutorials.com/serialdebug-library-arduino-ide/) - HOW TO

    # Relay pins (ESP32)

    #define REL_PIN1 15

    #define REL_PIN2 2

    #define REL_PIN3 4

    #define REL_PIN4 5

    #define REL_PIN5 27

    #define REL_PIN6 14

    #define REL_PIN7 12

    #define REL_PIN8 13

        {
          "toId" : "<NODE_BRIDGE_ID - String>",
          "command":{
            "type" : "<COMMAND_TYPE - String>",
            "message" : "<MESSAGE - String>"
          }
        }

    # Command message structure

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
              "wait": "0",
              "event": "toggle",
              "eventType": "button",
              "action": "noneA",
              "actionType": "noneAT",
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
              "wait": "0",
              "event": "noneE",
              "eventType": "noneET",
              "action": "start",
              "actionType": "relay",
              "data": "1"
            }
          }
        }
