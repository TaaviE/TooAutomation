/*
 *    TooNetworking - Networking abstraction layer for TooAutomation
 *    Copyright (C) 2018 Taavi E.
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, version 3 of the License.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file TooNetworking.cpp
 *
 * This file implements the necessary functionality
 * that allow nodes in the network to communicate
 */

#include <stdint.h>
#include <Arduino.h>
#include "TooAutomation.h"
#include "TooNetworking.h"
#include "TooNetworking_data.h"

uint8_t current_node_ID;

#ifdef TOONETWORKING_SIGNING

#include "modules/TooSigning/TooSigning.h"

#ifdef SW_SIGNING
    #include "drivers/ATSHA204_SW/ATSHA204_SW.h"
#else
    #include "drivers/ATSHA204/ATSHA204.h"
#endif // SW_SIGNING

/**
 * Secret HMACs
 */
#include "configuration/hmacs.c"
#endif // TOONETWORKING_SIGNING
  
#ifdef TOORADIO_RF24

BufferItem * buffer_first = 0;              /**< Pointer to the payload list start */
/*
 * TODO:
 * Send signed
 * Send encrypted
 * Send signed and encrypted
 * Packets received
 * Peek header but do not clear
 * Read packet
 * Connection available 50%
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
    current_node_ID = passed_node_id;
    mesh.setNodeID(passed_node_id);
    Serial.print("Node ID: ");
    Serial.println(mesh.getNodeID());
    mesh.begin();
    Serial.println(F("Mesh initialized"));
    
    #ifdef TOONETWORKING_SIGNING 
    TooSigning_signed_network_begin(passed_node_id);
    #endif // TOONETWORKING_SIGNING
    
    #ifdef TOONETWORKING_ENCRYPTION
    // TODO: Init encryption
    #endif // TOONETWORKING_SIGNING
    radio.flush_rx();
    radio.flush_tx();
}

/**
 * Allows sending a encrypted message
 * 
 * @param identifier sets the used payload type, see TOONETWORKING_RESERVED_TYPES for more information
 * @param size payload size
 * @param for_node destination node
 * @param payload payload
 * @return true if successful, false otherwise
 */
bool TooNetworking_send(uint8_t for_node, void * payload, uint8_t size, uint8_t identifier){
    return mesh.write(for_node, payload, identifier, size);
}

bool TooNetworking_other_node_is_online(uint8_t nodeID){
    return true; // TODO
}

bool TooNetworking_this_node_is_online(){
    return true; // TODO
}

#ifdef TOONETWORKING_SIGNING


bool TooNetworking_bufferlist_initialize() {
    buffer_first = malloc(sizeof(BufferItem));
    Serial.println((uint8_t) sizeof(buffer_first));
    if (buffer_first == NULL) {
        Serial.println(F("Buffer init failed"));
        return false;
    }
  
    ((BufferItem *) buffer_first)->next = NULL;
  
    buffer_first->next = NULL;
    return true;
}

/**
 * Checks if there's connection
 *
 * @return True if successful, false otherwise
 */
bool TooNetworking_connection_available(){
    return mesh.checkConnection();
}

void TooNetworking_bufferlist_remove(BufferItem * previous, BufferItem * current) {
    Serial.println(F("Removing from buffer list"));
    if (current = buffer_first) { // Start of buffer list
        free(current->payload);
        free(current);
        buffer_first = NULL;
    } else if (current->next == NULL) { // First in the buffer list
        free(current->payload);
        free(current);
        previous->next = NULL;
    } else if (previous != NULL) { // Somehwere in the middle of the list
        previous->next = current->next;
        free(current->payload);
        free(current);
    } else {
        Serial.print(F("Error case not matched, dumping pointers: "));
        Serial.print((uint8_t) buffer_first);
        Serial.print(F(" "));
        Serial.print((uint8_t) previous);
        Serial.print(F(" "));
        Serial.print((uint8_t) previous->next);
        Serial.print(F(" "));
        Serial.print((uint8_t) current);
        Serial.print(F(" "));
        Serial.println((uint8_t) current->next);
    }
}

bool TooNetworking_bufferlist_send_(BufferItem * current, BufferItem * previous){
    if(TooNetworking_connection_available()){
        if(TooNetworking_other_node_is_online(current->payload_destination)){
            size_t buffer_size = current->payload_size+sizeof(Payload_Metadata_Received);
            void * buffer = malloc(buffer_size);
            memmove(buffer, current, sizeof(Payload_Metadata_Received)); //Copy metadata to buffer
            memmove(buffer+sizeof(Payload_Metadata_Received), current->payload, current->payload_size); //Copy payload to buffer after  metadata
        
            if(TooNetworking_send(current->payload_destination, buffer, buffer_size, MSG_REGULAR)){
                TooNetworking_bufferlist_remove(current, previous);
                return true;
            } else {
                return false;
            }
        } else{
            return false;
        }
    } else{
        //TODO: Non-blocking fix connection
        return false;
    }
}

/**
 * Adds message that the system couldn't send instantly to the buffer
 *
 * */
void TooNetworking_bufferlist_add(uint8_t payload_destination, void * payload, uint8_t size, uint8_t type=MSG_REGULAR){
    Serial.println(F("Add item to buffer list"));
    BufferItem * current = buffer_first;
    BufferItem * previous = NULL;
    if (buffer_first == 0) {
        if (!TooNetworking_bufferlist_initialize()) {
            return false;
        } else {
        current = buffer_first;
        Serial.println(F("Initialized the buffer"));
        }
    }
  
    while (current != NULL) {  //Take the last item in the list
        Serial.print(F("Finding the last item: "));
        Serial.println((uint8_t) &current);
        current = current->next;
    }
  
    previous = current;
    current->next = malloc(sizeof(BufferItem)); //TODO: Safe malloc (enough room and error handling)
    current = current->next;
  
    current->payload_destination = payload_destination;
    current->payload_size = size;
    current->payload_type = type;
  
    if(type == 0){
        if(TooNetworking_bufferlist_send_(current, previous)){
            return true;
        }
    } else {
        Serial.println(F("Finding nonce for nodeID"));
        NonceReceived * nonce = TooSigning_received_noncelist_find_from_ID(payload_destination);
    
        if (nonce != 0) {
            Serial.print(F("TooNetworking_bufferlist_add payload: "));
            TooSigning_random_data_print(current->payload, current->payload_size);
            Serial.println();
            TooNetworking_send_signed_(current->payload_destination, current->payload, current->payload_size, nonce);
        } else { //TODO: Add encryption
            Serial.print(F("Adding pending nonce request... "));
            TooSigning_requested_noncelist_add(payload_destination);
            Serial.println(F("Added"));
        }
    }
  
    Serial.println(F("Adding item to buffer list"));
    current->next = malloc(sizeof(BufferItem));
  
    if (current->next == NULL) {
        Serial.println(F("Failed to malloc"));
        return false;
    }
  
    Serial.println(F("Added item to buffer list"));
    return true;
}



/**
 * Allows sending a signed message, type must be embedded inside the payload
 * the receiver must know what to do with the message once it's been 
 * verified, messages that fail the checks are DISCARDED!
 * 
 * @param for_node destination
 * @param payload payload
 * @param size size of payload
 * @return true if successful, false otherwise
 */
bool TooNetworking_send_signed(uint8_t for_node, void * payload, uint8_t size){
    //Check if there's a nonce for that node
    NonceReceived * tempnonce = TooSigning_received_noncelist_find_from_ID(for_node);
    if(tempnonce != 0){
        Serial.print(F("TooNetworking_send_signed - Payload: "));
        TooSigning_random_data_print(payload, size);
        Serial.println();
        TooNetworking_send_signed_(for_node, payload, size, tempnonce);
        return true;
    }
  
    //Else send nonce request
    TooSigning_requested_noncelist_add(for_node);
  
    //Store in buffer
    TooNetworking_bufferlist_add(for_node, payload, size, MSG_SIGNED);
    return true;
}
#endif //TOONETWORKING_SIGNING

#ifdef TOONETWORKING_ENCRYPTION

/**
 * Allows sending an encrypted message
 * 
 * @param
 * @return true if successful, false otherwise
 */
bool TooNetworking_send_encrypted(uint8_t for_node, void * payload, uint8_t size){
  
}
#endif // TooNetworking_ENCRYPTION

#ifdef TOONETWORKING_ENCRYPTION && TOONETWORKING_SIGNING

/**
 * Allows sending a signed and encrypted message
 * 
 * @param
 * @return true if successful, false otherwise
 */
bool TooNetworking_send_signed_encrypted(uint8_t for_node, void * payload, uint8_t size){
  
}
#endif // TooNetworking_ENCRYPTION && TooNetworking_SIGNING

/**
 * Allows reading a message from buffer
 * and it's NOT cleared after being read
 *
 * @param
 */
void TooNetworking_peek(RF24NetworkHeader& header, void* message, uint16_t maxlen){
    network.peek(header, message, maxlen);
}

/**
 * Allows reading a message from buffer
 * and it's cleared after being read
 *
 * @param
 * @return true if successful, false otherwise
 */
bool TooNetworking_read(RF24NetworkHeader& header, void* message, uint16_t maxlen){
    return network.read(header, message, maxlen);
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
bool TooNetworking_connection_fix(){
    if(!TooNetworking_connection_available()){
        Serial.println(F("No connection available, renewing"));
        return mesh.renewAddress();
    }
}

/**
 * **Regular message buffer list**
 *
 *
 * @defgroup TOONETWORKING_SIMPLE_BUFFER
 *
 * @{
 */

void TooNetworking_bufferlist_print() {
    Serial.println(F("___ BUFFER DUMP ___"));
  
    BufferItem * current = buffer_first;
    while (current != 0) {
        Serial.print(F("| For: "));
        Serial.println(current->payload_destination);
        Serial.print(F("| Pointer to next: "));
        Serial.println((uint16_t) current->next);
        Serial.print(F("| Payload size: "));
        Serial.println(current->payload_size);
        current = current->next;
        Serial.println(F("-------------------"));
    }
    Serial.println(F(""));
}

BufferItem * TooNetworking_bufferlist_find_for_id(uint8_t nodeID) {
    BufferItem * current = buffer_first;
    while (current != NULL) {
        if (current->payload_destination == nodeID) {
            return current;
        }
        current = current->next;
    }
  
    return NULL;
}


/** @} */

#ifdef TOONETWORKING_SIGNING

/**
 * Allows sending a signed message, type must be embedded inside the payload
 * the receiver must know what to do with the message once it's been
 * verified, messages that fail the checks are DISCARDED!
 *
 * @param for_node destination
 * @param payload payload
 * @param size size of payload
 * @param nonce nonce
 * @return true if successful, false otherwise
 */

bool TooNetworking_send_signed_(uint8_t for_node, void * payload, uint8_t size, uint32_t nonce){
    if(TooNetworking_connection_available()){
        Serial.println(F("Node is online"));
        if(TooNetworking_other_node_is_online(for_node)){
            Serial.println(F("Destination is online"));
            Serial.print(F("TooNetworking_send_signed_ - Payload: "));
            TooSigning_random_data_print(payload, size);
            Serial.println();
            
            size_t buffer_size = size+sizeof(Payload_MetadataSigned_Received);
            void * buffer = malloc(buffer_size);
            Payload_MetadataSigned_Received metadata;
            metadata.payload_size = size;
            metadata.payload_type = MSG_SIGNED;
            //Payload signing process start
            uint8_t hmac[20] = {0};
            TooSigning_read_hmac_from_progmem(current_node_ID, &hmac);
      
            Serial.println(F("HMAC used: "));
            TooSigning_random_data_print(hmac, 20);
//             Serial.println();
            TooSigning_init_hmac(hmac, 20);
      
            Serial.print(F("Payload: "));
            TooSigning_hash_data(payload, size);
            TooSigning_random_data_print(payload, size);
            
            Serial.print(F("Nonce used: "));
            Serial.println(nonce);
            Serial.println((uint8_t) &nonce);
            TooSigning_hash_data(&nonce, sizeof(uint32_t));
            Serial.print(F("Generated hash: "));
            TooSigning_hash_store(TooSigning_get_hmac(), metadata.payload_hash);
            TooSigning_random_data_print(metadata.payload_hash, 20);
            //Payload signing process end
            //To buffer, from metadata, with the size of metadata
            memmove(buffer, &metadata, sizeof(Payload_MetadataSigned_Received)); //Copy metadata to buffer
            //To buffer with an offset, from payload, with the size of payload
            memmove(buffer+sizeof(Payload_MetadataSigned_Received), payload, size); //Copy payload to buffer after metadata
            
            Serial.print(F("Full buffer: "));
            TooSigning_random_data_print(buffer, sizeof(Payload_MetadataSigned_Received)+size);
      
            if(TooNetworking_send(for_node, buffer, buffer_size, MSG_SIGNED)){
                return true;
            }
        }
        Serial.println(F("Destination is offline"));
    } else{
        Serial.println(F("This node is offline"));
        TooNetworking_connection_fix();
        //TODO: Preferrably non-blocking fix connection
    }
}

/**
 * Allows sending a signed message, type must be embedded inside the payload
 * the receiver must know what to do with the message once it's been
 * verified, messages that fail the checks are DISCARDED!
 *
 * @param for_node destination
 * @param payload payload
 * @param size size of payload
 * @return true if successful, false otherwise
 */
bool TooNetworking_bufferlist_send_signed(uint8_t for_node, void * payload, uint8_t size){
    //Check if there's a nonce for that node
    NonceReceived * tempnonce = TooSigning_received_noncelist_find_from_ID(for_node);
    if(tempnonce != 0){
        Serial.print(F("TooNetworking_bufferlist_send_signed - Payload: "));
        TooSigning_random_data_print(payload, size);
        Serial.println();
        if(TooNetworking_send_signed_(for_node, payload, size, tempnonce->nonce)){  
            //Message hasn't made it to the buffer list
            return true;
        }
    }
  
    //Else send nonce request
    TooSigning_requested_noncelist_add(for_node);
  
    //Store in buffer
    TooNetworking_bufferlist_add(for_node, payload, size, MSG_SIGNED);
    return true;
}

#endif //TOONETWORKING_SIGNING


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

void TooNetworking_bufferlist_send_all() {
    BufferItem * current = buffer_first;
    BufferItem * previous = 0;
  
    TooNetworking_bufferlist_print();
    Serial.println(F("Sending all: "));
    Serial.println((uint8_t) buffer_first);
    Serial.println((uint8_t) buffer_first->next);
    while (current != 0) {
        Serial.println(F("There's something in the buffer to send"));
    
        if(current->payload_type == MSG_REGULAR){
            if(TooNetworking_bufferlist_send_(current, previous)){
                TooNetworking_bufferlist_remove(current, previous);
            }
        }
        #ifdef TOONETWORKING_SIGNING
        else if(current->payload_type == MSG_SIGNED){
            NonceReceived * nonce = TooSigning_received_noncelist_find_from_ID(current->payload_destination);
      
            if (nonce != 0) {
                Serial.println(F(" ..one nonce for node is not 0!"));
                Serial.print(F("TooNetworking_bufferlist_send_all - Payload: "));
                TooSigning_random_data_print(current->payload, current->payload_size);
                Serial.println();
                if(TooNetworking_send_signed_(current->payload_destination, current->payload, current->payload_size, nonce->nonce)){
                    //Message was sent, might as well remove it
                    TooNetworking_bufferlist_remove(previous, current);
                }
            }
        }
    #endif
        previous = current;
        current = current->next;
    }
}

/** @} */

/**
 * Does the necessary master node
 * maintenance and DHCP(-like) actions on the network
 *
 * @return True if successful, false otherwise
 */
uint32_t displayTimer = 0;
bool TooNetworking_connection_maintenance(){
    mesh.update();
    #ifdef TOONETWORKING_MASTER
        //Serial.print(F("D"));
        mesh.DHCP();
        if (millis() - displayTimer > 5000) {
            displayTimer = millis();
            Serial.println(" ");
            Serial.println(F("********Assigned Addresses********"));
            for (int i = 0; i < mesh.addrListTop; i++) {
                Serial.print("NodeID: ");
                Serial.print(mesh.addrList[i].nodeID);
                Serial.print(" RF24Network Address: 0");
                Serial.println(mesh.addrList[i].address, OCT);
            }
            Serial.println(F("**********************************"));
        }
    #endif // TOONETWORKING_MASTER
    
    #ifdef TOONETWORKING_SIGNING
        //Serial.print(F("U"));
        //TooSigning_signed_network_begin(20);
        TooSigning_signed_network_update();
    #endif // TOONETWORKING_SIGNING
    
    #ifndef TOONETWORKING_MANUAL && !TOONETWORKING_SIGNING
        //Serial.print(F("A"));
        TooSigning_unsigned_network_available();
    #endif
    mesh.update();
}


#else  // TOORADIO_RF24
#error "Networking library not selected"
#endif // TOORADIO_RF24

/**
 * Automatically set up the networking
 * Every radio should provide TooNetworking_connection_begin(uint8_t nodeID)
 * for easy initialization of the network
 *
 * @param passed_node_id Current node's ID.
 * @return true if successful, false otherwise
 */

bool TooNetworking_begin(uint8_t passed_node_id) {
    TooNetworking_connection_begin(passed_node_id);
}

/**
 * Error out when preprocessor flags are with the wrong
 * capitalization to save debugging time
 *
 */
#ifdef TooNetworking_SIGNING
#error "Define TOONETWORKING_SIGNING instead!"
#endif

#ifdef TooNetworking_MASTER
#error "Define TOONETWORKING_MASTER instead!"
#endif

#ifdef TooRadio_RF24
#error "Define TOORADIO_RF24 instead!"
#endif
