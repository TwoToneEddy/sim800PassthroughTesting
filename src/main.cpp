#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial debugPort(6, 7);
bool wokenUp;

#define AUTO_BAUD_CMD       "AT\r\n"
#define TEXT_MODE_CMD       "AT+CMGF=1\r\n"
#define WAKE_CMD            "AT+CSCLK=0\r\n"
#define SLEEP_CMD            "AT+CSCLK=2\r\n"
#define DELETE_MSGS_CMD     "AT+CMGD=2,4\r\n"
#define CHECK_BATTERY_CMD   "AT+CBC\r\n"

struct MSG_CONTENTS{
    String senderNumber;
    String message;
};

struct RESPONSE{
            String raw;
            String lines[10];
            int size;
};

MSG_CONTENTS message;
RESPONSE response;



String sendCommand(String cmd){

  char buffer[256];
  int availableBytes;
  int i,j;
  Serial.print(cmd);
  delay(500);
  availableBytes = Serial.available();

  if(availableBytes < 1){
    delay(500);
    debugPort.println("No response, Trying again.");
    Serial.print(cmd);
    delay(500);
    availableBytes = Serial.available();
  }

  for(i=0; i<availableBytes; i++)
  {
      buffer[i] = Serial.read();
  }
  delay(100);
  availableBytes = Serial.available();

  for(j=i; j<availableBytes; i++)
  {
      buffer[j] = Serial.read();
  }
  delay(100);

  while(Serial.available()){
    Serial.read();
  }
  return buffer;
}






int getMostRecentMSGIndex(char inp[]){
    char buf[4];
    char catInt[4];
    String rxString = String(inp);
    rxString.toCharArray(buf,rxString.length());

    if(isdigit(buf[rxString.length()-4])){
        sprintf(catInt,"%c%c",buf[rxString.length()-4],buf[rxString.length()-3]);
    }else{
        sprintf(catInt,"%c",buf[rxString.length()-3]);
    }

    return atoi(catInt);
}

int checkForSms(){

  char buffer[256];
  char cmd[256];
  int smsIndex =0;
  MSG_CONTENTS message;
  int availableBytes= Serial.available();

  if(availableBytes>0){
    debugPort.println("Got something");

    for(int i=0; i<availableBytes; i++)
    {
        buffer[i] = Serial.read();
    }
    debugPort.print(buffer);

    if(strstr(buffer,"+CMTI:")!=NULL){
      debugPort.println("Found CMTI");
      smsIndex=getMostRecentMSGIndex(buffer);
      debugPort.println(smsIndex);
      sprintf(cmd,"AT+CMGR=%d\r\n",smsIndex);
      debugPort.print(sendCommand(cmd));
      delay(5000);
      debugPort.print(sendCommand(DELETE_MSGS_CMD));
    }
    else
      smsIndex = -1;
  }
  return smsIndex;
}

void setup() {
  // put your setup code here, to run once:
  debugPort.begin(57600);
  Serial.begin(57600);
  digitalWrite (2, HIGH);  // enable pull-up
  wokenUp = false;
  
  delay(1000);
  debugPort.print(sendCommand(AUTO_BAUD_CMD));
  debugPort.print(sendCommand(TEXT_MODE_CMD));
  /*
  debugPort.print(sendCommand(CHECK_BATTERY_CMD));
  debugPort.print(sendCommand(SLEEP_CMD));
  delay(1000);*/
  debugPort.println("Ready");

}

void loop() {
  // put your main code here, to run repeatedly:
  
  debugPort.println(checkForSms());
  delay(1000);

  /*
  if (Serial.available()) {      // If anything comes in Serial (USB),	
    debugPort.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)

  }

  if (debugPort.available()) {     // If anything comes in Serial1 (pins 0 & 1)	
    Serial.write(debugPort.read());   // read it and send it out Serial (USB)
  }*/
}