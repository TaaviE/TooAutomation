/*
    TooAutomation_Node - Example of a standard node of TooAutomation
    Copyright (C) 2018 Taavi E

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#define TOONETWORKING_SIGNING // Enable signing
#define SW_SIGNING
#define TOORADIO_RF24 // Select nRF24L01+ as the radio
#define TOO_RF24_CE 9 // Define needed pins
#define TOO_RF24_CS 10

#include "TooAutomation.h"

// Node ID
#define nodeID 1 

uint32_t timer = 0;

void setup(void) {
   Serial.begin(115200);
   TooNetworking_connection_begin(nodeID);
}

void loop(void) {
    TooNetworking_connection_maintenance();

    if(millis() - timer > 2000){
         uint32_t dat = millis();
         Serial.println(F("----------------- SENDING ---------------------"));
         Serial.println((uint8_t) dat);
         Serial.println((uint8_t) dat);
         TooSigning_random_data_print(&dat, 4);
         TooNetworking_bufferlist_send_signed(0, &dat, sizeof(dat));
         timer = millis();
         Serial.println(F("-----------------------------------------------"));
    }
    delay(100);
}
