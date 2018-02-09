/*
    TooAutomation_Master - Example of a master node of TooAutomation
    Copyright (C) 2018 Taavi E

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define TOONETWORKING_MASTER // Indicates this is the master node
#define TOONETWORKING_SIGNING // Enable signing
#define SW_SIGNING
#define TOORADIO_RF24 // Select nRF24L01+ as the radio
#define TOO_RF24_CE 9 // Define needed pins
#define TOO_RF24_CS 10

#include "TooAutomation.h"

#define nodeID 0

void setup(void) {
    Serial.begin(115200);
    TooNetworking_connection_begin(nodeID);
}

void loop(void) {
    TooNetworking_connection_maintenance();
}

