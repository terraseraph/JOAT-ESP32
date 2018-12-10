int addr = 0;
#define EEPROM_SIZE 64


void setNoteType(String type){
  int val = byte(random(10020));
  EEPROM.write(0, val);
}

void getNodeType(){
  Serial.print(byte(EEPROM.read(0))); Serial.print(" ");
}
