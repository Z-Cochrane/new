/*
 * timer.h
 *
 *  Created on: 3 paź 2018
 *      Author: Kasia_
 */

#ifndef TIMER_H_
#define TIMER_H_


/*
#define DEBUG_LED_DDR	DDRB
#define DEBUG_LED_PORT	PORTB
#define DEBUG_LED	PB1

#define LED_BLINK()	(DEBUG_LED_PORT ^= (1 << DEBUG_LED))
*/

void timer_init(void);


void TIMER_EVENT(void); //this function checks if there are data to read and if there is callback function registered.
//if there are data available to read this function sends this data to registered callback function

void register_Timer2_Event_Callback(void (*callback)(uint8_t czas));


#endif /* TIMER_H_ */
