#include <Arduino.h>
#include "SoftwareSerial.h"
#include <LedControl.h>

SoftwareSerial serial_connection(4, 5);//Create a serial connection with TX and RX on these pins
#define BUFFER_SIZE 64//This will prevent buffer overruns.
char inData[BUFFER_SIZE];//This is a character buffer where the data sent by the python script will go.
char inChar = -1; //Initialie the first character as nothing
int count = 0; //This is the number of lines sent in from the python script
int i = 0; //Arduinos are not the most capable chips in the world so I just create the looping variable once

//Matrix Setup
int DIN = 2;
int CS = 6;
int CLK = 7;

//Facial Expression
byte smile[8]=   {0x3c, 0x42, 0x95, 0xa1, 0xa1, 0x95, 0x42, 0x3c};
byte neutral[8]= {0x3c, 0x42, 0x95, 0x91, 0x91, 0x95, 0x42, 0x3c};
byte sad[8]=   {0x3c, 0x42, 0xa5, 0x91, 0x91, 0xa5, 0x42, 0x3c};

LedControl lc=LedControl(DIN,CLK,CS,0);

//Motor Functions
void forward(int t) {

  serial_connection.println("Moving FORWARD");
  Serial.println("Moving FORWARD");

  //forward @ full speed
  digitalWrite(12, HIGH); //Establishes forward direction of Channel A
  digitalWrite(9, LOW); //Disengage the Brake for Channel A
  analogWrite(3, 255); //Spins the motor on Channel A at full speed

  digitalWrite(13, HIGH); //Establishes forward direction of Channel B
  digitalWrite(8, LOW); //Disengage the Brake for Channel B
  analogWrite(11, 255); //Spins the motor on Channel B at full speed

  delay(t);

  digitalWrite(9, HIGH); //Eengage the Brake for Channel A
  digitalWrite(8, HIGH); //Eengage the Brake for Channel B
}

void backward(int t) {

  serial_connection.println("Moving BACKWARD");

  //backward @ half speed
  digitalWrite(12, LOW); //Establishes backward direction of Channel A
  digitalWrite(9, LOW); //Disengage the Brake for Channel A
  analogWrite(3, 123); //Spins the motor on Channel A at half speed

  //backward @ half speed
  digitalWrite(13, LOW); //Establishes backward direction of Channel B
  digitalWrite(8, LOW); //Disengage the Brake for Channel B
  analogWrite(11, 123); //Spins the motor on Channel B at half speed

  delay(3000);

  digitalWrite(9, HIGH); //Eengage the Brake for Channel A
  digitalWrite(8, HIGH); //Eengage the Brake for Channel B
}

void printByte(byte character [])
{
  int i = 0;
  for(i=0;i<8;i++)
  {
    lc.setRow(0,i,character[i]);
  }
}

void setup()
{
  Serial.begin(9600);//Initialize communications to the serial monitor in the Arduino IDE
  serial_connection.begin(9600);//Initialize communications with the bluetooth module
  serial_connection.println("Ready!!!");//Send something to just start comms. This will never be seen.
  Serial.println("Started");//Tell the serial monitor that the sketch has started.

  //Motors setup

  //Setup Channel A
  pinMode(12, OUTPUT); //Initiates Motor Channel A pin
  pinMode(9, OUTPUT); //Initiates Brake Channel A pin

  //Setup Channel B
  pinMode(13, OUTPUT); //Initiates Motor Channel A pin
  pinMode(8, OUTPUT); //Initiates Brake Channel A pin

  //Matrix Setup
  lc.shutdown(0,false);
  lc.setIntensity(0,15);      //Adjust the brightness maximum is 15
  lc.clearDisplay(0);
}

void loop()
{
  //Set smile as default
  printByte(neutral);

  //This will prevent bufferoverrun errors
  byte byte_count = serial_connection.available(); //This gets the number of bytes that were sent by the python script
  if (byte_count) //If there are any bytes then deal with them
  {
    Serial.println("Incoming Data");//Signal to the monitor that something is happening
    int first_bytes = byte_count; //initialize the number of bytes that we might handle.
    int remaining_bytes = 0; //Initialize the bytes that we may have to burn off to prevent a buffer overrun
    if (first_bytes >= BUFFER_SIZE - 1) //If the incoming byte count is more than our buffer...
    {
      remaining_bytes = byte_count - (BUFFER_SIZE - 1); //Reduce the bytes that we plan on handleing to below the buffer size
    }
    for (i = 0; i < first_bytes; i++) //Handle the number of incoming bytes
    {
      inChar = serial_connection.read(); //Read one byte
      inData[i] = inChar; //Put it into a character string(array)
    }
    inData[i] = '\0'; //This ends the character array with a null character. This signals the end of a string
    if (String(inData) == "FORWARD") //This could be any motor start string we choose from the python script
    {
      Serial.println("********* Moving Forward *********");
      printByte(smile);
      forward(1000);
    }
    else if (String(inData) == "BACKWARD") //Again this is an arbitrary choice. It would probably be something like: MOTOR_STOP
    {
      Serial.println("********* Moving Backward *********");
      printByte(sad);
      backward(1000);
    }
    for (i = 0; i < remaining_bytes; i++) //This burns off any remaining bytes that the buffer can't handle.
    {
      inChar = serial_connection.read();
    }
    Serial.println(inData);//Print to the monitor what was detected
    serial_connection.println("LeixBot received " + String(inData)); //Then send an incrmented string back to the python script
    count++;//Increment the line counter
  }
  delay(100);//Pause for a moment
}
