/*
    A signing library for TooAutomations
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

/**
 * @file hmacs.c
 *
 * This file contains the secret HMAC keys used
 * if signing is enabled
 */

#include <stdint.h>
#include <Arduino.h>

#pragma once

#ifndef __HMACS_H__
#define __HMACS_H__

//This array contains the hmacs used by the library, currently the index is equal to the node ID
static const uint8_t hmacs[2][20] PROGMEM = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
};

#endif
