/*
    A signing library for TooAutomation
    Copyright (C) 2018 Taavi E.

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

/**
 * @file TooSigning.h
 *
 * This file describes the necessary functionality
 * that allow nodes in the network to communicate
 */

#ifndef __TooSigning_H__
#define __TooSigning_H__

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "TooAutomation.h"
#include "modules/TooNetworking/TooNetworking.h"
#include "modules/TooNetworking/TooNetworking_data.h"

#include "RF24.h"
#include "RF24Mesh.h"
#include "RF24Network.h"

#ifdef SW_SIGNING
    #include "drivers/ATSHA204_SW/ATSHA204_SW.h"
#else
    #include "drivers/ATSHA204/ATSHA204.h"
#endif

#include "TooSigning.cpp"

extern class RF24Network network;

extern class RF24Mesh mesh;

class Sha256Class Sha256;


#ifdef __cplusplus
extern "C" {
#endif

/**
   Initialize the signing library

   @param passed_node_id Current node's ID.
*/
void TooSigning_signed_network_begin(uint8_t passed_nodeID);

void TooSigning_signed_network_update(void);

/**
 * **Functions that simplify hashing operations and hash management**

   @defgroup TOOSIGNING_HASHING

   @{
*/

/**
   Hashes large(ish) amounts of data

   @param payload data that has to be hashed
   @param payload_size size of the data that has to be hashed
*/
void TooSigning_hash_data(void *payload, size_t payload_size);

/**
   Dumps hash to serial

   @param hash pointer to standard hash
*/
void TooSigning_hash_print(uint8_t *hash);

/**
   Creates a copy of the hash somewhere else

   @param hash source
   @param result_hash destination
*/
void TooSigning_hash_store(void *hash, void *result_hash);

/**
   Compares two hashes with each other

   @param hash1 first hash
   @param hash2 second hash
   @return bool equality
*/
bool TooSigning_hash_compare(void *hash1, void *hash2);
/** @} */


/**
 * **Functions that simplify debugging**

   @defgroup TOOSIGNING_DEBUGGING

   @{
*/
void TooSigning_requested_noncelist_print(void);

void TooSigning_sent_noncelist_print(void);

void TooSigning_received_noncelist_print(void);

void TooSigning_random_data_print(void *data, size_t size);
/** @} */


/**
 * **Functions that initialize the required buffers**

   @defgroup TOOSIGNING_INITIALIZE

   @{
*/

/**
   Initializes the sent nonce list

   @return bool if the initialization succeeded
*/
bool TooSigning_sent_noncelist_initialize(void);

/**
   Initializes the received nonce list

   @return bool if the initialization succeeded
*/
bool TooSigning_received_noncelist_initialize(void);

/**
   Initializes the requested nonce list

   @return bool if the initialization succeeded
*/
bool TooSigning_requested_noncelist_initialize(void);
/** @} */

/**
   Requests nonce from node

   @param nodeID node from which to request the nonce
*/
void TooSigning_request_nonce_from_node_id(uint8_t nodeID);

/**
   Requests nonce from node

   @param nodeID node from which to request the nonce
*/
bool TooSigning_requested_noncelist_add(uint8_t passed_nodeID);

bool TooSigning_requested_noncelist_delete(NonceRequested *previous, NonceRequested *current);

bool TooSigning_requested_noncelist_received(uint8_t passed_nodeID);

NonceRequested *TooSigning_requested_noncelist_find_for_nodeID(uint8_t passed_nodeID);

bool TooSigning_requested_noncelist_retry_all(void);

bool TooSigning_requested_noncelist_remove_timeout(void);


NonceSent *TooSigning_sent_noncelist_find_from_ID(uint8_t nodeID);

bool TooSigning_sent_noncelist_add(uint8_t toNodeID, uint32_t nonce);

void TooSigning_sent_noncelist_remove(NonceSent *previous, NonceSent *current);

void TooSigning_sent_noncelist_remove_timeout(void);


NonceReceived *TooSigning_received_noncelist_find_from_ID(uint8_t nodeID);

bool TooSigning_received_noncelist_add(uint8_t passed_fromNodeId, uint32_t passed_nonce);

void TooSigning_received_noncelist_remove(NonceReceived *previous, NonceReceived *current);

void TooSigning_received_noncelist_remove_timeout(void);


void TooSigning_read_hmac_from_progmem(uint8_t nodeID, void *hmac_pointer);


#ifdef __cplusplus
}
#endif

//#ifdef __cplusplus
//#include "cpp_wrapper.h"
//#endif

#endif // __RF24_H__
