/*  
 *  LoRa 868 / 915MHz SX1272 LoRa module
 *  
 *  Copyright (C) Libelium Comunicaciones Distribuidas S.L. 
 *  http://www.libelium.com 
 *  
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or 
 *  (at your option) any later version. 
 *  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see http://www.gnu.org/licenses/. 
 *  
 *  Version:           1.2
 *  Design:            David Gascón
 *  Implementation:    Covadonga Albiñana, Victor Boria, Ruben Martin
 */

//Choose appropriate module
//#define AlbaTracker
#define LoraComModule

//REMEMBER to change SS pin in SX1272.h file accordingly
#include "SX1272.h"
#include <SPI.h>

int e;
char my_packet[100];
char message1 [] = "Note2";
char message2 [] = "Note2 - timeout";

int ADDRESS = 8;
int ADDRESS_RECEIVER = 1;

#define debug 2 //1 - get packet info, 2 get program debug info

unsigned long int timer = 10000;
int delayTime = 10000;
char messageWaiting = 0;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // Print a start message
  Serial.println(F("SX1272 module and Arduino: receive packets without ACK"));

   // Power ON the module
  sx1272.ON();
  
  // Set transmission mode and print the result
  e = sx1272.setMode(1);
  #if (debug > 1)
  Serial.print(F("Setting mode: state "));
  Serial.println(e, DEC);
  #endif
  
  // Select frequency channel
  e = sx1272.setChannel(CH_16_868);
  #if (debug > 1)
  Serial.print(F("Setting Channel: state "));
  Serial.println(e, DEC);
  #endif
  
  // Select output power (Max, High or Low)
  e = sx1272.setPower('P');
  #if (debug > 1)
  Serial.print(F("Setting Power: state "));
  Serial.println(e);
  #endif

//  // Set value for over current protection (0x1B if 20dBm output power - 0x0B is default!)
//  e = sx1272.setMaxCurrent(0x1B); // Only set this if 20dBm power is used
//  #if (debug > 1)
//  Serial.print(F("Setting Over Current Protection: state "));
//  Serial.println(e);
//  #endif

  //Select signal output pin
  #if defined(AlbaTracker)
  e = sx1272.setRfOutPin(0);
  #endif
  
  #if defined(LoraComModule)
  e = sx1272.setRfOutPin(1);
  #endif

  #if (debug > 1)
  Serial.print(F("Setting output pin: state "));
  Serial.println(e);
  #endif
  
  // Set the node address and print the result
  e = sx1272.setNodeAddress(ADDRESS);
  #if (debug > 1)
  Serial.print(F("Setting node address: state "));
  Serial.println(e, DEC);
  #endif
  
  // Print a success message
  #if (debug > 1)
  Serial.println(F("SX1272 successfully configured "));
  #endif

  // Set external interrupt for received packages
  setInterruptPin(); //used with Lora Com Module
  initExtInt(); //used with Lora Com Module

  // Initiate the receive mode
  e = sx1272.receive();
  #if (debug > 1)
  Serial.print("Receive setup: ");
  Serial.println(e, DEC);
  #endif
}

void loop(void)
{
  
  if (messageWaiting) {
    e = sx1272.getPacket();
    #if (debug > 1)
      Serial.print(F("Receive status: "));
      Serial.println(e, DEC);
    #endif
    if ( e == 0 )
    {
      Serial.print(F("Receive packet, state "));
      Serial.println(e, DEC);
  
      for (unsigned int i = 0; i < sx1272.packet_received.length; i++)
      {
        my_packet[i] = (char)sx1272.packet_received.data[i];
      }
      Serial.print(F("Message: "));
      Serial.println(my_packet);
    }

    // Now send a reply
    // Send message1 and print the result
    e = sx1272.sendPacketTimeout(ADDRESS_RECEIVER, message1);
    #if (debug > 1)
    Serial.print(F("Packet sent, state "));
    Serial.println(e, DEC);
    #endif

    // Get back to listening for packages to be received
    e = sx1272.receive();
    #if (debug > 1)
    Serial.print("Receive setup: ");
    Serial.println(e, DEC);
    #endif

    timer = millis() + delayTime;
    messageWaiting = 0;
  }

  // If no message has been received for some time, send something
  if (timer < millis()) {
    // Now send a reply
    // Send message1 and print the result
    e = sx1272.sendPacketTimeout(ADDRESS_RECEIVER, message1);
    #if (debug > 1)
    Serial.print(F("Packet sent, state "));
    Serial.println(e, DEC);
    #endif

    // Get back to listening for packages to be received
    e = sx1272.receive();
    #if (debug > 1)
    Serial.print("Receive setup: ");
    Serial.println(e, DEC);
    #endif
    timer += delayTime;
  }
}

void setInterruptPin() {
  //Set pin PC5 as input
  DDRC &= ~(1 << DDC5);
}

void initExtInt() {
  //Disable global interrupts
  cli();
  
  //Enable external interrupts on pins PCINT[8:14] pins (datasheet p. 70). Can be read from the PCIF1 register
  PCICR |= (1 << PCIE1); //(p. 73)

  //Only enable pin change interrupt on pin PC5 (A5 on Arduino) 
  PCMSK1 |= (1 << PCINT13);

  //Enable global interrupts
  sei();
}

ISR (PCINT1_vect)
{
  if (PINC & (1 << PINC5)) {
    messageWaiting = 1;
  }
}
