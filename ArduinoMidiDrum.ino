/*
 * Copyright (c) 2022 Immanuel Mundary
 * Email: ImmanuelMundary09@gmail.com
 *
 * This file is part of ArduinoMidiDrum.
 *
 * ArduinoMidiDrum is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <HardwareSerial.h>
#include <ArduMIDI.h>

//MIDI initialization
ArduMIDI midi = ArduMIDI(Serial, CH1);

//Piezo defines
#define NUM_PIEZOS 1

#define SERIAL_RATE 115200

//MIDI defines.
#define MAX_MIDI_VELOCITY 127
#define NOTE_ON_CMD 0x90
#define NOTE_OFF_CMD 0x80

#define MIN_TIME_BETWEEN_NOTES 50

//Mux Defines.
#define MUX_INPUT A1
#define MUX_S0 3
#define MUX_S1 4
#define MUX_S2 5

//MIDI note trigger map.
unsigned short noteMap[NUM_PIEZOS]={70,71,72,73,74,75,76,77};

//Program defines.
unsigned short thresholdMap[NUM_PIEZOS]={30,30,30,100,100,30,30,50};
unsigned short intensityEndMap[NUM_PIEZOS]={300};
unsigned short velocityStartMap[NUM_PIEZOS]={20};
unsigned short peakSignal[NUM_PIEZOS][2]={0};
/*ALL TIME MEASURED IN MILLISECONDS*/
unsigned long lastPeakTime[NUM_PIEZOS]={0};


void setup()
{
  midi.begin();
  Serial.begin(SERIAL_RATE);
  pinMode(MUX_INPUT, INPUT);
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
}

void loop()
{
  //Acquiring current time.
  unsigned long currentTime = millis();

  //Looping through all sensors.
  for (short i = 0; i < NUM_PIEZOS; i++)
  {
    //Setting MUX to proper pin for reading signal.
    digitalWrite(MUX_S0, i&1);
    digitalWrite(MUX_S1, i&3);
    digitalWrite(MUX_S2, i&7);

    //Reading signal intensity
    unsigned short newSignal = analogRead(MUX_INPUT);

    //Check if signal is greater than threshold.
    if(newSignal>thresholdMap[i]){

      //Checking if the signal is greater than previous signal.
      if(peakSignal[i][1] > newSignal ){
        
        //storing signal information.
        peakSignal[i][0]=peakSignal[i][1];
        peakSignal[i][1]=newSignal;
      }

      //
      else if(peakSignal[i][0] < peakSignal[i][1]){
        /*peak detected*/

        //if previous peak and current peak have atleast minimum note distance.
        if(currentTime - lastPeakTime[i] > MIN_TIME_BETWEEN_NOTES){

          //Calculating MIDI note velocity.
          newSignal = map(newSignal,thresholdMap[i],intensityEndMap[i],velocityStartMap[i],127);
          if(newSignal > MAX_MIDI_VELOCITY) newSignal = MAX_MIDI_VELOCITY;

          //MIDI note trigger.
          midi.noteOn(CH1, noteMap[i], newSignal);
          midi.noteOff(CH1, noteMap[i]);
    
          //Clearing signal information after note trigger.
          peakSignal[i][0]=0;
          peakSignal[i][1]=0;
          lastPeakTime[i]=currentTime;
        }
      }
    }
  }
}
