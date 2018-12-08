
/////////////////////////////
// button pins
/////////////////////////////
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

unsigned long btnLastDebounceTime=millis();

void button_init() {
  pinMode(BTN_PIN0, INPUT_PULLUP);
  pinMode(BTN_PIN1, INPUT_PULLUP);
  pinMode(BTN_PIN2, INPUT_PULLUP);
  pinMode(BTN_PIN3, INPUT_PULLUP);
  pinMode(BTN_PIN4, INPUT_PULLUP);
  pinMode(BTN_PIN5, INPUT_PULLUP);
  pinMode(BTN_PIN6, INPUT_PULLUP);
  pinMode(BTN_PIN7, INPUT_PULLUP);
  pinMode(BTN_PIN8, INPUT_PULLUP);
  pinMode(BTN_PIN9, INPUT_PULLUP);
}



void processButtonEvent()
{
    int button0 = digitalRead(BTN_PIN0);
    int button1 = digitalRead(BTN_PIN1);
    int button2 = digitalRead(BTN_PIN2);
    int button3 = digitalRead(BTN_PIN3);
    int button4 = digitalRead(BTN_PIN4);
    int button5 = digitalRead(BTN_PIN5);
    int button6 = digitalRead(BTN_PIN6);
    int button7 = digitalRead(BTN_PIN7);
    int button8 = digitalRead(BTN_PIN8);
    int button9 = digitalRead(BTN_PIN9);
    uint16_t buttonNo = 0;

    
    if (button0 == LOW || button1 == LOW || button2 == LOW || button3 == LOW || button4 == LOW || button5 == LOW || button6 == LOW || button7 == LOW || button8 == LOW || button9 == LOW){
      if ((millis()-btnLastDebounceTime) < 125)  //if 50 milliseconds has passed since last bounce
      {
           btnLastDebounceTime=millis();
           return;  //read value again now that bouncing is over
      }    
      btnLastDebounceTime=millis();
  
      
  
      //prepare message to be sent
      if(button0 == LOW) buttonNo = 0;
      if(button1 == LOW) buttonNo = 1;
      if(button2 == LOW) buttonNo = 2;
      if(button3 == LOW) buttonNo = 3;
      if(button4 == LOW) buttonNo = 4;
      if(button5 == LOW) buttonNo = 5;
      if(button6 == LOW) buttonNo = 6;
      if(button7 == LOW) buttonNo = 7;
      if(button8 == LOW) buttonNo = 8;
      if(button9 == LOW) buttonNo = 9;

      String data = String(buttonNo);

      Serial.print("Button Event: ");
      Serial.println(data);
      createJsonPacket(MY_ID, "toggle", "button", "noneA", "noneAT", data);
    }  
}
