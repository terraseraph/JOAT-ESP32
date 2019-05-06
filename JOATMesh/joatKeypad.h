
#include <Key.h>    // keypad
#include <Keypad.h> // keypad

////////////////////////////
// keypad
///////////////////////////
#define LEFT_PIN1 15 // Left pin on keyp pad pin1
#define LEFT_PIN2 2
#define LEFT_PIN3 4
#define LEFT_PIN4 13
#define LEFT_PIN5 12
#define LEFT_PIN6 14
#define LEFT_PIN7 27
#define LEFT_PIN8 33

#define KEYPAD_TIMEOUT 20000

/////////////////////////////////
// BUZZER
/////////////////////////////////
#define BUZZER_PIN 35

//Prototypes
bool ProcessKeyPad();
void keypad_init();
void keypad_custom_init();
void setPressedKey(char key);
int getKeypadDigits();
void setKeypadDigits(int digits);
void keypad_enableProcessLoop();
void keypad_disableProcessLoop();
void keypad_setDebounceTime(uint time);
void keypad_resetDigits();
uint8_t keypad_getDigitCount();

int KEYPAD_DIGITS = 6; //Amount of digits till it sends packet
int keypad_timeout;
int keypad_last_keypress;
uint8_t keypad_pressed_key;
uint8_t digit_count = 0;
uint8_t keypad_digits[11]; // number of digits to send, i.e pin number
bool playTimeoutSound = false;
bool enableKeypadProcess = true;

//Buzzer
int freq = 2000;
int channel = 0;
int resolution = 8;
bool buzzerOff = true;
long buzzerDebounce;

void processBuzzer()
{
  // Serial.println("== Processing buzzer ===");
  if ((millis() - buzzerDebounce) < 256) //if 256 milliseconds has passed since last bounce
  {
    buzzerOff = false;
    return; //read value again now that bouncing is over
  }
  else
  {
    if (!buzzerOff)
    {
      ledcWrite(channel, 0);
    }
    // ledcWriteTone(channel, 0);
    // ledcWriteNote(20, NOTE_A, 9);
  }
}

const uint8_t KEYPAD_ROWS = 4;
const uint8_t KEYPAD_COLS = 4;
char keypadKeys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
uint8_t KEYPAD_ROW_PINS[] = {LEFT_PIN1, LEFT_PIN2, LEFT_PIN3, LEFT_PIN4};
uint8_t KEYPAD_COL_PINS[] = {LEFT_PIN5, LEFT_PIN6, LEFT_PIN7, LEFT_PIN8};

const uint16_t BUZZER_SOUNDS[][4] = {
    {1000, 100, 500, 100} // Hz and duration, repeat
    ,
    {2000, 500, 1000, 200} // Hz and duration, repeat
};
Keypad *_keypad = NULL;

void keypad_init()
{
  Serial.println("Keypad Init");
  _keypad = new Keypad(makeKeymap(keypadKeys), KEYPAD_ROW_PINS, KEYPAD_COL_PINS, KEYPAD_ROWS, KEYPAD_COLS);
  NODE_TYPE = "keypad";
  keypad_setDebounceTime(150);

  memset(keypad_digits, 0, sizeof(keypad_digits));

  // ==== Buzzer init ==== //
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(BUZZER_PIN, channel);
}

void keypad_custom_init(char *keymap, byte *rowPins, byte *colPins, byte numberOfRows, byte numberOfCols)
{
  Serial.println("Custom Keypad Init");
  _keypad = new Keypad(keymap, rowPins, colPins, numberOfRows, numberOfCols);
  keypad_setDebounceTime(150);
  memset(keypad_digits, 0, sizeof(keypad_digits));

  // ==== Buzzer init ==== //
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(BUZZER_PIN, channel);
}

bool ProcessKeyPad()
{
  char key = _keypad->getKey();
  processBuzzer();

  if (!enableKeypadProcess)
  {
    return false;
  }
  if (key != NULL)
  {
    Serial.println("== Key not null ===");
    keypad_last_keypress = millis();
    keypad_digits[digit_count] = key;
    Serial.println("== setting pressed key ===");
    setPressedKey(key);
    Serial.print("Key Pad digit buffer: ");
    Serial.println((char *)keypad_digits);
    digit_count++;
    playTimeoutSound = true;

    if (digit_count == KEYPAD_DIGITS || key == '#')
    {
      Serial.println("== sending packet ===");
      state_createAndSendPacket(MY_ID, "event", "code", "keypad", "noneA", "noneAT", (char *)keypad_digits);
      // clear the pad
      keypad_resetDigits();
      playTimeoutSound = false;
      // ledcWriteTone(channel, 3000);
      ledcWriteNote(channel, NOTE_A, 3);
      buzzerDebounce = millis();
    }
    else
    {
      // ledcWriteTone(channel, 1000);
      ledcWriteNote(channel, NOTE_A, 1);
      buzzerDebounce = millis();
    }
    return true;
  }
  if ((millis() - keypad_last_keypress) > KEYPAD_TIMEOUT)
  {
    // Serial.println("== timeout reached ===");
    // reset keys pressed
    digit_count = 0;
    memset(keypad_digits, 0, sizeof(keypad_digits));
    if (playTimeoutSound)
    {
      // ledcWriteTone(channel, 5000);
      Serial.println("== playing timeout sound ===");
      ledcWriteNote(channel, NOTE_A, 6);
      buzzerDebounce = millis();
      playTimeoutSound = false;
    }
  }
  return false;
}

void setKeypadDigits(int digits)
{
  KEYPAD_DIGITS = digits;
}

int getKeypadDigits()
{
  return KEYPAD_DIGITS;
}

void setPressedKey(char loKey)
{
  int iKey = loKey - 48;
  if (loKey == '*')
  {
    iKey = 99;
  }
  else if (loKey == '#')
  {
    iKey = 98;
  }
  keypad_pressed_key = iKey;
}

void keypad_enableProcessLoop()
{
  Serial.println("== enable keypad loop ===");
  enableKeypadProcess = true;
}

void keypad_disableProcessLoop()
{
  Serial.println("== Disable keypad loop ===");
  enableKeypadProcess = false;
  memset(keypad_digits, 0, sizeof(keypad_digits));
}

void keypad_resetDigits()
{
  digit_count = 0;
  memset(keypad_digits, 0, sizeof(keypad_digits));
}

void keypad_setDebounceTime(uint time)
{
  _keypad->setDebounceTime(time);
}

uint8_t keypad_getDigitCount()
{
  return digit_count;
}
