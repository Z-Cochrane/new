#include <avr/io.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/sleep.h>

#include "mkuart.h"
#include "timer.h"
#include "power_save.h"

volatile uint8_t int1_flag;

void power_init(void) {


	 //debug, wybudzenie ze snu przez int1
	 PORTD |= (1 << PD3); //int1

	 EIMSK |= (1 << INT1);       //enable INT1 interrupt source in EIMSK register
	 EICRA |= (1 << ISC11);       //set interrupt active on falling edge


	MCUCR |= (1 << BODSE) | (1 << BODS);
	MCUCR = (1 << BODS);

	PRR |= (1 << PRADC) | (1 << PRTIM0) | (1 << PRTIM1) | (1 << PRTWI);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

inline void go_sleep(void) {

	cli();

//	PORTB &= ~(1<<PB1);

	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0);

	sei();
	sleep_mode()
	;
}

 ISR (INT1_vect) {
 //PORTB |= (1<<PB1);
	 int1_flag = 1;
 }


ISR (WDT_vect) {

	static uint8_t wdt_licznik;

	cli();

//	PORTB ^= (1<<PB1);

	/* Clear WDRF in MCUSR */
	MCUSR &= ~(1 << WDRF);

	if (wdt_licznik < WDT_MAX) {

		wdt_licznik++;

		WDTCSR = (1 << WDCE) | (1 << WDE);
		WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (1 << WDP0);

		sei();

		sleep_mode()
		;
	} else {

		wdt_licznik = 0;

		WDTCSR = (1 << WDCE) | (1 << WDE);
		WDTCSR = 0;

		sei();
	}
}
