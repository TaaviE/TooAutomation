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
 * @file TooNetworking.h
 *
 * This file implements the necessary functionality
 * that allow nodes in the network to communicate
 */

#pragma once
#include "TooNetworking_data.h"
#include "RF24Network.h"

#ifndef __TooNetworking_H__
#define __TooNetworking_H__

bool TooNetworking_connection_begin(uint8_t passed_node_id);

bool TooNetworking_send(uint8_t for_node, void * payload, uint8_t size, uint8_t identifier);

bool TooNetworking_other_node_is_online(uint8_t nodeID);

bool TooNetworking_this_node_is_online();

bool TooNetworking_send_signed_(uint8_t for_node, void * payload, uint8_t size, uint32_t nonce);

bool TooNetworking_bufferlist_initialize();

bool TooNetworking_connection_available();

bool TooNetworking_bufferlist_send_(BufferItem * current, BufferItem * previous);

void TooNetworking_bufferlist_add(uint8_t payload_destination, void * payload, uint8_t size, uint8_t type);

bool TooNetworking_send_signed(uint8_t for_node, void * payload, uint8_t size);

bool TooNetworking_send_encrypted(uint8_t for_node, void * payload, uint8_t size);

bool TooNetworking_send_signed_encrypted(uint8_t for_node, void * payload, uint8_t size);

void TooNetworking_peek(RF24NetworkHeader& header, void* message, uint16_t maxlen);

bool TooNetworking_read(RF24NetworkHeader& header, void* message, uint16_t maxlen);

bool TooNetworking_connection_check();

bool TooNetworking_connection_fix(void);

void TooNetworking_bufferlist_print();

void TooNetworking_bufferlist_remove(BufferItem * previous, BufferItem * current);

BufferItem * TooNetworking_bufferlist_find_for_id(uint8_t nodeID);

bool TooNetworking_bufferlist_send_signed(uint8_t for_node, void * payload, uint8_t size);

bool TooNetworking_send_encrypted(uint8_t for_node, void * payload, uint8_t size);

bool TooNetworking_send_signedencrypted(uint8_t for_node, void * payload, uint8_t size);

void TooNetworking_bufferlist_send_all();

bool TooNetworking_connection_maintenance();

bool TooNetworking_begin(uint8_t passed_node_id);

#include "TooNetworking.cpp"

#endif // __TooNetworking_H__
