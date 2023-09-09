/*
 * spi.c
 *
 *  Created on: 3 paź 2018
 *      Author: Kasia_
 */
#include <avr/io.h>

#include "spi.h"

void spi_init(void) {

	SPI_PORT |= (1 << SS) | (1 << MISO);
	SPI_DDR |= (1 << SS) | (1 << MOSI) | (1 << SCK);

	/* aktywacja  SPI, tryb pracy Master, Fosc/8 */
#if defined(__AVR_ATmega8__)

// SPIE=0, spi enable, master, spr0=1: 12/8 = 1,5 MHz
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
	SPSR |= (1 << SPI2X);
#endif

#if defined(__AVR_ATmega328P__)

// SPIE0=0, spi enable, master, spr00=1: 16/8*2 = 1 MHz
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
#endif
}

//Function: SPI_RW();
uint8_t spi_transmit_byte(uint8_t data) {

	spi_send_byte(data);
	return SPDR; //return data from the SPDR (here are now data from the nRF)
}

void spi_send_byte(uint8_t data) {
	SPDR = data;          		//send byte to SPDR register
	while (!(SPSR & (1 << SPIF)))
		; //wait for data shifting
}

void spi_send_buffer(uint8_t * data_buffer_out, uint8_t length) {
	uint8_t * pointer;				//declare pointer
	uint8_t i;						//declare indexing variable
	pointer = data_buffer_out;//pointer is showing on first byte of "data_buffer_out"
	for (i = 0; i < length; i++)	//until "i" is smaller than "length"
			{
		spi_send_byte(*pointer++);	//send bytes to nRF
	}
}

void spi_transmit_buffer(uint8_t * data_buffer_in,
		uint8_t * data_buffer_out, uint8_t length) {
	uint8_t * pointer1;				//declare pointer1
	uint8_t * pointer2;				//declare pointer2
	uint8_t i;						//declare indexing variable

	pointer1 = data_buffer_out;	//pointer1 is showing on first byte of "data_buffer_out"
	pointer2 = data_buffer_in; //pointer2 is showing on first byte of "data_buffer_in"
	for (i = 0; i < length; i++)	//until "i" is smaller than "length"
			{
		*pointer2++ = spi_transmit_byte(*pointer1++); //send bytes to nRF and read data back from nRF
	}
}

