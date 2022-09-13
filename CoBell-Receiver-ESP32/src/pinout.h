#ifndef CS_PINOUT_H
#define CS_PINOUT_H

#include <Arduino.h>

/**
 * ESP23            nRF24L01
 * -----+          +-----
 * GND  |          | IRQ
 * IO23 |<-------->| MOSI
 * IO19 |<-------->| MISO
 * IO18 |<-------->| SCK
 * IO05 |<-------->| CSN
 * IO17 |<-------->| CE
 * -----+          +-----
 */
#define RF24_CE   17
#define RF24_CSN  SS
#define RF24_MOSI MOSI
#define RF24_MISO MISO
#define RF24_IRQ  22

#endif