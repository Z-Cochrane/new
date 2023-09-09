/*
 * power_save.h
 *
 */

#ifndef POWER_SAVE_H_
#define POWER_SAVE_H_


//ilosc petli 8-sekundowych do wybudzenia przez WDT.
#define WDT_MAX	8

extern volatile uint8_t int1_flag;

void power_init(void);
void go_sleep(void);




#endif /* POWER_SAVE_H_ */
