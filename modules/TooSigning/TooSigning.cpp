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
 * @file TooSigning.cpp
 *
 * This file implements the necessary functionality
 * that allow nodes in the network to communicate with 
 * signed messages
 */
#include "modules/TooNetworking/TooNetworking.h"
#include "TooSigning.h"
#include "configuration/hmacs.c"

#ifdef SW_SIGNING
    #include "drivers/ATSHA204_SW/ATSHA204_SW.h"
#else
    #include "drivers/ATSHA204/ATSHA204.h"
#endif

Sha256Class Sha256;

extern class RF24Mesh mesh;

extern class RF24Network network;

extern class RF24 radio;

extern uint8_t current_node_ID;

NonceReceived *nonce_received_start = 0;
NonceSent *nonce_sent_start = 0;
NonceRequested *nonce_requested_start = 0;

/*
  Hashing related functions
*/
void TooSigning_hash_data(void *payload, size_t payload_size) {
    Serial.println((uint8_t) payload);
    Serial.println(payload_size);
    for (uint8_t i = 0; i < payload_size; i++) { //Read the payload from the
        Serial.print(F("Writing... "));            //the payload byte by byte to the crypto
        uint8_t *pload = (uint8_t *) payload;
        uint8_t *pload_shifted = pload + i;
        Serial.print((uint8_t) * pload_shifted, DEC);
        Serial.print(F(" "));
        Sha256.write(*pload_shifted);
    }
    Serial.println();
}

void TooSigning_hash_print(uint8_t *hash) {
    for (uint8_t i = 0; i < 32; i++) {
        Serial.print(hash[i]);
    }
    Serial.println();
}

void TooSigning_hash_store(void *hash, void *result_hash) {
    memmove(result_hash, hash, sizeof(uint8_t[32]));
}

bool TooSigning_hash_compare(void *hash1, void *hash2) {
    for (uint8_t byte = 0; byte < 32; byte++) {
        if (*(((uint8_t *) hash1) + byte) != *(((uint8_t *) hash2) + byte)) {
            return false;
        }
    }
    return true;
}

void TooSigning_requested_noncelist_print() {
    NonceRequested *current = nonce_requested_start;
    Serial.println(F("___ REQUESTED NONCE LIST DUMP ___"));
    while (current != 0) {
        Serial.print(F("Requested this: "));
        Serial.println((uint8_t) current);
        Serial.print(F("Requested from: "));
        Serial.println(current->nonce_from);
        Serial.print(F("Requested time: "));
        Serial.println(current->nonce_request_first);
        Serial.print(F("Requested last: "));
        Serial.println(current->nonce_request_last);
        Serial.print(F("Requested next: "));
        Serial.println((uint8_t) current->next);
        
        current = current->next;
        if(current->nonce_request_first == 0){
            current = 0;
        }
    }
}

void TooSigning_sent_noncelist_print() {
    Serial.println(F("___ SENT NONCE DUMP ___"));

    NonceSent *current = nonce_sent_start;
    while (current != 0) {
        Serial.print(F("To: "));
        Serial.println(current->nonce_to);
        Serial.print(F("Nonce: "));
        Serial.println(current->nonce);
        current = current->next;
    }
}

void TooSigning_received_noncelist_print() {
    NonceReceived *current = nonce_received_start;
    Serial.println(F("___ RECEIVED NONCE DUMP ___"));
    while (current != 0) {
        Serial.print(F("To: "));
        Serial.println(current->nonce_from);
        Serial.print(F("Nonce: "));
        Serial.println(current->nonce);
        Serial.print(F("Timestamp: "));
        Serial.println(current->nonce_when);
        current = current->next;

    }
}

void TooSigning_random_data_print(void *data, size_t size) {
    void *start = data;

    for (uint8_t offset = 0; offset < size; offset++) {
        Serial.print((uint8_t)(*((uint8_t * )(data + offset))));
        Serial.print(F(" "));
    }
    Serial.println(F(" "));
}

bool TooSigning_sent_noncelist_initialize() {
    Serial.print(F("Sent nonce list init: "));
    Serial.println(sizeof(NonceSent));

    nonce_sent_start = malloc(sizeof(NonceSent));
    Serial.println(F("Malloc'd"));
    Serial.println((uint8_t) nonce_sent_start);
    if (nonce_sent_start == 0) {
        return false;
    }
    Serial.println(F("Returning true"));
    nonce_sent_start->next = 0;
    nonce_sent_start->nonce = 0;
    nonce_sent_start->nonce_to = 0;
    return true;
}

bool TooSigning_received_noncelist_initialize() {
    Serial.print(F("Received nonce list init: "));
    nonce_received_start = malloc(sizeof(NonceReceived));
    Serial.println(F("Malloc'd"));
    Serial.println((uint8_t) nonce_received_start);
    if (nonce_received_start == 0) {
        return false;
    }

    Serial.println(F("Returning true"));
    nonce_received_start->next = 0; 
    nonce_received_start->nonce_from = 255;
    nonce_received_start->nonce = 0;
    nonce_received_start->nonce_when = 0;
    return true;
}

bool TooSigning_requested_noncelist_initialize() {
    Serial.print(F("Requested nonce list init: "));
    nonce_requested_start = malloc(sizeof(NonceRequested));
    Serial.println((uint8_t) nonce_requested_start);
    if (nonce_requested_start == 0) {
        Serial.println(F("Request list init failed"));
        return false;
    }

    Serial.println(F("Setting next to 0"));
    nonce_requested_start->next = 0;
    nonce_requested_start->nonce_from = 255;    
    nonce_requested_start->nonce_request_first = 0;
    nonce_requested_start->nonce_request_last = 0;
    return true;
}

void TooSigning_request_nonce_from_node_id(uint8_t nodeID) {
    NoncePayload nonce_payload;
    nonce_payload.nonce = 0;
    Serial.print(F("Requesting nonce from: "));
    Serial.println(nodeID);
    uint8_t status = mesh.write(&nonce_payload, 'R', 1, nodeID);
    Serial.print(F("Status: "));
    Serial.println(status);
}

bool TooSigning_requested_noncelist_add(uint8_t passed_nodeID) {
    Serial.println(F("Adding to requested noncelist"));
    NonceRequested *current = nonce_requested_start;
    if (current == 0) {
        Serial.println(F("Not initialized"));
        if (!TooSigning_requested_noncelist_initialize()) {
            return false;
        }
        current = nonce_requested_start;
    } else {
        while (current->next != 0) {
            Serial.println(F("Looking for the last"));
            current = current->next;
        }
    }
    Serial.println(F("Storing data"));
    current->nonce_from = passed_nodeID;
    current->nonce_request_first = millis();
    TooSigning_request_nonce_from_node_id(passed_nodeID);
    current->nonce_request_last = millis();
    Serial.println(F("Stored"));
    TooSigning_requested_noncelist_print();
}

bool TooSigning_requested_noncelist_delete(NonceRequested *previous, NonceRequested *current) {
    //Delete list item
    if (previous == 0) {
        Serial.println(F("Removing first nonce request"));
        free(current);
        nonce_requested_start = 0;
    } else {
        Serial.println(F("Removing a nonce request in the middle"));
        previous->next = current->next;
        free(current);
    }
}

bool TooSigning_requested_noncelist_received(uint8_t passed_nodeID) {
    NonceRequested *current = nonce_requested_start;
    NonceRequested *previous = 0;
    TooSigning_requested_noncelist_print();
    while (current != 0) {
        if (current->nonce_from == passed_nodeID) {
            Serial.println(F("Deleting request"));
            TooSigning_requested_noncelist_delete(previous, current);
        }
        TooSigning_requested_noncelist_print();
        previous = current;
        current = current->next;
    }
}

NonceRequested *TooSigning_requested_noncelist_find_for_nodeID(uint8_t passed_nodeID) {
    //Find if request exists for nodeID
    NonceRequested *current = nonce_requested_start;
    while (current != 0) {
        if (current->nonce_from == passed_nodeID) {
            return current;
        }
        current = current->next;
    }
    return 0;
}

bool TooSigning_requested_noncelist_retry_all() {
    NonceRequested *previous = 0;
    NonceRequested *current = nonce_requested_start;
    TooSigning_requested_noncelist_print();
    while (current != 0) {
        Serial.println(F("Request list is not 0"));
        Serial.println(current->nonce_request_last);
        if (current->nonce_request_first != 0){
            if (millis() - current->nonce_request_last > 2000) {
                Serial.println(F("Rerequesting nonce"));
                TooSigning_request_nonce_from_node_id(current->nonce_from);
                current->nonce_request_last = millis();
            }
        }
        previous = current;
        current = current->next;
    }
}

bool TooSigning_requested_noncelist_remove_timeout() {
    NonceRequested *current = nonce_requested_start;
    NonceRequested *previous = 0;
    while (current != 0) {
        if (current->nonce_request_first != 0){
            if (millis() - current->nonce_request_first > 10000) {
                Serial.println(F("Removing nonce request"));
                TooSigning_requested_noncelist_delete(previous, current); //deletes current
            }
        }
        previous = current;
        current = current->next;

    }
    return 0;
}

NonceSent *TooSigning_sent_noncelist_find_from_ID(uint8_t nodeID) {
    NonceSent *current = nonce_sent_start;
    while (current != 0) {
        Serial.println(F("Sent noncelist find"));
        if (current->nonce_to == nodeID) {
            Serial.print(F("Found for: "));
            Serial.println(nodeID);
            return current;
        }
        current = current->next;

    }

    return 0;
}

bool TooSigning_sent_noncelist_add(uint8_t toNodeID, uint32_t nonce) {
    NonceSent *current = nonce_sent_start;
    Serial.println(F("Finding last in list"));
    //delay(100);
    Serial.println(F("Starting"));
    if (current == 0) {
        Serial.println(F("Initializing"));
        if (!TooSigning_sent_noncelist_initialize()) {
            return false;
        }
        current = nonce_sent_start;
    } else {
        while (current->next != 0) {
            current = current->next;
        }

        Serial.println(F("Allocating"));
        Serial.println((char) current);

        current->next = calloc(1, sizeof(NonceSent));
        if (current->next == 0) {
            return false;
        }
        current = current->next;
    }
    Serial.println(F("Allocated"));
    current->nonce_to = toNodeID;
    current->nonce = nonce;
    current->next = 0;
    Serial.println(F("Data stored"));
    delay(1000);
    return true;
}

void TooSigning_sent_noncelist_remove(NonceSent *previous, NonceSent *current) {
    Serial.println(F("Removing nonce"));
    Serial.println(F("/* Current:"));
    Serial.print(F("* Nonce: "));
    Serial.println(current->nonce);
    Serial.print(F("* Millis: "));
    Serial.println(millis());
    Serial.print(F("* This: "));
    Serial.println((uint8_t) current);
    Serial.print(F("* Previous: "));
    Serial.println((uint8_t) previous);
    Serial.print(F("* Next: "));
    Serial.println((uint8_t) current->next);
    Serial.println(F("\*"));
    if (previous != 0) {
        previous->next = current->next;
    } else {
        nonce_sent_start = 0;
    }
    free(current);
    Serial.println(F("Removed nonce"));
}

void TooSigning_sent_noncelist_remove_timeout() {
    Serial.println(F("Removing sent timeout"));
    NonceSent *current = nonce_sent_start;
    NonceSent *previous = 0;
    while (current != 0) {
        if (current->nonce != 0){
            if (millis() - current->nonce > 5000) {
                Serial.println(F("Found outdated nonce"));
                TooSigning_sent_noncelist_remove(previous, current);
            }
        } else{
            Serial.println(F("Start of buffer, ignored"));   
        }

        previous = current;
        current = current->next;
    }
}

NonceReceived *TooSigning_received_noncelist_find_from_ID(uint8_t nodeID) {
    Serial.print(F("Received nonce list find for: "));
    Serial.println(nodeID);
    NonceReceived *current = nonce_received_start;
    while (current != 0) {
        if (current->nonce_from == nodeID) {
            Serial.print(F("Found nonce: "));
            Serial.println(current->nonce);
            return current;
        }
        current = current->next;

    }
    Serial.println(F("Found no nonce"));
    return 0;
}

bool TooSigning_received_noncelist_add(uint8_t passed_nonce_from, uint32_t passed_nonce) {
    Serial.println(F("Received nonce list add"));
    NonceReceived *current = nonce_received_start;
    Serial.println(F("Received nonce preparing"));
    Serial.println(F("Searching for nonce list last"));
    TooSigning_received_noncelist_print();

    if (current == 0) {
        if (!TooSigning_received_noncelist_initialize()) {
            return false;
        }
        current = nonce_received_start;
    } else {
        while (current->next != 0) {
            current = current->next;
        }
        current->next = malloc(sizeof(NonceReceived));
        current = current->next;
        if (current == 0) {
            return false;
        }
    }

    current->nonce_from = passed_nonce_from;
    current->nonce = passed_nonce;
    current->nonce_when = millis();
    current->next = 0;
    TooSigning_received_noncelist_print();
    TooSigning_requested_noncelist_received(passed_nonce_from);
    return true;
}

void TooSigning_received_noncelist_remove(NonceReceived *previous, NonceReceived *current) {
    Serial.println(F("Received nonce list remove"));
    TooSigning_received_noncelist_print();
    if (previous == 0) {
        free(current);
        nonce_received_start = 0;
    } else {
        previous->next = current->next;
        free(current);
    }
    TooSigning_received_noncelist_print();
}

void TooSigning_received_noncelist_remove_timeout() {
    NonceReceived *current = nonce_received_start;
    NonceReceived *previous = 0;
    while (current != 0) {
        Serial.println(F("First not empty"));
        TooSigning_received_noncelist_print();
        if (millis() - current->nonce_when > 5000) {
            Serial.println(F("Received nonce timeout: "));
            Serial.println(current->nonce_when);
            TooSigning_received_noncelist_remove(previous, current);
        }
        previous = current;
        current = current->next;
        TooSigning_received_noncelist_print();
    }
}


void TooSigning_read_hmac_from_progmem(uint8_t nodeID, void *hmac_pointer) {
    uint8_t hmac[20] = {0};
    uint8_t first_address_hmac = 20 * nodeID;
    Serial.println(nodeID);
    for (uint8_t offset = 0; offset < 20; offset++) {
        uint8_t character = pgm_read_byte_near(&(hmacs[nodeID][offset]));
        Serial.print(character);
        Serial.print(F(" "));
        memmove(((uint8_t * ) & hmac) + offset, &character, 1);
    }
    memmove(hmac_pointer, hmac, sizeof(hmac));
    Serial.println();
}


void TooSigning_init_hmac(uint8_t* hmac, uint8_t length){
    Sha256.initHmac(hmac, length);
}

uint8_t * TooSigning_get_hmac(){
    return Sha256.resultHmac();
}
/*
  Intercepting signing payloads
*/
bool TooSigning_unsigned_network_available(void) {
    //Serial.println(F(","));
    if (network.available()) {
        Serial.print(F("NETWORK RECEIVE: "));
        RF24NetworkHeader header;
        network.peek(header);
        Serial.println((char) header.type);
        switch (header.type) { // Is there anything ready for us?
            case 'S': { //"S" like "you sent me something signed"
                Serial.print(F("S Time: "));
                Serial.println(millis());
                Payload_MetadataSigned_Received payload;

                network.peek(header, &payload, sizeof(Payload_MetadataSigned_Received));
                uint8_t nodeID = mesh.getNodeID(header.from_node);

                Serial.print(F("From: "));
                Serial.println(nodeID);
                Serial.print(F("To: "));
                Serial.println(current_node_ID);


                uint8_t hmac[20] = {0};
                TooSigning_read_hmac_from_progmem(nodeID, &hmac);
                if (hmac[0] == hmacs[0][0]) {
                    Serial.println(F("Equal"));
                }

                Serial.print(F("HMAC: "));
                Serial.println(hmacs[0][0], DEC);
                Serial.println(hmac[0], DEC);

                for (int i; i > 20; i++) {
                    Serial.print(hmac[i]);
                }

                Serial.println();

                Sha256.initHmac(hmac, 20);

                Serial.print(F("Size of payload: "));
                Serial.println(payload.payload_size);

                Serial.print(F("Metadata: "));
                TooSigning_random_data_print(&payload, sizeof(Payload_MetadataSigned_Received));

                size_t sizeoffullbuffer = sizeof(Payload_MetadataSigned_Received) + payload.payload_size;

                void *buf = malloc(sizeoffullbuffer);
                network.peek(header, buf, sizeoffullbuffer);

                Serial.print(F("Full buffer: "));
                TooSigning_random_data_print(buf, sizeoffullbuffer);
                TooSigning_hash_data((uint8_t *) buf + sizeof(Payload_MetadataSigned_Received), payload.payload_size);

                NonceSent *tempnonce = TooSigning_sent_noncelist_find_from_ID(nodeID);
                if (tempnonce != 0) {
                    Serial.print(F("Nonce used: "));
                    Serial.println(tempnonce->nonce);
                    TooSigning_hash_data(&(tempnonce->nonce), sizeof(uint32_t));
                } else {
                    Serial.println(F("Nonce not found!"));
                    return false; //TODO WARNING: Just keeping the message in the buffer
                }

                uint8_t calculated_hash[32];
                TooSigning_hash_store(Sha256.resultHmac(), calculated_hash);


                Serial.print(F("Calculated hash: "));
                TooSigning_hash_print(calculated_hash);
                Serial.print(F("Received hash: "));
                TooSigning_hash_print(payload.payload_hash);

                free(buf);
                if (TooSigning_hash_compare(calculated_hash, payload.payload_hash)) {
                    Serial.println(F("EQUAL HASH?!"));
                } else {
                    Serial.println(F("Inequal hash!"));
                    return false;
                }

                return true;
            }
            case 'R': { //"R" like "send me a nonce"
                Serial.print(F("R Time: "));
                Serial.println(millis());


                NoncePayload payload;
                RF24NetworkHeader received_header;

                network.read(received_header, &payload,
                             sizeof(NoncePayload));  //We just wanted to know the type of the message, discard the content
                uint32_t time = millis();
                payload.nonce = time;
                Serial.print(F("Sent nonce: "));
                Serial.println(payload.nonce);
                Serial.print(F("To node: "));
                uint16_t nodeID = mesh.getNodeID(received_header.from_node);
                Serial.println(nodeID);
                Serial.println(F("Switch"));
                bool state = mesh.write(&payload, 'N', sizeof(payload), nodeID);
                if (state) {
                    TooSigning_sent_noncelist_add(nodeID, time);
                    Serial.println(F("Nonce stored"));
                    Serial.println(F("Completed nonce sending"));
                    TooSigning_sent_noncelist_print();
                    return false;
                }
                return false;
            }
            case 'N': { //"N" like "you sent me a nonce"
                Serial.print(F("N Time: "));
                Serial.println(millis());

                NoncePayload payload_nonce;
                RF24NetworkHeader header;

                network.read(header, &payload_nonce, sizeof(NoncePayload));
                Serial.print(F("Recived nonce: "));
                Serial.println(payload_nonce.nonce);
                Serial.print(F("From node: "));
                uint16_t nodeID = mesh.getNodeID(header.from_node);
                Serial.println(nodeID);
                TooSigning_received_noncelist_add(nodeID, payload_nonce.nonce);
                return false;
            }
            default: {
                Serial.print(F("Received message with type '"));
                Serial.print(header.type);
                Serial.println(F("'"));
                return true;
            }
        }
    }
    return false;
}

/*
  Signed network maintenance
*/
uint32_t network_maintenance_timer = 0;
uint32_t nonce_retry_timer = 0;

void TooSigning_signed_network_update() {
    mesh.update();

    if (millis() - network_maintenance_timer > 500) {
        Serial.println(F("Maintenance"));
        if (nonce_received_start != 0) {
            Serial.println(F("1: Checking for received nonce timeouts"));
            TooSigning_received_noncelist_remove_timeout();
        }

        if (nonce_sent_start != 0) {
            Serial.println(F("2: Checking for sent nonce timeouts"));
            TooSigning_sent_noncelist_remove_timeout();
        }

        if (nonce_requested_start != 0) {
            Serial.println(F("3: Checking for requested timeouts"));
            TooSigning_requested_noncelist_remove_timeout();
        }

        network_maintenance_timer = millis();
    }

    mesh.update();
    //Serial.println(F("d"));
    if (millis() - nonce_retry_timer > 2000) {
        if (nonce_requested_start != 0) {
            Serial.println(F("Requesting nonces for all"));
            TooSigning_requested_noncelist_retry_all();
        }
        nonce_retry_timer = millis();
    }

    mesh.update();
}

void TooSigning_signed_network_begin(uint8_t nodeID){
  TooSigning_received_noncelist_initialize();
  TooSigning_requested_noncelist_initialize();
  TooSigning_sent_noncelist_initialize();
}
