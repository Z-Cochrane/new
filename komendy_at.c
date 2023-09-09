/*
 * komendy_at.c  
 *
 *  Created on: 13-03-2012
 *      Author: Mirosław Kardaś
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "mkuart.h"
#include "komendy_at.h"

#include "nrf24.h"
#include "ds18x20.h"
#include "power_save.h"

#define AT_CNT 	7	// Ilość poleceń AT

//----------- tablica z poleceniami AT i wskaźnikami funkcji do ich obsługi --------------------
const TATCMD polecenia_at[AT_CNT] PROGMEM = {
		// { at_cmd } , { wskaźnik do funkcji obsługi at },
		{ "AT", at_service }, { "ATI", ati_service },
		{ "AT+RST", at_rst_service }, { "AT+STA", at_sta_service }, { "AT+CH",
				at_ch_service }, { "AT+REG", at_reg_service }, { "AT+ADD",
				at_add_service }, };

//----------------- funkcja do analizowania danych odebranych z UART ------------------------------
void parse_uart_data(char * pBuf) {

	int8_t (*_at_srv)(uint8_t inout, char * data);

	char * cmd_wsk;
	char * reszta;
	uint8_t i = 0, len;

	//digi lion start
	//todo: digi lion3

	if (0
			== ((strncasecmp(pBuf, "LION", 4)) && (strncasecmp(pBuf, "LBAT", 4)))) {

		if (int1_flag) {
			uart_puts(pBuf);
			uart_puts("\r\n");
		}

#if TESTY == 1
		uart_puts(pBuf);
		uart_puts("\r\n");
#endif

		cmd_wsk = strtok_r(pBuf, ":", &reszta);
		nRF_TX_buf[LION_INFO_FL] = strcasecmp(cmd_wsk, "LION");

		cmd_wsk = strtok_r(NULL, ",", &reszta);
		nRF_TX_buf[VBAT_U] = atoi(cmd_wsk);

		cmd_wsk = strtok_r(NULL, "V", &reszta);
		nRF_TX_buf[VBAT_F] = atoi(cmd_wsk);

		cmd_wsk = strtok_r(NULL, ",", &reszta);
		nRF_TX_buf[LION_STATUS_FL] = strcasecmp(cmd_wsk, " OFF");

//		nRF_TX_buf[LION_STATUS_FL] = strcasecmp(reszta,"OFF");


#if TESTY == 1

		len = strlen(cmd_wsk);

		uart_puts(cmd_wsk);
		uart_puts("\r\n");
		uart_putint(len,10);
		uart_puts("\r\n");

		for (uint8_t i=LION_INFO_FL;i<LION_STATUS_FL+1;i++) {
			uart_putint(nRF_TX_buf[i],10);
			uart_putc('[');
			uart_putint(i,10);
			uart_putc(']');
		}
		uart_puts("\r\n");
#endif


		/*
		 *	-------------------------------
		 *
		 *
		 *		nrf_power_up();
		 *		nrf_send_string(pBuf);
		 *
		 *		-------------------------------
		 *
		 */

	} else

	//digi lion stop

//		 Wyświetlamy odebrane polecenia AT na LCD

	if (strpbrk(pBuf, "=?")) {
		// obsługa poleceń AT we/wy + parametry

		if (strpbrk(pBuf, "?")) {
			// zapytania do układu w postaci: AT+CMD?

			cmd_wsk = strtok_r(pBuf, "?", &reszta);
			len = strlen(cmd_wsk);
			for (i = 0; i < AT_CNT; i++) {
				if (len
						&& 0
								== strncasecmp_P(cmd_wsk,
										polecenia_at[i].polecenie_at, len)) {
					if (pgm_read_word(polecenia_at[i].polecenie_at)) { // <--- UWAGA! w tekście książki zabrakło pgm_read_word()
						_at_srv = (void *) pgm_read_word(
								&polecenia_at[i].at_service);
						if (_at_srv) {
							if (_at_srv(0, reszta) < 0)
								uart_puts_P(PSTR("ERROR\r\n"));
						}
					}
					uart_puts("\r\n");
					break;
				}
			}

		} else {
			// ustawienia układu w postaci: AT+CMD=parametry

			cmd_wsk = strtok_r(pBuf, "=", &reszta);
			len = strlen(cmd_wsk);
			for (i = 0; i < AT_CNT; i++) {
				if (len
						&& 0
								== strncasecmp_P(cmd_wsk,
										polecenia_at[i].polecenie_at, len)) {
					if (pgm_read_word(polecenia_at[i].polecenie_at)) { // <--- UWAGA! w tekście książki zabrakło pgm_read_word()
						_at_srv = (void *) pgm_read_word(
								&polecenia_at[i].at_service);
						if (_at_srv && !_at_srv(1, reszta))
							uart_puts_P(PSTR("OK\r\n"));
						else
							uart_puts_P(PSTR("ERROR\r\n"));
					}
					break;
				}
			}
		}

	} else {
		// obsługa poleceń AT bez parametrów

		if (0 == pBuf[0])
			uart_puts("\r\n"); // reakcja na znak CR lub CRLF z terminala
		else {
			for (i = 0; i < AT_CNT; i++) {
				if (0
						== strncasecmp_P(pBuf, polecenia_at[i].polecenie_at,
								strlen(pBuf))) {
					if (pgm_read_word(polecenia_at[i].polecenie_at)) { // <--- UWAGA! w tekście książki zabrakło pgm_read_word()
						_at_srv = (void *) pgm_read_word(
								&polecenia_at[i].at_service);
						if (_at_srv)
							_at_srv(2, 0);
					}
					break;
				}
			}
		}
	}

	if ( AT_CNT == i)
		uart_puts_P(PSTR("ERROR\r\n"));
}

//----------------- obsługa poszczególnych komend AT ----------------------------------
int8_t at_service(uint8_t inout, char * params) {
	uart_puts_P(PSTR("OK\r\n"));
	return 0;
}

int8_t ati_service(uint8_t inout, char * params) {
	uart_puts_P(PSTR("\r\n+-----------------------+\r\n"));
	uart_puts_P(PSTR("| nRF UART konfigurator |\r\n"));
	uart_puts_P(PSTR("|                       |\r\n"));
	uart_puts_P(PSTR("| "));
	uart_puts_P(PSTR(__DATE__ ", " __TIME__));
	uart_puts_P(PSTR(" |\r\n|(c) Daniel Parzniewski |\r\n"));
	uart_puts_P(PSTR("+-----------------------+\r\n"));
	uart_puts("\r\n");
	uart_puts_P(PSTR("AT,ATI,+RST,+STA,+CH,+REG,+ADD\r\n"));
	uart_puts_P(
			PSTR(
					"uzycie: AT+CH? - odczyt konf.; AT+CH - parametry wywolania\r\n"));
	uart_puts_P(PSTR("AT+CH=x - zapis konf., gdzie x - numer kanalu\r\n"));

	return 0;
}

int8_t at_rst_service(uint8_t inout, char * params) {

	uart_puts_P(PSTR("restart za 2 sekundy...\r\n"));
	_delay_ms(2000);

	cli();
	// wyłącz przerwania
	wdt_enable(0); // ustaw watch-dog
	while (1)
		;		// czekaj na RESET

	return 0;
}

int8_t at_sta_service(uint8_t inout, char * params) {

	uint8_t reg;
//	uint8_t tab[TX_ADDRESS_LENGTH];

	uart_puts_P(PSTR("+STA:\r\n"));

	reg = nrf_read_register(CONFIG);
	uart_puts_P(PSTR("Rejestr CONFIG\r\n"));

	uart_puts_P(PSTR("RX_DR IRQ: "));
	if (reg & (1 << MASK_RX_DR))
		uart_puts_P(PSTR("OFF\r\n"));
	else
		uart_puts_P(PSTR("ON\r\n"));

	uart_puts_P(PSTR("TX_DS IRQ: "));
	if (reg & (1 << MASK_TX_DS))
		uart_puts_P(PSTR("OFF\r\n"));
	else
		uart_puts_P(PSTR("ON\r\n"));

	uart_puts_P(PSTR("MAX_RT IRQ: "));
	if (reg & (1 << MASK_MAX_RT))
		uart_puts_P(PSTR("OFF\r\n"));
	else
		uart_puts_P(PSTR("ON\r\n"));

	uart_puts_P(PSTR("CRC Enable: "));
	uart_putint((reg & (1 << EN_CRC)) >> 3, 10);
	uart_puts("\r\n");

	uart_puts_P(PSTR("CRC encoding: "));
	if (reg & (1 << CRC0))
		uart_puts_P(PSTR("2 bajty\r\n"));
	else
		uart_puts_P(PSTR("1 bajt\r\n"));

	uart_puts_P(PSTR("Tryb: POWER_"));
	if (reg & (1 << PWR_UP))
		uart_puts_P(PSTR("UP\r\n"));
	else
		uart_puts_P(PSTR("DOWN\r\n"));

	reg = nrf_read_register(EN_AA);
	uart_puts_P(PSTR("Enable AA p: "));

	for (uint8_t i = 0; i < 7; i++) {

		if ((reg & (1 << i))) {
			uart_putint(i, 10);
			uart_putc(';');
		}
	}
	uart_puts("\r\n");

	reg = nrf_read_register(EN_RX_ADDR);
	uart_puts_P(PSTR("Enable RX p: "));

	for (uint8_t i = 0; i < 7; i++) {

		if ((reg & (1 << i))) {
			uart_putint(i, 10);
			uart_putc(';');
		}
	}
	uart_puts("\r\n");

	/*	reg = nrf_read_register(RF_CH);
	 uart_puts_P(PSTR("CH: "));
	 uart_putint(reg, 10);*/

	reg = nrf_read_register(RF_SETUP);
	uart_puts_P(PSTR("\r\nDATA RATE: "));
	if (4 == ((reg & 0x28) >> 3))
		uart_puts_P(PSTR("250 kbps\r\n"));
	else if (0 == ((reg & 0x28) >> 3))
		uart_puts_P(PSTR("1 Mbps\r\n"));
	else
		uart_puts_P(PSTR("2 Mbps\r\n"));

	uart_puts_P(PSTR("RF power: "));
	if (3 == ((reg & 0x06) >> 1))
		uart_puts_P(PSTR("0 dBm\r\n"));
	else if (2 == ((reg & 0x06) >> 1))
		uart_puts_P(PSTR("-6 dBm\r\n"));
	else if (1 == ((reg & 0x06) >> 1))
		uart_puts_P(PSTR("-12 dBm\r\n"));
	else
		uart_puts_P(PSTR("-18 dBm\r\n"));

	reg = nrf_get_status();
	uart_puts_P(PSTR("Rejestr STATUS\r\n"));

	uart_puts_P(PSTR("RX_DR: "));
	uart_putint((reg & (1 << RX_DR)) >> 6, 10);
	uart_puts("\r\n");
	uart_puts_P(PSTR("TX_DS: "));
	uart_putint((reg & (1 << TX_DS)) >> 5, 10);
	uart_puts("\r\n");
	uart_puts_P(PSTR("MAX_RT: "));
	uart_putint((reg & (1 << MAX_RT)) >> 4, 10);
	uart_puts("\r\n");
	uart_puts_P(PSTR("RX_P_NO: "));
	uart_putint((reg & 0x0E) >> 1, 10);
	uart_puts("\r\n");
	uart_puts_P(PSTR("TX_FULL: "));
	uart_putint(reg & (1 << TX_FULL), 10);
	uart_puts("\r\n");

	reg = nrf_read_register(OBSERVE_TX);
	uart_puts_P(PSTR("Rejestr OBSERVE_TX:\r\n"));
	uart_puts_P(PSTR("Lost packets: "));
	uart_putint((reg & (0xF0)) >> 4, 10);
	uart_puts("\r\n");

	uart_puts_P(PSTR("Retransmitted packets: "));
	uart_putint((reg & 0x0F), 10);
	uart_puts("\r\n");
	/*
	 nrf_read(TX_ADDR, tab, TX_ADDRESS_LENGTH);

	 uart_puts_P(PSTR("TX: "));
	 for (uint8_t i = 0; i < TX_ADDRESS_LENGTH; i++) {
	 uart_putint(tab[i], 16);
	 uart_putc(' ');
	 }
	 uart_puts("\r\n");

	 nrf_read(RX_ADDR_P0, tab, RX_ADDRESS_LENGTH);

	 uart_puts_P(PSTR("RX[0]: "));
	 for (uint8_t i = 0; i < RX_ADDRESS_LENGTH; i++) {
	 uart_putint(tab[i], 16);
	 uart_putc(' ');
	 }
	 uart_puts("\r\n");

	 nrf_read(RX_ADDR_P1, tab, RX_ADDRESS_LENGTH);

	 uart_puts_P(PSTR("RX[1]: "));
	 for (uint8_t i = 0; i < RX_ADDRESS_LENGTH; i++) {
	 uart_putint(tab[i], 16);
	 uart_putc(' ');
	 }
	 uart_puts("\r\n");
	 */
	reg = nrf_read_register(FIFO_STATUS);
	uart_puts_P(PSTR("Rejestr FIFO_STATUS\r\n"));
	uart_puts_P(PSTR("TX_FULL: "));
	uart_putint((reg & (1 << FIFO_FULL)) >> 5, 10);
	uart_puts("\r\n");

	uart_puts_P(PSTR("TX_EMPTY: "));
	uart_putint((reg & (1 << TX_EMPTY)) >> 4, 10);
	uart_puts("\r\n");

	uart_puts_P(PSTR("RX_FULL: "));
	uart_putint((reg & (1 << RX_FULL)) >> 1, 10);
	uart_puts("\r\n");

	uart_puts_P(PSTR("RX_EMPTY: "));
	uart_putint((reg & (1 << RX_EMPTY)), 10);
	uart_puts("\r\n");

	reg = nrf_read_register(FEATURE);
	uart_puts_P(PSTR("FEATURE\r\n"));

	uart_puts_P(PSTR("EN_DYN_ACK: "));
	uart_putint((reg & (1 << EN_DYN_ACK)), 10);
	uart_puts("\r\n");

	uart_puts_P(PSTR("EN_ACK_PAY: "));
	uart_putint((reg & (1 << EN_ACK_PAY)) >> 1, 10);
	uart_puts("\r\n");

	uart_puts_P(PSTR("EN_DPL: "));
	uart_putint((reg & (1 << EN_DPL)) >> 2, 10);

	reg = nrf_read_register(DYNPD);
	uart_puts_P(PSTR(", dla p: "));

	for (uint8_t i = 0; i < 7; i++) {

		if ((reg & (1 << i))) {
			uart_putint(i, 10);
			uart_putc(';');
		}
	}
	uart_puts("\r\n");
	uart_puts_P(PSTR("(wymaga ENAA)\r\n"));

	return 0;
}

int8_t at_ch_service(uint8_t inout, char * params) {

	char * wsk;

//zapis do
	if (1 == inout) {
		// wyłuskujemy pierwszy parametr
		wsk = strtok(params, " ");
		// sprawdzamy czy są parametry, jeśli nie to błąd
		if (!strlen(wsk))
			return -1;
		nrf_ch = atoi(wsk);

		nrf_set_channel(nrf_ch);

		uart_puts_P(PSTR("+CH: "));
		uart_putint(nrf_ch, 10);
		uart_puts("\r\n");
		//0: odczyt z
	} else if (!inout) {

		nrf_ch = nrf_read_register(RF_CH);

		uart_puts_P(PSTR("+CH: "));
		uart_putint(nrf_ch, 10);
		uart_puts("\r\n");
		//2 - info
	} else if (2 == inout) {
		uart_puts_P(PSTR("AT+CH = (1-9)\r\n"));
	}

	return 0;
}

int8_t at_reg_service(uint8_t inout, char * params) {

	uint8_t reg, val;
	char * wsk;

	//zapis
	if (1 == inout) {
		// sprawdzamy czy są parametry, jeśli nie to błąd
		if (!strlen(params))
			return -1;
		// wyłuskujemy pierwszy parametr do przecinka
		wsk = strtok(params, ",");
		// sprawdzamy czy są parametry, jeśli nie to błąd
		if (!strlen(wsk))
			return -1;
		// zamieniamy liczbę ASCII na wartość dziesiętną
		reg = atoi(wsk);
		// jeśli Y  nie mieści się w zakresie od 0 do 3 to błąd
		if (!(reg >= 0 && reg < 25))
			return -1;
		// wyłuskujemy drugi parametr do przecinka
		wsk = strtok(0, ",");
		// sprawdzamy czy są parametry, jeśli nie to błąd
		if (!strlen(wsk))
			return -1;
		// zamieniamy liczbę ASCII na wartość dziesiętną
		val = atoi(wsk);
		// jeśli X  nie mieści się w zakresie od 0 do 39 to błąd
		if (!(val >= 0 && val < 255))
			return -1;
		uart_puts_P(PSTR("+REG: "));

		nrf_power_down();

		nrf_write_register(reg, val);

		nrf_set_rx_mode();

		uart_putint(reg, 16);
		uart_putc('=');
		uart_putint(val, 16);
		uart_puts("\r\n");

	} else if (2 == inout) {
		uart_puts_P(PSTR("AT+REG = (0-25), (0-255)\r\n"));
	} else {
		if (!inout) {
			// sprawdzamy czy są parametry, jeśli nie to błąd
			if (!strlen(params))
				return -1;
			// wyłuskujemy pierwszy parametr do przecinka
			wsk = strtok(params, ",");
			// sprawdzamy czy są parametry, jeśli nie to błąd
			if (!strlen(wsk))
				return -1;
			// zamieniamy liczbę ASCII na wartość dziesiętną
			reg = atoi(wsk);
			// jeśli Y  nie mieści się w zakresie od 0 do 3 to błąd
			if (!(reg >= 0 && reg < 25))
				return -1;

			uart_puts_P(PSTR("+REG: "));

			val = nrf_read_register(reg);

			uart_putint(reg, 16);
			uart_puts_P(PSTR("="));
			uart_putint(val, 16);
			uart_puts("\r\n");
		}
	}

	return 0;
}

int8_t at_add_service(uint8_t inout, char * params) {

	uint8_t reg = 0;
	uint8_t val0;
	uint8_t val[TX_ADDRESS_LENGTH];
	char * wsk;

	//zapis
	if (1 == inout) {
		// sprawdzamy czy są parametry, jeśli nie to błąd
		if (!strlen(params))
			return -1;
		// wyłuskujemy pierwszy parametr do przecinka
		wsk = strtok(params, ",");
		// sprawdzamy czy są parametry, jeśli nie to błąd
		if (!strlen(wsk))
			return -1;
		// zamieniamy liczbę ASCII na wartość dziesiętną
//		reg = atoi(wsk);
		if (wsk[0] == 'T' || wsk[0] == 't')
			reg = 0x10;
		else if (wsk[2] == '1')
			reg = 0x0B;

		// jeśli Y  nie mieści się w zakresie od 0 do 3 to błąd
		if (!(reg >= 0x0A && reg < 0x11))
			return -1;

		for (uint8_t i = 0; i < TX_ADDRESS_LENGTH; i++) {

			// wyłuskujemy drugi do 6 parametry do przecinka
			wsk = strtok(0, ",");
			// sprawdzamy czy są parametry, jeśli nie to błąd
			if (!strlen(wsk))
				return -1;

			// zamieniamy liczbę ASCII na wartość dziesiętną
			val[i] = atoi(wsk++);

			// jeśli X  nie mieści się w zakresie od 0 do 255 to błąd
			if (!(val[i] >= 0 && val[i] < 255))
				return -1;
		}

		uart_puts_P(PSTR("+ADD: "));

		nrf_power_down();

		if (0x10 == reg)

			nrf_set_tx_address(val);

		else if (0x0B == reg)
			nrf_set_rx_address(1, val);

		nrf_set_rx_mode();

		uart_putint(reg, 16);
		uart_putc('=');

		for (uint8_t i = 0; i < TX_ADDRESS_LENGTH; i++) {
			uart_putint(val[i], 16);
			uart_putc(' ');
		}
		uart_puts("\r\n");
//info
	} else if (2 == inout) {
		uart_puts_P(PSTR("AT+ADD = (TX/RX1-5), (0-255),...,... (5x)\r\n"));
	} else {
		//odczyt
		if (!inout) {
			/*			// sprawdzamy czy są parametry, jeśli nie to błąd
			 if (!strlen(params))
			 return -1;
			 // wyłuskujemy pierwszy parametr do przecinka
			 wsk = strtok(params, ",");
			 // sprawdzamy czy są parametry, jeśli nie to błąd
			 if (!strlen(wsk))
			 return -1;
			 // zamieniamy liczbę ASCII na wartość dziesiętną
			 reg = atoi(wsk);
			 // jeśli Y  nie mieści się w zakresie od 0 do 3 to błąd
			 if (!(reg >= 0 && reg < 25))
			 return -1;*/

			uart_puts_P(PSTR("+ADD:\r\n"));

			nrf_read(TX_ADDR, val, TX_ADDRESS_LENGTH);

//			uart_putint(reg, 16);
			uart_puts_P(PSTR("TX= "));
			for (uint8_t i = 0; i < TX_ADDRESS_LENGTH; i++) {
				uart_putint(val[i], 16);
				uart_putc(' ');
			}
			uart_puts("\r\n");

			for (uint8_t i = 0; i < 2; i++) {

				nrf_read(RX_ADDR_P0 + i, val, RX_ADDRESS_LENGTH);

//			uart_putint(reg, 16);
				uart_puts_P(PSTR("RX"));
				uart_putint(i, 10);
				uart_puts_P(PSTR("= "));
				for (uint8_t j = 0; j < RX_ADDRESS_LENGTH; j++) {
					uart_putint(val[j], 16);
					uart_putc(' ');
				}
				uart_puts("\r\n");
			}

			for (uint8_t i = 2; i < 6; i++) {

//				nrf_read(RX_ADDR_P0 + i, reg+0, 1);
				val0 = nrf_read_register(RX_ADDR_P0 + i);

//			uart_putint(reg, 16);
				uart_puts_P(PSTR("RX"));
				uart_putint(i, 10);
				uart_puts_P(PSTR("= "));
				uart_putint(val0, 16);

				uart_puts("\r\n");
			}
		}
	}

	return 0;
}
