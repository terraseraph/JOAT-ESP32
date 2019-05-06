
//Prototypes
bool processPhoneLoop();
void phone_init();
void playPhoneTone();
void stopOffHookTone();
void playOffHookTone();
void playCallingTone();

#define HOOK_SWITCH 32

int PHONE_KEYPAD_DIGITS = 10;
bool enableTones = true;
bool playingHookTone = false;
bool onHook = false;

const byte PHONE_KEYPAD_ROWS = 4;
const byte PHONE_KEYPAD_COLS = 3;
char phoneKeys[PHONE_KEYPAD_ROWS][PHONE_KEYPAD_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
byte PHONE_KEYPAD_ROW_PINS[] = {LEFT_PIN1, LEFT_PIN2, LEFT_PIN3, LEFT_PIN4};
byte PHONE_KEYPAD_COL_PINS[] = {LEFT_PIN5, LEFT_PIN6, LEFT_PIN7};

void phone_init()
{

    pinMode(HOOK_SWITCH, INPUT_PULLUP);

    KEYPAD_DIGITS = PHONE_KEYPAD_DIGITS;
    keypad_custom_init(makeKeymap(phoneKeys), PHONE_KEYPAD_ROW_PINS, PHONE_KEYPAD_COL_PINS, sizeof(PHONE_KEYPAD_ROW_PINS), sizeof(PHONE_KEYPAD_COL_PINS));
    // _keypad = new Keypad(makeKeymap(phoneKeys), PHONE_KEYPAD_ROW_PINS, PHONE_KEYPAD_COL_PINS, sizeof(PHONE_KEYPAD_ROW_PINS), sizeof(PHONE_KEYPAD_COL_PINS));
    keypad_setDebounceTime(150);
    // ledcSetup(channel, freq, resolution);
    // ledcAttachPin(BUZZER_PIN, channel);
    MP3_init();
}

bool processPhoneLoop()
{
    // if (ProcessKeyPad())
    // {
    //     playPhoneTone();
    // };
    processMp3Loop();

    // Process hook magnet
    if (digitalRead(HOOK_SWITCH) == LOW)
    {
        if (onHook)
        { //if the phone was just picked up
            playOffHookTone();
            Serial.println("==== Phone off hook ====");
        }
        onHook = false;
        if (ProcessKeyPad())
        {
            playPhoneTone();
        };
        if (keypad_getDigitCount() == PHONE_KEYPAD_DIGITS)
        {
            playCallingTone();
        }

        // if (!playingHookTone)
        // {
        //     if (onHook)
        //     {
        //         Serial.println("==== Phone off hook ====");
        //         playOffHookTone();
        //         playingHookTone = true;
        //         onHook = false;
        //         keypad_enableProcessLoop();
        //         enableTones = true;
        //     }
        // }
    }
    else
    {
        if (!onHook)
        {
            Serial.println("==== Phone on hook ====");
            keypad_resetDigits();
            stopOffHookTone();
            onHook = true;
        }
        //         Serial.println("==== Phone on hook ====");
        //         keypad_disableProcessLoop();
        //         stopOffHookTone();
        //         playingHookTone = false;
        //         enableTones = false;
        //         onHook = true;
        //     }
    }
}

void processPhoneAction(String action, uint8_t folderId, uint8_t fileId)
{
    processMp3Action(action, folderId, fileId);
}

void playPhoneTone()
{
    if (!enableTones)
    {
        return;
    }
    processMp3Action("play", 1, keypad_pressed_key);
    Serial.print("Playing : ");
    Serial.println(keypad_pressed_key);
}

void playOffHookTone()
{
    if (!enableTones)
    {
        return;
    }
    processMp3Action("play", 1, 97);
}

void stopOffHookTone()
{
    processMp3Action("stop", 0, 0);
}

void playCallingTone()
{
    if (!enableTones)
    {
        return;
    }
    processMp3Action("play", 1, 96);
}
