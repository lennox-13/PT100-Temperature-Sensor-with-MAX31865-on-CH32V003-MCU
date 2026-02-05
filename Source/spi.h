#ifndef __SPI_H
#define __SPI_H

#include <ch32v00x.h>

// -----------------------------------------------------------------------------
// SPI1 driver interface
// -----------------------------------------------------------------------------

// Initialize SPI1 peripheral and SPI1 GPIO pins
void SPI1_Init(void);

// Transfer a single byte over SPI
// Returns the received byte; returns 0 if a timeout occurs
uint8_t SPI_TransferByte(uint8_t data);

#endif // __SPI_H
