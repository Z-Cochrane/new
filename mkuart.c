/*
 * mkuart.c
 *
 *  Created on: 2010-09-04
 *       Autor: Mirosęaw Kardaę
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/pgmspace.h>

#include "mkuart.h"

volatile uint8_t ascii_line;

// definiujemy w końcu nasz bufor UART_RxBuf
volatile char UART_RxBuf[UART_RX_BUF_SIZE];
// definiujemy indeksy określające ilość danych w buforze
volatile uint8_t UART_RxHead; // indeks oznaczający głowę węża
volatile uint8_t UART_RxTail; // indeks oznaczający ogon węża

// definiujemy w końcu nasz bufor UART_RxBuf
volatile char UART_TxBuf[UART_TX_BUF_SIZE];
// definiujemy indeksy określające ilość danych w buforze
volatile uint8_t UART_TxHead; // indeks oznaczający głowę węża
volatile uint8_t UART_TxTail; // indeks oznaczający ogon węża

// Wskaźnik do funkcji callback dla zdarzenia UART_RX_STR_EVENT()
static void (*uart_rx_str_event_callback)(char * pBuf);

// funkcja do rejestracji funkcji zwrotnej w zdarzeniu UART_RX_STR_EVENT()
void register_uart_str_rx_event_callback(void (*callback)(char * pBuf)) {
	uart_rx_str_event_callback = callback;
}

// Zdarzenie do odbioru danych łańcucha tekstowego z bufora cyklicznego
void UART_RX_STR_EVENT(char * rbuf) {

	if (ascii_line) {
		if (uart_rx_str_event_callback) {
			uart_get_str(rbuf);
			(*uart_rx_str_event_callback)(rbuf);
		} else {
			UART_RxHead = UART_RxTail;
		}
	}
}

void USART_Init(uint16_t baud) {
	/* Ustawienie prędkości */
	UBRR0H = (uint8_t) (baud >> 8);
	UBRR0L = (uint8_t) baud;
	/* Załączenie nadajnika I odbiornika */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	/* Ustawienie format ramki: 8bitów danych, 1 bit stopu */
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
}

// definiujemy funkcję dodającą jeden bajt do z bufora cyklicznego
void uart_putc(char data) {
	uint8_t tmp_head;
	ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
	{
		tmp_head = (UART_TxHead + 1) & UART_TX_BUF_MASK;
	}
	// pętla oczekuje jeżeli brak miejsca w buforze cyklicznym na kolejne znaki
	while (tmp_head == UART_TxTail) {
	}

	UART_TxBuf[tmp_head] = data;
	UART_TxHead = tmp_head;

	// inicjalizujemy przerwanie występujące, gdy bufor jest pusty, dzięki
	// czemu w dalszej części wysyłaniem danych zajmie się już procedura
	// Obsługi przerwania
	UCSR0B |= (1 << UDRIE0);
}

void uart_puts(char *s)		// wysyęa ęaęcuch z pamięci RAM na UART
{
	register char c;
	while ((c = *s++))
		uart_putc(c);			// dopoki nie napotkasz 0 wysyłaj znak
}

void uart_puts_P(const char *s)
{
	register char c;
	while ((c = (pgm_read_byte(s++))))
		uart_putc(c);			// dopoki nie napotkasz 0 wysyłaj znak
}



void uart_putint(int value, int radix)	// wysyęa na port szeregowy tekst
{
	char string[17];			// bufor na wynik funkcji itoa
	itoa(value, string, radix);		// konwersja value na ASCII
	uart_puts(string);			// wyęlij string na port szeregowy
}

// definiujemy procedurę obsęugi przerwania nadawczego, pobierajęcę dane z bufora cyklicznego
ISR( USART_UDRE_vect) {
	// sprawdzamy czy indeksy sę ręne
	if (UART_TxHead != UART_TxTail) {
		// obliczamy i zapamiętujemy nowy indeks ogona węa (moęe się zręwnaę z gęowę)
		UART_TxTail = (UART_TxTail + 1) & UART_TX_BUF_MASK;
		// zwracamy bajt pobrany z bufora  jako rezultat funkcji
		UDR0 = UART_TxBuf[UART_TxTail];
	} else {
		// zerujemy flagę przerwania występujęcego gdy bufor pusty
		UCSR0B &= ~(1 << UDRIE0);
	}
}

// definiujemy funkcję pobierajęcę jeden bajt z bufora cyklicznego
int uart_getc(void) {
	int data = -1;
	// sprawdzamy czy indeksy sę ręwne
	if (UART_RxHead == UART_RxTail)
		return data;
	ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
	{
		// obliczamy i zapamiętujemy nowy indeks ęogona węża (moęe się zręwnaę z gęowę)
		UART_RxTail = (UART_RxTail + 1) & UART_RX_BUF_MASK;
		// zwracamy bajt pobrany z bufora  jako rezultat funkcji
		data = UART_RxBuf[UART_RxTail];
	}
	return data;
}

char * uart_get_str(char * buf) {
	int c;
	char * wsk = buf;
	if (ascii_line) {
		while ((c = uart_getc())) {
			if (13 == c || c < 0)
				break;
			*buf++ = c;
		}
		*buf = 0;
		ascii_line--;
	}
	return wsk;
}

// definiujemy procedurę obsęugi przerwania odbiorczego, zapisujęcę dane do bufora cyklicznego
ISR( USART_RX_vect) {

	register uint8_t tmp_head;
	register char data;

	data = UDR0; //pobieramy natychmiast bajt danych z bufora sprzętowego

	// obliczamy nowy indeks ęgęowy węża
	tmp_head = (UART_RxHead + 1) & UART_RX_BUF_MASK;

	// sprawdzamy, czy węę nie zacznie zjadaę węasnego ogona
	if (tmp_head == UART_RxTail) {
		// tutaj moęemy w jakię wygodny dla nas sposęb obsęuęyę  bęęd spowodowany
		// prębę nadpisania danych w buforze, mogęoby dojęę do sytuacji gdzie
		// nasz węę zaczęęby zjadaę węasny ogon
		// jednym z najbardziej oczywistych rozwięzaę jest np natychmiastowe
		// wyzerowanie zmiennej ascii_line lub sterowanie sprzętowę linię
		// zajętoęci bufora
		UART_RxHead = UART_RxTail;
	} else {
		switch (data) {
		case 0:					// ignorujemy bajt = 0
		case 10:
			break;			// ignorujemy znak LF
		case 13:
			ascii_line++;	// sygnalizujemy obecnoęę kolejnej linii w buforze
		default:
			UART_RxHead = tmp_head;
			UART_RxBuf[tmp_head] = data;
		}

	}
}

