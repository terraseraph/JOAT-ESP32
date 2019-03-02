//==========================================
// Got info from:
// https://www.instructables.com/id/ESP32-With-RFID-Access-Control/
// This header controls the RFID functionality
// SDA - 5
// SCK - 18
// MOS - 23
// MIS - 19
// RST - 22
// gnd
// 3v3
//==========================================

#include <SPI.h>     //library responsible for communicating of SPI bus
#include <MFRC522.h> //library responsible for communicating with the module RFID-RC522
#define SS_PIN 5
#define RST_PIN 22
#define SIZE_BUFFER 18
#define MAX_SIZE_BLOCK 16
#define greenPin 12
#define redPin 32

#define RFID_TOGGLE_DELAY 1000
//used in authentication
MFRC522::MIFARE_Key key;
//authentication return status code
MFRC522::StatusCode status;
// Defined pins to module RC522
// MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522 mfrc522(SS_PIN, RST_PIN);

void rfidWritingData();
void rfidReadingData();
void toggleGreenPin();
void toggleRedPin();
void dump_byte_array(byte *buffer, byte bufferSize);

Scheduler rdidScheduler;
Task taskToggleRed(RELAY_TOGGLE_DELAY, 1, &toggleRedPin);
Task taskToggleGreen(RFID_TOGGLE_DELAY, 1, &toggleGreenPin);

void rfid_init()
{

    rdidScheduler.init();
    rdidScheduler.addTask(taskToggleRed);
    rdidScheduler.addTask(taskToggleGreen);

    pinMode(greenPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    // SPIClass SPI();
    SPI.begin(); // Init SPI bus
    // Init MFRC522
    mfrc522.PCD_Init();
    mfrc522.PCD_DumpVersionToSerial();
    Serial.println("Approach your reader card...");
    Serial.println();
}

void processRfid()
{
    //waiting the card approach
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return;
    }
    // Select a card
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return;
    }

    // Dump debug info about the card; PICC_HaltA() is automatically called
    // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));</p><p>
    // call menu function and retrieve the desired option
    // int op = menu();
    int op = 0; //TODO: fix this to either allow writing a new cart or not

    if (op == 0)
        rfidReadingData();
    else if (op == 1)
        rfidWritingData();
    else
    {
        Serial.println(F("Incorrect Option!"));
        return;
    }

    //instructs the PICC when in the ACTIVE state to go to a "STOP" state
    mfrc522.PICC_HaltA();
    // "stop" the encryption of the PCD, it must be called after communication with authentication, otherwise new communications can not be initiated
    mfrc522.PCD_StopCrypto1();
}

void toggleRedPin()
{
    digitalWrite(redPin, LOW);
}

void toggleGreenPin()
{
    digitalWrite(greenPin, LOW);
}

//reads data from card/tag
void rfidReadingData()
{
    //prints the technical details of the card/tag
    mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

    //prepare the key - all keys are set to FFFFFFFFFFFFh
    for (byte i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF;

    //buffer for read data
    byte buffer[SIZE_BUFFER] = {0};

    //the block to operate
    byte block = 1;
    byte size = SIZE_BUFFER;                                                                         //authenticates the block to operate
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("Authentication failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        digitalWrite(redPin, HIGH);
        taskToggleRed.setIterations(1); //ensures the task re runs
        taskToggleRed.enable();
        return;
    }

    //read data from block
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("Reading failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        digitalWrite(redPin, HIGH);
        taskToggleRed.setIterations(1); //ensures the task re runs
        taskToggleRed.enable();
        return;
    }
    else
    {
        digitalWrite(greenPin, HIGH);
        taskToggleGreen.setIterations(1); //ensures the task re runs
        taskToggleGreen.enable();
    }
    Serial.print(F("\nData from block ["));
    Serial.print(block);
    Serial.print(F("]: "));
    // Serial.println(&(mfrc522.uid));

    //prints read data
    String cardId = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        cardId.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        cardId.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++)
    {
        Serial.write(buffer[i]);
    }
    Serial.println(" ");
    const char *strBuffer = reinterpret_cast<const char *>(buffer);
    // const char *strUid = reinterpret_cast<const char *>(&(mfrc522.uid));
    state_createAndSendPacket(MY_ID, "event", "code", "rfid", "noneA", "noneAT", cardId.substring(1));
}

void rfidWritingData()
{
    //prints thecnical details from of the card/tag
    mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

    // waits 30 seconds dor data entry via Serial
    Serial.setTimeout(30000L);
    Serial.println(F("Enter the data to be written with the '#' character at the end \n[maximum of 16 characters]:"));

    //prepare the key - all keys are set to FFFFFFFFFFFFh
    for (byte i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF;

    //buffer for storing data to write
    byte buffer[MAX_SIZE_BLOCK] = "";
    byte block;    //the block to operate
    byte dataSize; //size of data (bytes)

    //recover on buffer the data from Serial
    //all characters before chacactere '#'
    dataSize = Serial.readBytesUntil('#', (char *)buffer, MAX_SIZE_BLOCK);
    //void positions that are left in the buffer will be filled with whitespace
    for (byte i = dataSize; i < MAX_SIZE_BLOCK; i++)
    {
        buffer[i] = ' ';
    }

    block = 1;                   //the block to operate
    String str = (char *)buffer; //transforms the buffer data in String
    Serial.println(str);

    //authenticates the block to operate
    //Authenticate is a command to hability a secure communication
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                      block, &key, &(mfrc522.uid));

    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        digitalWrite(redPin, HIGH);
        taskToggleRed.setIterations(1); //ensures the task re runs
        taskToggleRed.enable();
        return;
    }
    //else Serial.println(F("PCD_Authenticate() success: "));

    //Writes in the block
    status = mfrc522.MIFARE_Write(block, buffer, MAX_SIZE_BLOCK);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        digitalWrite(redPin, HIGH);
        taskToggleRed.setIterations(1); //ensures the task re runs
        taskToggleRed.enable();
        return;
    }
    else
    {
        Serial.println(F("MIFARE_Write() success: "));
        digitalWrite(greenPin, HIGH);
        taskToggleGreen.setIterations(1); //ensures the task re runs
        taskToggleGreen.enable();
    }
}

//menu to operation choice
int menu()
{
    Serial.println(F("\nChoose an option:"));
    Serial.println(F("0 - Reading data"));
    Serial.println(F("1 - Writing data\n"));

    //waits while the user does not start data
    while (!Serial.available())
    {
    };

    //retrieves the chosen option
    int op = (int)Serial.read();

    //remove all characters after option (as \n per example)
    while (Serial.available())
    {
        if (Serial.read() == '\n')
            break;
        Serial.read();
    }
    return (op - 48); //subtract 48 from read value, 48 is the zero from ascii table
}
