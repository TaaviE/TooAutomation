/*
    TooNetworking - Networking abstraction layer for TooAutomation
    Copyright (C) 2017 Taavi E

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

#pragma once

#ifndef __TooNetworking_H__
#define __TooNetworking_H__
#include <stdint.h>
#include <Arduino.h>

#ifdef TOONETWORKING_SIGNING
#include "sha256.h"
typedef struct PayloadMetadataSigned { //To simplify memory operations
  uint8_t payload_size;
  uint8_t payload_hash[32];
};

typedef struct Payload_Metadata { // To simplify memory operations
  uint8_t payload_size;
};

typedef struct BufferItem { // Network buffer item
  uint8_t payload_size = 0;
  uint8_t payload_hash[32] = {0};
  uint8_t payload_destination = 0;
  BufferItem * payload_next = 0;
  void * payload = 0;
};

typedef struct NonceSent {
  uint8_t toNodeID = 255;
  uint32_t nonce = 0;
  NonceSent * payload_next = 0;
};

typedef struct NonceReceived {
  uint8_t nonce_from = 255;
  uint32_t nonce_time = 0;
  uint32_t nonce_timestamp = 0;
  NonceReceived * payload_next = 0;
};

typedef struct NoncePayload {
  uint32_t nonce = 0;
};

typedef struct NonceRequested {
  uint8_t nonce_from = 255;
  uint32_t nonce_time = 0;
  uint32_t nonce_lastrequest = 0;
  NonceRequested * payload_next = 0;
};

Sha256Class Sha256;

NonceSent * nonce_sent_start = 0;
NonceReceived * nonce_received_start = 0;
NonceRequested * nonce_requested_first = 0;
BufferItem * buffer_first = 0;

#else

typedef struct Payload_Metadata { //To calculate the size more easily
  uint8_t payload_size = 0;
};

typedef struct BufferItem { // Network buffer item
  uint8_t payload_size = 0;
  uint8_t payload_destination = 0;
  BufferItem * payload_next = 0;
  void * payload = 0;
};
BufferItem * buffer_first = 0;
#endif // TOONETWORKING_SIGNING

#ifdef TOORADIO_RF24

/*
* TODO:
* Send
* Send signed
* Send encrypted
* Send signed and encrypted
* Packets received
* Peek header but do not clear
* Read packet
* Connection available
*/

#include "RF24.h"
#include "RF24Mesh.h"
#include "RF24Network.h"

RF24 radio(TOO_RF24_CE, TOO_RF24_CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

  /**
   * Automatically set up the networking, implementation depends on the radio
   * 
   * @param passed_node_id Current node's ID.
   * @return true if successful, false otherwise
   */
bool TooNetworking_connection_begin(uint8_t passed_node_id){
  mesh.setNodeID(passed_node_id);
  Serial.println(mesh.getNodeID());
  mesh.begin();
  Serial.println(F("Mesh initialized"));
}

  /**
   * Allows sending a encrypted message
   * 
   * @param
   * @return true if successful, false otherwise
   */
bool TooNetworking_send(uint8_t for_node, void * payload, uint8_t size){

}

#ifdef TOONETWORKING_SIGNING
  /**
   * Allows sending a signed message
   * 
   * @param
   * @return true if successful, false otherwise
   */
bool TooNetworking_send_signed(uint8_t for_node, void * payload, uint8_t size){

}
#endif // TOONETWORKING_SIGNING

#ifdef TOONETWORKING_ENCRYPTION
  /**
   * Allows sending an encrypted message
   * 
   * @param
   * @return true if successful, false otherwise
   */
bool TooNetworking_send_encrypted(uint8_t for_node, void * payload, uint8_t size){

}
#endif // TOONETWORKING_ENCRYPTION

#ifdef TOONETWORKING_ENCRYPTION && TOONETWORKING_SIGNING
  /**
   * Allows sending a signed and encrypted message
   * 
   * @param
   * @return true if successful, false otherwise
   */
bool TooNetworking_send_signed_encrypted(uint8_t for_node, void * payload, uint8_t size){

}
#endif // TOONETWORKING_ENCRYPTION && TOONETWORKING_SIGNING

  /**
   * Allows reading a message from buffer 
   * and it's NOT cleared after being read
   * 
   * @param
   * @return true if successful, false otherwise
   */
bool TooNetworking_peek(RF24NetworkHeader& header, void* message, uint16_t maxlen){

}

  /**
   * Allows reading a message from buffer 
   * and it's cleared after being read
   * 
   * @param 
   * @return true if successful, false otherwise
   */
bool TooNetworking_read(RF24NetworkHeader& header, void* message, uint16_t maxlen){

}

  /**
   * Checks if there's something ready to be processed
   * 
   * @return True if successful, false otherwise
   */
bool TooNetworking_connection_available(){
   return network.available();
}

  /**
   * Checks if the master node is reachable
   * 
   * @return true if successful, false otherwise
   */
bool TooNetworking_connection_check(){
   return mesh.checkConnection();
}

  /**
   * Tries to reestablish connection to the network
   * 
   * @return True if successful, false otherwise
   */
bool TooNeteorking_connection_fix(){
   return mesh.renewAddress();
}

#ifdef TOONETWORKING_MASTER
  /**
   * Does the necessary maintenance and DHCP(-like) actions on the network
   * 
   * @return True if successful, false otherwise
   */
bool TooNeteorking_connection_maintenance(){
   
}
#else
  /**
   * Does the necessary maintenance on the network
   * 
   * @return True if successful, false otherwise
   */
bool TooNeteorking_connection_maintenance(){
   
}

#endif

  /**
   * Tries to reestablish connection to the network
   * 
   * @return True if successful, false otherwise
   */
bool TooNeteorking_connection_fix(){
   return mesh.renewAddress();
}
#else  // TOORADIO_RF24
   #error "Networking library not selected"
#endif // No radio

  /**
   * Automatically set up the networking
   * Every radio should provide TooNetworking_connection_begin(uint8_t nodeID)
   * for easy initialization of the network
   * 
   * @param passed_node_id Current node's ID.
   * @return true if successful, false otherwise
   */
bool TooNetworking_begin(uint8_t passed_node_id){
  TooNetworking_connection_begin(passed_node_id);
}

#endif // __TooNetworking_H__