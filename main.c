/*                  e-gadget.header
 * main.c
 *
 *  Created on: 2020-09-16
 *    Modyfied: 2020-09-29 12:38:30
 *      Author: Danny
 *
 * Project name: "NRF24_N_SOLAR_LION3"
 *
 *          MCU: ATmega328P
 *        F_CPU: 16 000 000 Hz
 *
 *    Flash: 8 330 bytes   [ 25,4 % ]
 *      RAM:  252 bytes   [ 12,3 % ]
 *   EEPROM:  0 bytes   [ 0,0 % ]
 *
 */

/*
 * 	TESTY == 1:
 *
 * 	flash: 8800 (+340)
 * 	ram: 256 (+2)
 *
 *
 */
 
 /*
  *
  * Temperatura zewnętrzna plus pewne dane telemetryczna dla testu zasięgu (nadajnik zasilany z baterii słonecznej)
  *
  */

#include <avr/io.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#include <avr/sleep.h>

#include "ds18x20.h"
#include "spi.h"
#include "nrf24.h"
#include "mkuart.h"
#include "timer.h"
#include "power_save.h"

#include "komendy_at.h"

#if defined(__AVR_ATmega328P__)
//na potrzeby resetowania m328 poprzez watchdog/uart
void __init3(void) __attribute__ (( section( ".init3" ), naked, used ));
#endif

uint8_t onew_n, subzero, cel, cel_fract_bits, ds_err, licznik;
int16_t t_min = 850;
int16_t t_max = -500;

//uint8_t nRF_TX_buf[TX_DATA_LENGTH];
char bufor[100];	// bufor na potrzeby odebranych danych z UART

void timer_isr(uint8_t czas);
//void nrf_incoming_message(void * nRF_RX_buff, uint8_t len, uint8_t rx_pipe);

int main(void) {

//	DEBUG_LED_DDR |= (1 << DEBUG_LED);	//GREEN SEK LED

	nRF_TX_buf[ID] = NRF_TRANSMITTER_ID;

	ds_err = search_sensors(&onew_n);

	nRF_TX_buf[ONEW_N] = onew_n;
	nRF_TX_buf[ONEW_FLAG] = ds_err;

	timer_init();
	spi_init();
	nrf_init();
	power_init();

//	register_nRF_IRQ_EVENT_Callback(nrf_incoming_message);
	register_Timer2_Event_Callback(timer_isr);

	// inicjalizacja UART
	USART_Init(__UBRR);

	// rejestracja własnej funkcji do analizowania danych odebranych przez UART
	register_uart_str_rx_event_callback(parse_uart_data);

	sei();

	uart_puts("\r\n");
	uart_puts_P(PSTR("nRF 24L01+\r\n"));
	uart_puts("\r\n");

	uart_puts_P(PSTR("Info: polecenie ATI"));
	uart_puts("\r\n");

	/*
	 uart_puts_P(PSTR("Polecenia AT: AT,ATI,+RST,+STA,+CH,+REG,+ADD\r\n"));
	 uart_puts_P(
	 PSTR(
	 "uzycie: AT+CH? - odczyt konf.; AT+CH - parametry wywolania\r\n"));
	 uart_puts_P(PSTR("AT+CH=x - zapis konf., gdzie x - numer kanalu\r\n"));
	 */

	while (1) {

#if USE_IRQ == 0
//		l = 0x0F & nrf_get_retransmission_count();//sprawdźmy ile razy trzeba było powtórzyć pakiet zanim dotarł
//			uart_putint(l, 10);
//			uart_puts("\r\n");

		if (DataReady()) {
			l = nrf_get_payload_length();		//ilość bajtów do odebrania
			GetData(nRF_TX_buf);//pobierz dane z FIFO

			uart_puts((char*)nRF_TX_buf);//..wyświetl odebrane dane
			uart_puts("\r\n");
		}
#endif
		TIMER_EVENT();
#if USE_IRQ == 1
		nRF_IRQ_EVENT();
#endif
		UART_RX_STR_EVENT(bufor);	// zdarzenie odbiorcze UART
	}
}

void timer_isr(uint8_t czas) {

	uint8_t plost;	//, retr,obs;

	//sprawdzenie int1 flagi

//	if (int1_flag) {
//
//
//		int1_flag = 0;
//	}

	if (1 == czas) {

		uint8_t *cl = (uint8_t*) gSensorIDs; // pobieramy wskaźnik do tablicy adresów czujników
		for (uint8_t i = 0; i < MAXSENSORS * OW_ROMCODE_SIZE; i++)
			*cl++ = 0; // kasujemy całą tablicę
		ds_err = search_sensors(&onew_n); // ponownie wykrywamy ile jest czujników i zapełniamy tablicę

		nRF_TX_buf[ONEW_N] = onew_n;
		nRF_TX_buf[ONEW_FLAG] = ds_err;

		if (int1_flag) {

			uart_puts_P(PSTR("DS18B20: "));
			uart_putint(onew_n, 2);
			uart_puts("\r\n");
		}

#if TESTY == 1
		uart_puts_P(PSTR("DS18B20: "));
		uart_putint(onew_n, 2);
		uart_puts("\r\n");
#endif

	}

	if ((3 == czas) && !ds_err) {
		ds_err = DS18X20_start_meas(DS18X20_POWER_EXTERN, NULL);
		nRF_TX_buf[ONEW_FLAG] = ds_err;
	}

	if ((5 == czas) && !ds_err) {

		ds_err = DS18X20_read_meas_single(*gSensorIDs[0], &subzero, &cel,
				&cel_fract_bits);

		nRF_TX_buf[ONEW_FLAG] = ds_err;

		if ( DS18X20_OK == ds_err) {
			if (subzero)
				nRF_TX_buf[TEMP_SUBZERO] = 1;
			else
				nRF_TX_buf[TEMP_SUBZERO] = 0;

			nRF_TX_buf[TEMP] = cel;
			nRF_TX_buf[TEMP_FRAC] = cel_fract_bits;

			DS18X20_temp_minmax(&t_min, &t_max, subzero, cel, cel_fract_bits);

			if (t_min < 0) {
				nRF_TX_buf[T_MIN_SUBZERO] = 1;
				nRF_TX_buf[T_MIN] = t_min / 10 * (-1);
				nRF_TX_buf[T_MIN_FRAC] = (t_min + nRF_TX_buf[T_MIN] * 10)
						* (-1);

			} else {
				nRF_TX_buf[T_MIN_SUBZERO] = 0;
				nRF_TX_buf[T_MIN] = t_min / 10;
				nRF_TX_buf[T_MIN_FRAC] = t_min - nRF_TX_buf[T_MIN] * 10;
			}

			if (t_max < 0) {
				nRF_TX_buf[T_MAX_SUBZERO] = 1;
				nRF_TX_buf[T_MAX] = t_max / 10 * (-1);
				nRF_TX_buf[T_MAX_FRAC] = (t_max + nRF_TX_buf[T_MAX] * 10)
						* (-1);

			} else {
				nRF_TX_buf[T_MAX_SUBZERO] = 0;
				nRF_TX_buf[T_MAX] = t_max / 10;
				nRF_TX_buf[T_MAX_FRAC] = t_max - nRF_TX_buf[T_MAX] * 10;
			}
		}

		if (int1_flag) {

			uart_puts_P(PSTR("T= "));

			if (subzero)
				uart_putc('-');
			else
				uart_putc('+');

			uart_putint(cel, 10);
			uart_puts(".");
			uart_putint(cel_fract_bits, 10);
			uart_puts("\r\n");
		}

#if TESTY == 1
		uart_puts_P(PSTR("T= "));

		if(subzero) uart_putc('-');
		else
		uart_putc('+');

		uart_putint(cel, 10);
		uart_puts(".");
		uart_putint(cel_fract_bits, 10);
		uart_puts("\r\n");
#endif

	}

	if (7 == czas) {

		licznik++;
		nRF_TX_buf[NR] = licznik;

		nrf_power_up();
		nrf_send_int(nRF_TX_buf);

		if (int1_flag) {

			uart_puts_P(PSTR("Wysylam bufor...\r\n"));
			for (uint8_t i = 0; i < TX_DATA_LENGTH; i++) {
				uart_putint(nRF_TX_buf[i], 10);
				uart_putc(' ');
			}
			uart_puts("\r\n");

			for (uint8_t i = 0; i < TX_DATA_LENGTH; i++) {
				uart_putint(nRF_TX_buf[i], 16);
				uart_putc(' ');
			}
			uart_puts("\r\n");
		}

#if USE_IRQ == 0
		while (nrf_is_sending())
		;
#endif

	}

	if (9 == czas) {

//		uint8_t reg;

		nRF_TX_buf[OBS_TX] = nrf_get_retransmission_count();
//		reg = nrf_get_retransmission_count();

		plost = (nRF_TX_buf[OBS_TX] & 0xF0) >> 4;
//		plost = (reg & 0xF0) >> 4;

		if (int1_flag) {

			uart_puts_P(PSTR("Last packet retransmission count: "));
			uart_putint(nRF_TX_buf[OBS_TX] & 0x0F, 10);
			uart_puts("\r\n");

			uart_puts_P(PSTR("Packets lost: "));
			uart_putint(plost, 10);
			uart_puts_P(PSTR(", ["));
			uart_putint(nRF_TX_buf[MAXRT15], 10);
			uart_puts("]\r\n");

			uart_puts_P(PSTR("Nr nadanego pakietu: "));
			uart_putint(licznik, 10);
			uart_puts("\r\n");
		}

#if TESTY == 1
		uart_puts_P(PSTR("Last packet retransmission count: "));
		uart_putint(nRF_TX_buf[OBS_TX] & 0x0F, 10);
		uart_puts("\r\n");

		uart_puts_P(PSTR("Packets lost: "));
		uart_putint(plost, 10);
		uart_puts_P(PSTR(", ["));
		uart_putint(nRF_TX_buf[MAXRT15], 10);
		uart_puts("]\r\n");

		uart_puts_P(PSTR("Nr nadanego pakietu: "));
		uart_putint(licznik, 10);
		uart_puts("\r\n");
#endif

		//kasowanie flagi PLOS_CNT w rej. OBSERVE_TX. DS str. 56.
		if (15 == plost) {

			nRF_TX_buf[MAXRT15]++;
			nrf_set_channel(nrf_ch);
		}

	}

	//todo: obsługa baterii adc, wybudzanie

	if (11 == czas) {

		if (int1_flag)
			uart_puts_P(PSTR("Sleep mode...\r\n"));

#if TESTY == 1
		uart_puts_P(PSTR("Sleep mode...\r\n"));
#endif

		go_sleep();

		if (int1_flag)
			uart_puts("\r\n");

#if TESTY == 1
		uart_puts("\r\n");
#endif
	}

	int1_flag = 0;
}

/*
 void nrf_incoming_message(void * nRF_RX_buff, uint8_t len, uint8_t rx_pipe) {

 nRF_TX_buf[RX_S]++;

 uart_puts_P(PSTR("--- ODEBRANO TRANSMISJE ---\r\n"));
 uart_puts_P(PSTR("#: "));
 uart_putint(nRF_TX_buf[RX_S], 10);
 uart_putc('/');
 uart_putint(nRF_TX_buf[NR], 10);
 uart_puts("\r\n");
 uart_puts((char*) nRF_RX_buff);

 uart_puts_P(PSTR("\r\nZnakow: "));
 uart_putint(len, 10);
 uart_puts_P(PSTR(", @: "));
 uart_putint(rx_pipe, 10);
 uart_puts("\r\n");

 if (strpbrk((char*) nRF_RX_buff, "R")) {
 uart_puts_P(PSTR("restart za 2 sekundy...\r\n"));
 _delay_ms(2000);

 cli();
 // wyłącz przerwania
 wdt_enable(0); // ustaw watch-dog
 while (1)
 ;		// czekaj na RESET

 }
 }
 */

#if defined(__AVR_ATmega328P__)
void __init3(void) {
	/* wyłączenie watchdoga (w tych mikrokontrolerach, w których watchdog
	 * ma możliwość generowania przerwania pozostaje on też aktywny po
	 * resecie) */

	MCUSR = 0;

	WDTCSR = (1 << WDCE) | (1 << WDE);
	WDTCSR = 0;
}
#endif

