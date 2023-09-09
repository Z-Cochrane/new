/*
 * spi.h
 *
 *  Created on: 3 paź 2018
 *      Author: Kasia_
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

#define SPI_DDR     DDRB
#define SPI_PORT    PORTB
#define SPI_PIN     PINB

#define SS 		PB2			//CS = CSN = D10 = SS
#define MOSI	PB3
#define MISO	PB4
#define SCK		PB5

#define CSN_SS_0()	(SPI_PORT &= ~(1<<SS))
#define CSN_SS_1()	(SPI_PORT |= (1<<SS))

void spi_init(void);

uint8_t spi_transmit_byte(uint8_t data);
void spi_send_byte(uint8_t data);
void spi_send_buffer(uint8_t * data_buffer_out, uint8_t length);
void spi_transmit_buffer(uint8_t * data_buffer_in, uint8_t * data_buffer_out,
		uint8_t length);

#endif /* SPI_H_ */
