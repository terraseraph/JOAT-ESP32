
#include <Key.h>    // keypad
#include <Keypad.h> // keypad

////////////////////////////
// keypad
///////////////////////////
#define LEFT_PIN1 15 // Left pin on keyp pad pin1
#define LEFT_PIN2 2
#define LEFT_PIN3 0
#define LEFT_PIN4 4
#define LEFT_PIN5 27
#define LEFT_PIN6 14
#define LEFT_PIN7 12
#define LEFT_PIN8 13

#define KEYPAD_TIMEOUT 20000

/////////////////////////////////
// BUZZER
/////////////////////////////////
#define BUZZER_PIN 6

//Prototypes
bool ProcessKeyPad();
void keypad_init();

int KEYPAD_DIGITS = 6; //Amount of digits till it sends packet
int keypad_timeout;
int keypad_last_keypress;
uint8_t digit_count = 0;
uint8_t keypad_digits[11]; // number of digits to send, i.e pin number
bool playTimeoutSound = false;

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

  memset(keypad_digits, 0, sizeof(keypad_digits));
}

bool ProcessKeyPad()
{
  if ((millis() - keypad_last_keypress) > KEYPAD_TIMEOUT)
  {
    // reset keys pressed
    digit_count = 0;
    memset(keypad_digits, 0, sizeof(keypad_digits));
    if(playTimeoutSound){
        //  tone(BUZZER_PIN, 1000);
        //  delay(100); //Schedule these
        //  noTone(BUZZER_PIN);
         playTimeoutSound = false;
    }
  }

  char key = _keypad->getKey();
  if (key != NULL)
  {
    keypad_last_keypress = millis();
    keypad_digits[digit_count] = key;
    Serial.print("Key Pad digit buffer: ");
    Serial.println((char *)keypad_digits);
    digit_count++;
    playTimeoutSound = true;

    if (digit_count == KEYPAD_DIGITS)
    {
      //strcpy((char*) sentMessage[sentMessageHead].packet.data, (const char*)keypad_digits);
      createJsonPacket(MY_ID, "code", "keypad", "noneA", "noneAT", (char *)keypad_digits);
      // clear the pad
      digit_count = 0;
      memset(keypad_digits, 0, sizeof(keypad_digits));
      playTimeoutSound = false;
      //      tone(BUZZER_PIN, 1000);
      //      delay(100); //Schedule these
      //      noTone(BUZZER_PIN);
    }
    else
    {
      //      tone(BUZZER_PIN, 500);
      //      delay(100); //Schedule these
      //      noTone(BUZZER_PIN);
    }
    return true;
  }
  return false;
}
