
/////////////////////////////
// Magnet switch pins
/////////////////////////////
#define MAG_PIN0 15
#define MAG_PIN1 2
#define MAG_PIN2 0
#define MAG_PIN3 4
#define MAG_PIN4 16
#define MAG_PIN5 17
#define MAG_PIN6 5
#define MAG_PIN7 18
#define MAG_PIN8 19
#define MAG_PIN9 21

String intToStringArray(int int_array[], int size_of_array);
bool SWITCH_STATE[] = {false, false, false, false, false, false, false, false, false, false};

void magSwitch_init() {
  pinMode(MAG_PIN0, INPUT_PULLUP);
  pinMode(MAG_PIN1, INPUT_PULLUP);
  pinMode(MAG_PIN2, INPUT_PULLUP);
  pinMode(MAG_PIN3, INPUT_PULLUP);
  pinMode(MAG_PIN4, INPUT_PULLUP);
  pinMode(MAG_PIN5, INPUT_PULLUP);
  pinMode(MAG_PIN6, INPUT_PULLUP);
  pinMode(MAG_PIN7, INPUT_PULLUP);
  pinMode(MAG_PIN8, INPUT_PULLUP);
  pinMode(MAG_PIN9, INPUT_PULLUP);

  NODE_TYPE = "magSwitch";
}


void processMagSwitchEvent() {
    int magnet0 = digitalRead(MAG_PIN0);
    int magnet1 = digitalRead(MAG_PIN1);
    int magnet2 = digitalRead(MAG_PIN2);
    int magnet3 = digitalRead(MAG_PIN3);
    int magnet4 = digitalRead(MAG_PIN4);
    int magnet5 = digitalRead(MAG_PIN5);
    int magnet6 = digitalRead(MAG_PIN6);
    int magnet7 = digitalRead(MAG_PIN7);
    int magnet8 = digitalRead(MAG_PIN8);
    int magnet9 = digitalRead(MAG_PIN9);
    int switchNo = -1;

    // LOW is on
    if (magnet0 == LOW && !SWITCH_STATE[0] || magnet0 == HIGH && SWITCH_STATE[0])  switchNo = 0;
    else if(magnet1 == LOW && !SWITCH_STATE[1] || magnet1 == HIGH && SWITCH_STATE[1])  switchNo = 1;
    else if(magnet2 == LOW && !SWITCH_STATE[2] || magnet2 == HIGH && SWITCH_STATE[2])  switchNo = 2;
    else if(magnet3 == LOW && !SWITCH_STATE[3] || magnet3 == HIGH && SWITCH_STATE[3])  switchNo = 3;
    else if(magnet4 == LOW && !SWITCH_STATE[4] || magnet4 == HIGH && SWITCH_STATE[4])  switchNo = 4;
    else if(magnet5 == LOW && !SWITCH_STATE[5] || magnet5 == HIGH && SWITCH_STATE[5])  switchNo = 5;
    else if(magnet6 == LOW && !SWITCH_STATE[6] || magnet6 == HIGH && SWITCH_STATE[6])  switchNo = 6;
    else if(magnet7 == LOW && !SWITCH_STATE[7] || magnet7 == HIGH && SWITCH_STATE[7])  switchNo = 7;
    else if(magnet8 == LOW && !SWITCH_STATE[8] || magnet8 == HIGH && SWITCH_STATE[8])  switchNo = 8;
    else if(magnet9 == LOW && !SWITCH_STATE[9] || magnet9 == HIGH && SWITCH_STATE[9])  switchNo = 9;
      
    if (switchNo >= 0)
    {
        Serial.printf("Magnet Switch Event: ");

        SWITCH_STATE[0] = magnet0 == LOW;
        SWITCH_STATE[1] = magnet1 == LOW;
        SWITCH_STATE[2] = magnet2 == LOW;
        SWITCH_STATE[3] = magnet3 == LOW;
        SWITCH_STATE[4] = magnet4 == LOW;
        SWITCH_STATE[5] = magnet5 == LOW;
        SWITCH_STATE[6] = magnet6 == LOW;
        SWITCH_STATE[7] = magnet7 == LOW;
        SWITCH_STATE[8] = magnet8 == LOW;
        SWITCH_STATE[9] = magnet9 == LOW;

        int int_data[10] = {0};
        String str_data = intToStringArray(int_data, 10);
        //prepare packet for magnet
        if (magnet0 == LOW) int_data[0] = 1;
        if (magnet1 == LOW) int_data[1] = 1;
        if (magnet2 == LOW) int_data[2] = 1;
        if (magnet3 == LOW) int_data[3] = 1;
        if (magnet4 == LOW) int_data[4] = 1;
        if (magnet5 == LOW) int_data[5] = 1;
        if (magnet6 == LOW) int_data[6] = 1;
        if (magnet7 == LOW) int_data[7] = 1;
        if (magnet8 == LOW) int_data[8] = 1;
        if (magnet9 == LOW) int_data[9] = 1;
        createJsonPacket(MY_ID, "toggle", "magnetSwitch", "noneA", "noneAT", str_data);
    }
  }


String intToStringArray(int int_array[], int size_of_array) {
  String returnstring = "";
  for (int temp = 0; temp < size_of_array; temp++){
    returnstring += String(int_array[temp]);
//    returnstring += itoa(int_array[temp]);
    if(temp < size_of_array - 1){
      returnstring += ",";
    }
  }
  return "[" + returnstring + "]";
}
