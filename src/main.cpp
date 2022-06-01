#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial debugPort(6, 7);
bool wokenUp;
#define SERIAL_RX_BUFFER_SIZE 64

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

//MSG_CONTENTS message;
RESPONSE response;
char gsmBuffer[256];
char recipientNumber[32];
char message[256];
int batteryVoltage;

int sendCommand(char *buffer, char *cmd){
  int charCounter=0,timeoutCounter=0,rxComplete=0;
  int timeout = 10;
  int attemptCounter = 0;

  Serial.print(cmd);

  while(rxComplete == 0 && attemptCounter < 2){
    while(Serial.available()){
      timeoutCounter = 0;
      buffer[charCounter] = Serial.read();
      charCounter ++;
    }

    timeoutCounter ++;
    delay(50);

    if(timeoutCounter >= timeout){
      if(charCounter > 0)
        rxComplete = 1;
      else{
        attemptCounter ++;
        timeoutCounter = 0;

        // Only attempt a second time
        if(attemptCounter == 1){
          debugPort.printf("Sending %s again\n",cmd);
          Serial.print(cmd);
        }
      }
    }

  }

  buffer[charCounter] = '\0';
  return charCounter;
}

void processMessage(char *number,char *msg, int msgNumber){
  char buffer[256];
  char command[24];
  const char s[2] = "\"";
  char *token;
  int tokenCounter = 0;

  sprintf(command,"AT+CMGR=%d\r\n",msgNumber);
  sendCommand(buffer,command);

  //debugPort.println("In processMessage()");
  token = strtok(buffer, s);

  while( token != NULL ) {
    //debugPort.printf( "Token %d: %s\n",tokenCounter, token );
    if(tokenCounter == 3)
      strcpy(number,token);
    if(tokenCounter == 7)
      strcpy(msg,&token[2]);

    token = strtok(NULL, s);
    tokenCounter++;
  }


}

void initSim800(){
  int i = 0;

  for(i = 0; i < 5; i++){
    debugPort.println("Waiting for SIM800");
    delay(1000);
  }
  debugPort.println("Sim800 wait over");
  sendCommand(gsmBuffer,AUTO_BAUD_CMD);
  debugPort.print(gsmBuffer);
  delay(1000);
  sendCommand(gsmBuffer,AUTO_BAUD_CMD);
  debugPort.print(gsmBuffer);
  delay(1000);
  sendCommand(gsmBuffer,TEXT_MODE_CMD);
  debugPort.print(gsmBuffer);
  delay(1000);
  sendCommand(gsmBuffer,CHECK_BATTERY_CMD);
  debugPort.print(gsmBuffer);
  delay(1000);
  sendCommand(gsmBuffer,WAKE_CMD);
  debugPort.print(gsmBuffer);
  delay(1000);
  debugPort.println("Ready");

}



void setup() {
  // put your setup code here, to run once:
  debugPort.begin(57600);
  Serial.begin(57600);
  initSim800();

}

void loop() {
 

  /*
  sendCommand(gsmBuffer,"AT+CMGR=1\r\n");
  debugPort.print(gsmBuffer);
  sendCommand(gsmBuffer,CHECK_BATTERY_CMD);
  debugPort.print(gsmBuffer);
  */

  processMessage(recipientNumber,message,1);
  debugPort.printf("Number:%s\n",recipientNumber);
  debugPort.printf("Message: %s",message);
  delay(2000);
  
  /*
  if (Serial.available()) {      // If anything comes in Serial (USB);,	
    debugPort.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)

  }

  if (debugPort.available()) {     // If anything comes in Serial1 (pins 0 & 1)	
    Serial.write(debugPort.read());   // read it and send it out Serial (USB)
  }*/
}