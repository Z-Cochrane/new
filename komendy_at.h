/*
 * komendy_at.h  
 *
 *  Created on: 13-03-2012
 *      Author: Mirosław Kardaś
 */

#ifndef KOMENDY_AT_H_
#define KOMENDY_AT_H_

#define TESTY	0


// definicja typu strukturalnego
typedef struct {
	char polecenie_at[8];
	int8_t (* at_service)(uint8_t inout, char * params);
} const TATCMD;


// deklaracje zmiennych zewnętrznych
extern TATCMD polecenia_at[] PROGMEM;

// deklaracje funkcji
void parse_uart_data( char * pBuf );

int8_t at_service(uint8_t inout, char * params);
int8_t ati_service(uint8_t inout, char * params);
int8_t at_rst_service(uint8_t inout, char * params);

int8_t at_sta_service(uint8_t inout, char * params);
int8_t at_ch_service(uint8_t inout, char * params);
int8_t at_reg_service(uint8_t inout, char * params);
int8_t at_add_service(uint8_t inout, char * params);

#endif /* KOMENDY_AT_H_ */
