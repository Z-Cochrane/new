/*
 * timer.c
 *
 *  Created on: 3 paź 2018
 *      Author: Kasia_
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "timer.h"

volatile uint8_t sek_flag, sekundy;

void timer_init(void) {
	/* ustawienie TIMER2 dla F_CPU = 16 MHz */
	//------------------------------------------------------------------------------
	TCCR2A |= (1 << WGM21); /* tryb CTC */
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); /* prescaler = 1024 */

	OCR2A = 155; /* dodatkowy podział przez 155 (rej. przepełnienia) */
	TIMSK2 |= (1 << OCIE2A); /* zezwolenie na przerwanie CompareMatch */
	/* przerwanie wykonywane z częstotliwością ok 10ms */
	//------------------------------------------------------------------------------
}


//-------------------------------//
// callback function declaration //
//-------------------------------//
static void (*Timer2_Event_Callback)(uint8_t czas);

//---------------------------------------------------------------//
// function which is used to register your own callback function //
//---------------------------------------------------------------//
void register_Timer2_Event_Callback(void (*callback)(uint8_t czas)) {
	Timer2_Event_Callback = callback; //in this line we give an address to our callback function
}

//-----------------------------//
// Received data event function //
//-----------------------------//
void TIMER_EVENT(void) {

	if (sek_flag) {
		if (Timer2_Event_Callback)
			(*Timer2_Event_Callback)(sekundy);

		sek_flag = 0;


	}
}

ISR( TIMER2_COMPA_vect) {
	static uint8_t licznik = 0;

	if (++licznik > 100) {
//		LED_BLINK();

		sek_flag = 1;
		sekundy++;
		if (sekundy > 12) {
			sekundy = 0;
		}
		licznik = 0;
	}
}
