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

MSG_CONTENTS message;
RESPONSE response;

String _readSerial(uint32_t timeout)
{

    uint64_t timeOld = millis();


    while (!Serial.available() && !(millis() > timeOld + timeout))
    {
        delay(13);
    }

    String str;

    while(Serial.available())
    {
        if (Serial.available()>0)
        {
            str += (char) Serial.read();
        }
    }

    return str;

}

String sendCommandSimple(String cmd){
  Serial.print(cmd);
  delay(500);
  return _readSerial(1000);
}

int sendCommand(char *buffer, char *cmd){
  int charCounter=0,timeoutCounter=0,rxComplete=0;
  int timeout = 10;

  Serial.print(cmd);

  while(rxComplete == 0){
    while(Serial.available()){
      timeoutCounter = 0;
      buffer[charCounter] = Serial.read();
      charCounter ++;
    }

    timeoutCounter ++;
    delay(50);
    if(timeoutCounter >= timeout)
      rxComplete = 1;
  }
  buffer[charCounter] = '\0';
  return charCounter;
}

/*
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
}*/


void printByte(char *buf){
  //debugPort.println(strlen(buf));
  for(int i = 0; i < strlen(buf); i++){
    debugPort.write(buf[i]);
  }
}
int i;
char gsmBuffer[256];

void setup() {
  // put your setup code here, to run once:
  debugPort.begin(57600);
  Serial.begin(57600);
  digitalWrite (2, HIGH);  // enable pull-up
  wokenUp = false;
  i =0;
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
  debugPort.println("Ready");
  i=0;

}

void loop() {
  // put your main code here, to run repeatedly:
  
  //debugPort.println(checkForSms());
  //delay(1000);
  //debugPort.println(i);
  //debugPort.print(sendCommand("AT+CMGR=1\r\n"));
  //debugPort.flush();

  debugPort.printf("Cycle %d\n",i);
  i++;
  sendCommand(gsmBuffer,"AT+CMGR=1\r\n");
  debugPort.print(gsmBuffer);
  sendCommand(gsmBuffer,CHECK_BATTERY_CMD);
  debugPort.print(gsmBuffer);
  delay(1000);
  
  /*
  if (Serial.available()) {      // If anything comes in Serial (USB);,	
    debugPort.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)

  }

  if (debugPort.available()) {     // If anything comes in Serial1 (pins 0 & 1)	
    Serial.write(debugPort.read());   // read it and send it out Serial (USB)
  }*/
}