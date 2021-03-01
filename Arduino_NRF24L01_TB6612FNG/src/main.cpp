#include <Arduino.h>
#include <SPI.h>
#include "MOTORS.h"
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);
const uint64_t pipe_address = 0xc3de775240; 

#define AIN1 3
#define BIN1 7
#define AIN2 4
#define BIN2 8
#define PWMA 5
#define PWMB 6
#define STBY A0

#define CENTRE_MAX 10
#define CENTRE_MIN -10

const int offsetL = 1; //motorL direction offset
const int offsetR = 1; //motorR direction offset

Motor motorL = Motor(AIN1, AIN2, PWMA, offsetL, STBY);
Motor motorR = Motor(BIN1, BIN2, PWMB, offsetR, STBY);

RF24 radio(9,10);
const uint64_t pipe_address = 0xc3de775240; 


char* Data_Received = NULL;
void Process_Data(char*);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  radio.begin();           
  radio.openReadingPipe(1, pipe_address);
  radio.startListening();
}

void loop() {
  // put your main code here, to run repeatedly:
  while(radio.available())
  {
    radio.read(Data_Received, sizeof(Data_Received));
    Process_Data(Data_Received);
  }
}
void Process_Data(char* Data)
{
  char* tokTemp = NULL;
  char* tokenString = Data;
  int Roll = atoi(strtok_r(tokenString, "|", &tokTemp));
  int Pitch = atoi(strtok_r(NULL, "^", &tokTemp));
  int L_Speed = DEFAULTSPEED, R_Speed = DEFAULTSPEED;
  if(Pitch>CENTRE_MAX)
  {
    // Increasing Y : Forward
    L_Speed = map(Pitch,CENTRE_MAX,90,CENTRE_MAX,255);
    R_Speed = map(Pitch,CENTRE_MAX,90,CENTRE_MAX,255);
  }
  else if (Pitch < CENTRE_MIN) {
    // Increasing Y : Backward
    L_Speed = map(Pitch,CENTRE_MIN,-90,CENTRE_MIN,-255);
    R_Speed = map(Pitch,CENTRE_MIN,-90,CENTRE_MIN,-255);
  }
  else {
    // Buffer Zone : Stop
    L_Speed  = 0;
    R_Speed  = 0;
  }

  if(Roll>CENTRE_MAX)
  {
    // Increasing X : Right--> L_Speed+=X_Mapping and R_speed-=X_Mapping
    int X_Mapping =map(Roll,CENTRE_MAX,90,CENTRE_MAX,255);
    L_Speed += X_Mapping;
    R_Speed -= X_Mapping;
  }
  if(Roll < CENTRE_MIN) {
    // Decreasing X : Left--> L_Speed-=X_Mapping and R_speed+=X_Mapping
    int X_Mapping =map(Roll,CENTRE_MIN,-90,CENTRE_MAX,255);
    L_Speed -= X_Mapping;
    R_Speed += X_Mapping;
  }
  constrain(L_Speed,0,255);
  constrain(R_Speed,0,255);

  motorL.drive(L_Speed);
  motorR.drive(R_Speed);
}