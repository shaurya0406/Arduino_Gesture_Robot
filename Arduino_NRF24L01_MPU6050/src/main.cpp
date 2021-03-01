#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "nRF24L01.h"
#include "RF24.h"

#define MPU 0x68 
#define Enable_Button 2

RF24 radio(9,10);
const uint64_t pipe_address = 0xc3de775240; 

double AcX,AcY,AcZ;
int Data_Enable = 0;

void Init_MPU();
void Read_MPU();
int Calculate_PitchRoll(double, double, double);
char* Prepare_Data();
void ISR_Flag();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(Enable_Button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Enable_Button), ISR_Flag, CHANGE);
  Init_MPU(); 
  radio.begin();    
  radio.openWritingPipe(pipe_address); 
}

void loop() {
  // put your main code here, to run repeatedly:
  Read_MPU();
  if(Data_Enable)
  {
    char* Prepared_Data = Prepare_Data();
    radio.write(Prepared_Data, sizeof(Prepared_Data));
    Data_Enable = 0;
  }
}
void Init_MPU(){
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  
  Wire.write(0);     
  Wire.endTransmission(true);
  delay(1000);
}
void Read_MPU(){
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,6,true);  
  AcX=Wire.read()<<8|Wire.read();       
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read();  
}
int Calculate_PitchRoll(double A, double B, double C){
  double DataA, DataB, Value;
  DataA = A;
  DataB = (B*B) + (C*C);
  DataB = sqrt(DataB);
  
  Value = atan2(DataA, DataB);
  Value = Value * 180/3.14;
  
  return (int)Value; // [-90 to 90]
}
char* Prepare_Data()
{
  char * Temp_String = (char*)malloc(100 * sizeof(char));
  int Roll = Calculate_PitchRoll(AcX, AcY, AcZ);   
  int Pitch = Calculate_PitchRoll(AcY, AcX, AcZ);  
  sprintf(Temp_String, "%d|%d^", Roll, Pitch);
  return Temp_String;
}
void ISR_Flag()
{
  Data_Enable = !Data_Enable;
}