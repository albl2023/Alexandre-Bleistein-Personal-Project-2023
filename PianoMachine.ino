/*
  Shift Register Pins: 
      Latch = 2 [yellow wire]
      Clock = SCK [orange wire]
      Data = MOSI [blue wire]
*/

//Setting ShiftPWM Library Parameters
const int ShiftPWM_latchPin = 2; 
const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = true;

#include <ShiftPWM.h> //https://github.com/elcojacobs/ShiftPWM

#define MAX_DUTY_CYCLE 255
#define PWM_FREQUENCY 150
#define NUM_REGISTERS 4

#include <MIDIUSB.h> //https://github.com/arduino-libraries/MIDIUSB

midiEventPacket_t notes;
midiEventPacket_t noteMemory[31]; //Stores note data for each individual solenoid (motor)
int noteAsPin;
bool validNote;

unsigned long millisTime[31] = {0}; //Acts as a timer for each individual solenoid
unsigned long currentTime;

void setup()
{
  Serial.begin(115200);
  ShiftPWM.SetAmountOfRegisters(NUM_REGISTERS);
  ShiftPWM.Start(PWM_FREQUENCY, MAX_DUTY_CYCLE);
  ShiftPWM.SetAll(0);
}


void loop()
{
  currentTime = millis(); //Updates clock
  notes = MidiUSB.read(); //Recieves MIDI data from computer
  if(notes.byte2-48 >= 0 && notes.byte2-48 <= 30) //Filters out unnecessary MIDI data
  {
    validNote = true;
    noteAsPin = (int)notes.byte2-48; 
  }
  
  //Check if computer is saying a note should be ON [144]
  if(notes.byte1 == 144 && notes.byte3 != 0 && validNote == true)
  {
    //Store note data, engage the solenoid of that note, and start timer for that solenoid
    noteMemory[noteAsPin] = notes;
    ShiftPWM.SetOne(noteAsPin, 255);
    millisTime[noteAsPin] = currentTime;
  } 

  //Check if computer is saying a note should be OFF [128] or [144 & Note Velocity = 0]
  if((notes.byte1 == 128 || (notes.byte1 == 144 && notes.byte3 == 0)) && validNote == true  )
  {
    //Disengage solenoid of that note, store note data, and reset timer for that solenoid
    ShiftPWM.SetOne(noteAsPin, 0);
    noteMemory[noteAsPin] = notes;
    millisTime[noteAsPin] = 0;
  }

  for(int i = 0; i < 31; i++)
  {
    //Checking for any solenoid that has been ON [144] for longer than 100ms
    if((currentTime - millisTime[i] > 100) && (noteMemory[i].byte1 == 144 && noteMemory[i].byte3 != 0 && validNote == true) )
    {
      //If above is true, that solenoid is set to a low power state to save energy
      ShiftPWM.SetOne(noteMemory[i].byte2-48, 150);
    }
  }
  
/*
  //Code which helps in fixing bugs
   //ShiftPWM.PrintInterruptLoad();
   Serial.print(noteMemory[27].byte1);
   Serial.print(" ");
   Serial.print(noteMemory[27].byte2);
   Serial.print(" ");
   Serial.print(noteMemory[27].byte3);
   Serial.print("      ");
   
   Serial.print(notes.byte1);
   Serial.print(" ");
   Serial.print(notes.byte2);
   Serial.print(" ");
   Serial.println(notes.byte3);
   
   for(int i = 0; i<31; i++) Serial.print(millisTime[i]); Serial.println("");
   delay(100);
 */

   validNote = false;
   
}
