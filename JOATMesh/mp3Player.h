// Most of the logic:
// https://github.com/cefaloide/ArduinoSerialMP3Player/blob/master/ArduinoSerialMP3Player/ArduinoSerialMP3Player.ino
// https://github.com/terraseraph/seraphim-escape-arduino-joat/blob/master/JOAT/MP3.cpp

#define MP3_BAUD_RATE 9600
#define MP3_START_CMD 0x7E
#define MP3_VERS 0xFF
#define MP3_END_CMD 0xEF
#define MP3_RX 16
#define MP3_TX 17

void MP3_init();
void MP3_Reset();
void MP3_NextSong();
void MP3_PrevSong();
void MP3_PlayWithIndex(uint8_t idx);
void MP3_VolumeUp();
void MP3_VolumeDown();
void MP3_SetVolume(uint8_t vol);
void MP3_SingleCyclePlay(uint8_t songIdx);
void MP3_SleepMode();
void MP3_Wakeup();
void MP3_Play();
void MP3_Pause();
void MP3_PlayWithFolderFilename(uint8_t folderId, uint8_t fileId);
void MP3_Stop();
void MP3_CyclePlayFolder(uint8_t folderId);
void MP3_ShufflePlay();
void MP3_GroupPlay(int cnt, uint8_t[]);
void MP3_PlayWithVolumeIndex(uint8_t vol, uint8_t songIdx);
String decodeMP3Answer();
String sanswer(void);
void processMp3Loop();

void MP3_sendCommand(uint8_t cmd, uint8_t dataLen, uint8_t data[]);

HardwareSerial mp3Serial(2);
uint8_t mp3Command[15];
uint8_t mp3Data[10];

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

void MP3_init()
{
    // mp3Serial.begin(MP3_BAUD_RATE);
    mp3Serial.begin(9600, SERIAL_8N1, MP3_RX, MP3_TX);
    MP3_Reset();
}

void processMp3Loop()
{
    char c = ' ';

    // Check for the answer.
    if (mp3Serial.available())
    {
        Serial.println(decodeMP3Answer());
    }
}

void processMp3Action(String action, uint8_t folderId, uint8_t fileId)
{
    Serial.print("MP3 action : ");
    Serial.print(action);
    Serial.print(" : ");
    Serial.print(folderId);
    Serial.print(" : ");
    Serial.print(fileId);

    if (action == "play")
    {
        MP3_PlayWithFolderFilename(folderId, fileId);
    }
    else if (action == "pause")
    {
        MP3_Pause();
    }
    else if (action == "resume")
    {
        MP3_Play();
    }
    else if (action == "repeatSingle")
    {
        MP3_SingleCyclePlay(fileId);
    }
    else if (action == "repeatFolder")
    {
        MP3_CyclePlayFolder(folderId);
    }
    else if (action == "next")
    {
        MP3_NextSong();
    }
    else if (action == "previous")
    {
        MP3_PrevSong();
    }
    else if (action == "stop")
    {
        MP3_Stop();
    }
}

void MP3_Reset()
{
    memset((void *)mp3Data, 0x00, sizeof(mp3Data));
    MP3_sendCommand(0x0C, 2, mp3Data); // reset;
    mp3Data[1] = 0x02;
    MP3_sendCommand(0x09, 2, mp3Data); // select device;
}
void MP3_NextSong()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x01, 2, mp3Data); // next song;
}
void MP3_PrevSong()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x02, 2, mp3Data); // prev song;
}
void MP3_PlayWithIndex(uint8_t idx)
{
    mp3Data[0] = 0x00;
    mp3Data[1] = idx;
    MP3_sendCommand(0x03, 2, mp3Data); // play indexed song;
}
void MP3_VolumeUp()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x04, 2, mp3Data); // volume up;
}
void VolumeDown()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x05, 2, mp3Data); // volume down;
}
void MP3_SetVolume(uint8_t vol)
{
    mp3Data[0] = 0x00;
    if (vol < 0)
        mp3Data[1] = 0;
    else if (vol > 30)
        mp3Data[1] = 30;
    else
        mp3Data[1] = vol;
    MP3_sendCommand(0x06, 2, mp3Data); // set volumne
}
void MP3_SingleCyclePlay(uint8_t songIdx)
{
    mp3Data[0] = 0x00;
    mp3Data[1] = songIdx;
    MP3_sendCommand(0x08, 2, mp3Data); // single cycle play song;
}
void MP3_SleepMode()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x0A, 2, mp3Data); // sleep;
}
void MP3_Wakeup()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x0B, 2, mp3Data); // wake up;
}
void MP3_Play()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x0D, 2, mp3Data); // play;
}
void MP3_Pause()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x0E, 2, mp3Data); // Pause;
}
void MP3_PlayWithFolderFilename(uint8_t folderId, uint8_t fileId)
{
    mp3Data[0] = folderId;
    mp3Data[1] = fileId;
    MP3_sendCommand(0x0F, 2, mp3Data); // Play Folder filename;
}
void MP3_Stop()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x16, 2, mp3Data); // stop;
}
void MP3_CyclePlayFolder(uint8_t folderId)
{
    mp3Data[0] = 0x00;
    mp3Data[1] = folderId;
    MP3_sendCommand(0x17, 2, mp3Data); // cycle play folder;
}
void MP3_ShufflePlay()
{
    mp3Data[0] = 0x00;
    mp3Data[1] = 0x00;
    MP3_sendCommand(0x18, 2, mp3Data); // shuffle play;
}
void MP3_GroupPlay(int cnt, uint8_t data[])
{
    // not working
    MP3_sendCommand(0x21, cnt, data);
}
void MP3_PlayWithVolumeIndex(uint8_t vol, uint8_t songIdx)
{
    // not working
    mp3Data[0] = vol;
    mp3Data[1] = songIdx;
    MP3_sendCommand(0x22, 2, mp3Data); // play with volume;
}
void MP3_sendCommand(uint8_t cmd, uint8_t dataLen, uint8_t data[])
{
    uint8_t dataIdx = 5;
    mp3Command[0] = MP3_START_CMD;
    mp3Command[1] = MP3_VERS;
    mp3Command[3] = cmd;
    mp3Command[4] = 0x00; // feedback always 0;
    for (int i = 0; i < dataLen; i++)
    {
        mp3Command[dataIdx] = data[i];
        dataIdx++;
    }
    mp3Command[2] = dataIdx - 1; // size of data
    mp3Command[dataIdx] = MP3_END_CMD;
    dataIdx++;

    for (int i = 0; i < dataIdx; i++)
    {
        mp3Serial.write(mp3Command[i]);
    }
}

String decodeMP3Answer()
{
    String decodedMP3Answer = "";

    decodedMP3Answer += sanswer();

    switch (ansbuf[3])
    {
    case 0x3A:
        decodedMP3Answer += " -> Memory card inserted.";
        break;

    case 0x3D:
        decodedMP3Answer += " -> Completed play num " + String(ansbuf[6], DEC);
        break;

    case 0x40:
        decodedMP3Answer += " -> Error";
        break;

    case 0x41:
        decodedMP3Answer += " -> Data recived correctly. ";
        break;

    case 0x42:
        decodedMP3Answer += " -> Status playing: " + String(ansbuf[6], DEC);
        break;

    case 0x48:
        decodedMP3Answer += " -> File count: " + String(ansbuf[6], DEC);
        break;

    case 0x4C:
        decodedMP3Answer += " -> Playing: " + String(ansbuf[6], DEC);
        break;

    case 0x4E:
        decodedMP3Answer += " -> Folder file count: " + String(ansbuf[6], DEC);
        break;

    case 0x4F:
        decodedMP3Answer += " -> Folder count: " + String(ansbuf[6], DEC);
        break;
    }

    return decodedMP3Answer;
}

String sbyte2hex(uint8_t b)
{
    String shex;

    shex = "0X";

    if (b < 16)
        shex += "0";
    shex += String(b, HEX);
    shex += " ";
    return shex;
}

String sanswer(void)
{
    uint8_t i = 0;
    String mp3answer = "";

    // Get only 10 Bytes
    while (mp3Serial.available() && (i < 10))
    {
        uint8_t b = mp3Serial.read();
        ansbuf[i] = b;
        i++;

        mp3answer += sbyte2hex(b);
    }

    // if the answer format is correct.
    if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF))
    {
        return mp3answer;
    }

    return "???: " + mp3answer;
}
