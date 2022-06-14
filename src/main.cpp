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
char batteryVoltage[16];
char resp[256];
uint8_t messageIndex;

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

/*
Token 0: AT+CBC
+CBC: 0
Token 1: 88
Token 2: 4109

OK
*/
void getVbat(char *vbat){
  const char s[2] = ",";
  char buffer[256];
  char *token;
  int tokenCounter = 0;

  sendCommand(buffer,CHECK_BATTERY_CMD);
  token = strtok(buffer, s);

  while( token != NULL ) {
    debugPort.printf( "Token %d: %s\n",tokenCounter, token );
    if(tokenCounter == 2)
      strcpy(vbat,token);

    token = strtok(NULL, s);
    tokenCounter++;
  }


}

void sendSms(char *number, char *msg){
  char numberCommand[64];
  char msgCommand[128];

  
  sendCommand(gsmBuffer,TEXT_MODE_CMD);
  debugPort.print(gsmBuffer);
  sprintf(numberCommand,"AT+CMGS=\"%s\"\r\n",number);
  sendCommand(gsmBuffer,numberCommand);
  debugPort.print(gsmBuffer);

  sprintf(msgCommand,"%s%c",msg,0x1A);
  sendCommand(gsmBuffer,msgCommand);
  debugPort.print(gsmBuffer);
  delay(500);
  Serial.println();
  delay(500);

  

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

// +CMTI: "SM",9
uint8_t checkMsg(){
  char buffer[64] = "";
  char num[8] = "";
  int i = 0;
  const char s[2] = ",";
  char *token;
  int tokenCounter = 0;
  uint8_t msgIndex = 0;

  debugPort.println("Check message");
  while(Serial.available()){
    buffer[i] = Serial.read();
    i++;
  }

  if(strstr(buffer,"+CMTI")){
    //debugPort.printf("Got Message:\n %s",buffer);

    token = strtok(buffer, s);

    while( token != NULL ) {
      //debugPort.printf( "Token %d: %s\n",tokenCounter, token );
      if(tokenCounter == 1)
        strcpy(num,token);

      token = strtok(NULL, s);
      tokenCounter++;
    }
    msgIndex = atoi(num);
     
  }

  return msgIndex;
}


void setup() {
  // put your setup code here, to run once:
  debugPort.begin(57600);
  Serial.begin(57600);
  initSim800();
  messageIndex = 0;
  //sendSms("+447747465192", "Testing");

}

void loop() {

  messageIndex = checkMsg();
  debugPort.printf("messageIndex: %d\n",messageIndex);
  if(messageIndex > 0){
    processMessage(recipientNumber,message,messageIndex);
    debugPort.printf("Number : %s\nMessage : %s\n",recipientNumber,message);
    
    getVbat(batteryVoltage);
    //sprintf(resp,"Got: %s, vbat: %s",message,batteryVoltage);
    sendSms(recipientNumber,batteryVoltage);
    
  }
  delay(5000);
  //delay(5000);
  //sendSms("+447747465192", "Testing");
  //delay(30000);

  /*
  sendCommand(gsmBuffer,"AT+CMGR=1\r\n");
  debugPort.print(gsmBuffer);
  sendCommand(gsmBuffer,CHECK_BATTERY_CMD);
  debugPort.print(gsmBuffer);
  */

 
  /*
  processMessage(recipientNumber,message,1);
  debugPort.printf("Number:%s\n",recipientNumber);
  debugPort.printf("Message: %s",message);
  delay(2000);
  */
  


  /*  
  if (Serial.available()) {      // If anything comes in Serial (USB);,	
    debugPort.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)

  }

  if (debugPort.available()) {     // If anything comes in Serial1 (pins 0 & 1)	
    Serial.write(debugPort.read());   // read it and send it out Serial (USB)
  }
  */
}