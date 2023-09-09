/*
 * ----------------------------------------------------------------------------
 * â€śTHE COFFEEWARE LICENSEâ€ť (Revision 1):
 * <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a coffee in return.
 * -----------------------------------------------------------------------------
 * This library is based on this library:
 *   https://github.com/aaronds/arduino-nrf24l01
 * Which is based on this library:
 *   http://www.tinkerer.eu/AVRLib/nRF24L01
 * -----------------------------------------------------------------------------
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>

#include "spi.h"
#include "nrf24.h"

volatile uint8_t RX_flag, TX_flag, MAX_RT_flag;
uint8_t nrf_ch = KANAL;

char nrf_rx_buffer[RX_DATA_LENGTH + 1];

/*------------------------------------------------
 Address Data Pipe 0 (RX_ADDR_P0): 0x7878787878
 Address Data Pipe 1 (RX_ADDR_P1): 0xB3B4B5B6F1
 Address Data Pipe 2 (RX_ADDR_P2): 0xB3B4B5B6CD
 Address Data Pipe 3 (RX_ADDR_P3): 0xB3B4B5B6A3
 Address Data Pipe 4 (RX_ADDR_P4): 0xB3B4B5B60F
 Address Data Pipe 5 (RX_ADDR_P5): 0xB3B4B5B605
 -------------------------------------------------*/

/* ------------------------------------------------------------------------- */
//uint8_t TX_Address[TX_ADDRESS_LENGTH] = { 0xB3, 0xB4, 0xB5, 0xB6, 0xF1 };
//uint8_t RX1_Address[RX_ADDRESS_LENGTH] = { 0x78, 0x78, 0x78, 0x78, 0x78 };
uint8_t RX0_Address[5] = { 0xB3,0xB4,0xB5,0xB6,0xF1 };
uint8_t RX1_Address[5] = {0x78,0x78,0x78,0x78,0x78 };
/* ------------------------------------------------------------------------- */

///////////////////////////////////////////////////////////////////////////////
//                  Register Settings                                        //
///////////////////////////////////////////////////////////////////////////////
const uint16_t nRF24_Config[] PROGMEM
		= {
#if USE_IRQ == 1
				(CONFIG << 8) | (1<<MASK_RX_DR) | (1 << EN_CRC) | (1 << CRC0) | (0 << PWR_UP)
						| (0 << PRIM_RX),	//CRC 2B,

#else
				(CONFIG<<8) | (1<<MASK_RX_DR) | (1<<MASK_TX_DS) | (1<MASK_MAX_RT) | (1<<EN_CRC) | (1<<CRC0) | (0<<PWR_UP) | (0<<PRIM_RX),//CRC 2B,

#endif
				(EN_AA << 8) | 0x03,//0x3F,								//All ACK
				(EN_RX_ADDR << 8) | 0x03,//0x3F,							//All enable
				(SETUP_AW << 8) | 0x03,					//5 bytes adress P0-P5
				(SETUP_RETR << 8) | (0x03 << ARD) | (0x0F << ARC),//repeat 15 x 1ms
				(RF_CH << 8) | KANAL,								//channel 2
				(RF_SETUP << 8) | (1 << RF_DR_LOW) | (0 << RF_DR)
						| (0x03 << RF_PWR),		//250kbps, 0dB out.Pow.
				(STATUS << 8) | 0xFF,							//Clear Status
				(OBSERVE_TX << 8) | 0x00, 							//Read Only
				(CD << 8) | 0x00, 									//Read Only
				(RX_ADDR_P0 << 8) | 0xE7, 									//
				(RX_ADDR_P1 << 8) | 0xC2, 									//
				(RX_ADDR_P2 << 8) | 0xC3, 									//
				(RX_ADDR_P3 << 8) | 0xC4, 									//
				(RX_ADDR_P4 << 8) | 0xC5, 									//
				(RX_ADDR_P5 << 8) | 0xC6, 									//
				(TX_ADDR << 8) | 0xE7, 										//
				(RX_PW_P0 << 8) | 0x20, 									//
				(RX_PW_P1 << 8) | 0x20, 									//
				(RX_PW_P2 << 8) | 0, 									//
				(RX_PW_P3 << 8) | 0, 									//
				(RX_PW_P4 << 8) | 0, 									//
				(RX_PW_P5 << 8) | 0, 									//
				(FIFO_STATUS << 8) | 0x00, 									//
				(DYNPD << 8) | 0x03,//0x3F, 										//
				(FEATURE << 8) | (1 << EN_DPL) | (1 << EN_ACK_PAY)
						| (1 << EN_DYN_ACK)	//
		};

///////////////////////////////////////////////////////////////////////////////
//                  Register Access                                          //
///////////////////////////////////////////////////////////////////////////////

//Returns status
uint8_t nrf_get_status() {
	uint8_t ret;

	CSN_SS_0();
	ret = spi_transmit_byte(NOP);
	CSN_SS_1();

	return ret;
}

/* Set channel number */
void nrf_set_channel(uint8_t ch) {
	nrf_write_register(RF_CH, 0x7F & ch);
}

/* Set the RX address P2-P5 */
void nrf_set_r_address(uint8_t pipe_a, uint8_t adr) {

	if (pipe_a < RX_ADDR_P2)
		return;

	nrf_write_register(pipe_a, adr);
}

/* Set the RX address */
void nrf_set_rx_address(uint8_t pipe_a, uint8_t * adr) {

	uint8_t tab[RX_ADDRESS_LENGTH];

	//adres jest zapysywany do nrf od najmłodszszego bitu (LSB), w tabeli przekazanej do funkcji przez wskaźnik, pierwszy element to bit najstarszy (MSB)
	for (uint8_t i=0;i<RX_ADDRESS_LENGTH;i++) {

		tab[i] = adr[RX_ADDRESS_LENGTH - 1 - i];
	}


	nrf_write(pipe_a, tab, RX_ADDRESS_LENGTH);
}

/* Set the TX address */
void nrf_set_tx_address(uint8_t* adr) {

	uint8_t tab[TX_ADDRESS_LENGTH];

	//adres jest zapysywany do nrf od najmłodszszego bitu (LSB), w tabeli przekazanej do funkcji przez wskaźnik, pierwszy element to bit najstarszy (MSB)
	for (uint8_t i=0;i<TX_ADDRESS_LENGTH;i++) {

		tab[i] = adr[TX_ADDRESS_LENGTH - 1 - i];

	}


	/* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
	nrf_write(RX_ADDR_P0, tab, RX_ADDRESS_LENGTH);
	nrf_write(TX_ADDR, tab, TX_ADDRESS_LENGTH);
}

/* Checks if receive FIFO is empty or not */
uint8_t nrf_get_fifo_status() {
	uint8_t fifoStatus;

	fifoStatus = nrf_read_register(FIFO_STATUS);
	fifoStatus = fifoStatus & FIFO_STATUS_RX_EMPTY;

	return fifoStatus;
}

/* Returns the length of data waiting in the RX FIFO */
uint8_t nrf_get_payload_length() {
	uint8_t ret;

	CSN_SS_0();
	spi_send_byte(R_RX_PL_WID);
	ret = spi_transmit_byte(NOP);
	CSN_SS_1();

	return ret;
}

/* Returns the number of retransmissions occurred for the last message */
uint8_t nrf_get_retransmission_count() {
	uint8_t ret;

	ret = nrf_read_register(OBSERVE_TX);

	return ret;
}

#if USE_IRQ ==0
uint8_t nrf_is_sending() {
	uint8_t status;

	/* read the current status */
	status = nrf_get_status();
	/* if sending successful (TX_DS) or max retries exceed (MAX_RT). */
	if (status & (STATUS_TX_DS | STATUS_MAX_RT)) {
		CE_0();
		return 0; /* false */
	}
	return 1; /* true */
}

/* Checks if data is available for reading */
/* Returns 1 if data is ready ... */
uint8_t DataReady() {
	// See note in getData() function - just checking RX_DR isn't good enough
	uint8_t status;

	status = nrf_get_status();
	// We can short circuit on RX_DR, but if it's not set, we still need
	// to check the FIFO for any pending packets
	if (status & STATUS_RX_DR) {
		return 1;
	}
	status = !nrf_get_fifo_status();

	return status;
}

/* Reads payload bytes into data array */
void GetData(uint8_t* data, uint8_t length) {
//	uint8_t length, i;
	uint8_t i;
	uint8_t *wsk = 0;

	//TODO: zmienna length bez irq/f. getdata

//	length = nrf_read_register(R_RX_PL_WID); //Read Payload Length
//	CSN_SS_0();
//	length = spi_transmit_byte(R_RX_PL_WID);
//	CSN_SS_1();
	//read data
//	nrf_read(RD_RX_PAYLOAD, data, length);

	CSN_SS_0();//chip select low
	spi_send_byte( RD_RX_PAYLOAD);//read payload command
	for (i = 0; i < length; i++)//read data until i < len
	{
		*wsk++ = spi_transmit_byte(NOP);
	}
	CSN_SS_1();

	data = wsk;

	/* Reset status register */
	nrf_write_register(STATUS, STATUS_RX_DR);
//	nrf_flush_rx();//???

	//TODO: length jaka parametr (w main wcześniej i tak odczytywane); flush rx??
	//FIFO status?
}
#endif

//Flush Rx FIFO
void nrf_flush_rx() {
//	nrf_write_register(FLUSH_RX, 0);		//flush Rx
	CSN_SS_0();
	spi_send_byte(FLUSH_RX);
	CSN_SS_1();
}

void nrf_flush_tx() {
	CSN_SS_0();
	spi_send_byte(FLUSH_TX);
	CSN_SS_1();
}

void nrf_set_rx_mode() {
	uint8_t reg;

	//najpierw idĹş do trybu standby I
	CE_0();

	nrf_flush_rx();
	reg = nrf_get_status();				// read register STATUS's value
	reg = reg | STATUS_RX_DR;
	reg = reg & ~(STATUS_TX_DS | STATUS_MAX_RT);
	nrf_write_register(STATUS, reg); // Set Status
	reg = nrf_read_register(CONFIG);	// read register CONFIG's value
	reg = reg | (1 << PRIM_RX) | (1 << PWR_UP);	//set bit 1
	nrf_write_register(CONFIG, reg); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled..
	CE_1();
}

//Function: SwitchToTxMode();
void nrf_set_tx_mode() {
	uint8_t reg;

	//najpierw idĹş do trybu standby I
	CE_0();

	nrf_flush_tx();
	reg = nrf_get_status();	// read register STATUS's value
	reg = reg | STATUS_TX_DS | STATUS_MAX_RT;
	reg = reg & ~STATUS_RX_DR;
	nrf_write_register(STATUS, reg); // Set Status
	reg = nrf_read_register(CONFIG);	// read register CONFIG's value
	reg = reg & ~(1 << PRIM_RX);			//set bit 0
	reg = reg | (1 << PWR_UP);
	nrf_write_register(CONFIG, reg); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled.
}

//Function: SwitchToPowerDown();
void nrf_power_down(void) {
	uint8_t reg;

	//najpierw idĹş do trybu standby I
	CE_0();

	nrf_flush_rx(); //flush Rx
	nrf_flush_tx(); //flush Tx
	reg = nrf_read_register(CONFIG);	// read register CONFIG's value
	reg = reg & ~(1 << PWR_UP);
	nrf_write_register(CONFIG, reg);
}


void nrf_power_up(void) {
	uint8_t reg;

	//najpierw idĹş do trybu standby I
	CE_0();

	nrf_flush_rx(); //flush Rx
	nrf_flush_tx(); //flush Tx
	reg = nrf_read_register(CONFIG);	// read register CONFIG's value
	reg = reg | (1 << PWR_UP);
	nrf_write_register(CONFIG, reg);
}

// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
void nrf_send_int(uint8_t* data) {

	uint8_t *wsk = data;

	nrf_set_tx_mode();
	//Write data to buffer

	CSN_SS_0();
	spi_send_byte(W_TX_PAYLOAD);

	for (uint8_t i = 0; i < TX_DATA_LENGTH; i++) {
		spi_send_byte(*wsk++);
	}
	CSN_SS_1();

	//rozpocznij wysyłanie
	CE_1();
}

void nrf_send_string(char *data) {

	char *wsk = data;
	uint8_t length;

	length = strlen(data);

	if (length > TX_DATA_LENGTH) length = RX_ADDRESS_LENGTH;

	nrf_set_tx_mode();
	//Write data to buffer

	CSN_SS_0();
	spi_send_byte(W_TX_PAYLOAD);

	for (uint8_t i = 0; i < length; i++) {
		spi_send_byte(*wsk++);
	}
	CSN_SS_1();

	//rozpocznij wysyłanie
	CE_1();
}

void nrf_init(void) {
	uint8_t i;
	uint16_t temp;

	NRF_DDR |= (1 << NRF_CE);

#if USE_IRQ == 1

	NRF_IRQ_PORT |= (1 << NRF_IRQ);

#if  ( defined(__AVR_ATmega8__))
	GICR |= (1<<INT0);          //enable INT0 interrupt source in EIMSK register
	MCUCR |= (1<<ISC01);//set interrupt active on falling edge
#elif  ( defined(__AVR_ATmega328P__))
	EIMSK |= (1 << INT0);       //enable INT0 interrupt source in EIMSK register
	EICRA |= (1 << ISC01);       //set interrupt active on falling edge
#endif

#endif

	for (i = 0; i < (sizeof(nRF24_Config) / 2); i++) {
		temp = pgm_read_word(&nRF24_Config[i]);
		nrf_write_register((temp >> 8), temp & 0xFF);
	}

	//Ustaw adresy dla poszczególnych kanałów
	nrf_set_tx_address(RX0_Address);
	nrf_set_rx_address(RX_ADDR_P1, RX1_Address);

/*	nrf_set_r_address(RX_ADDR_P2, 0x32);
	nrf_set_r_address(RX_ADDR_P3, 0x33);
	nrf_set_r_address(RX_ADDR_P4, 0x34);
	nrf_set_r_address(RX_ADDR_P5, 0x35);*/
}

//-------------------------------//
// callback function declaration //
//-------------------------------//
static void (*nRF_IRQ_EVENT_Callback)(void * nRF_RX_buff, uint8_t len,
		uint8_t rx_pipe);

//---------------------------------------------------------------//
// function which is used to register your own callback function //
//---------------------------------------------------------------//
void register_nRF_IRQ_EVENT_Callback(
		void (*callback)(void * nRF_RX_buff, uint8_t len, uint8_t rx_pipe)) {
	nRF_IRQ_EVENT_Callback = callback;
}

//-----------------------------//
// Received data event function //
//-----------------------------//
void nRF_IRQ_EVENT(void) {

/*
	//TODO: nRF_IRQ_EVENT
	if (RX_flag) { // && !nrf_get_fifo_status()) {

//		uint8_t fifo_status;
		char * wsk;
		uint8_t pipe_no = 0, length = 0;

		RX_flag = 0;

//		do {

		wsk = nrf_rx_buffer;

		length = nrf_get_payload_length(); //if DPL

		if (length > RX_DATA_LENGTH) {
//			uart_puts("length > RX_DATA_LENGTH");
//			return;
			length = RX_DATA_LENGTH;
		}
		pipe_no = nrf_get_status();
		pipe_no = (pipe_no & 0x0E) >> 1;

		CSN_SS_0();
		spi_send_byte(R_RX_PAYLOAD);
		for (uint8_t i = 0; i < length; i++)    //read data until i < len
				{
			*wsk++ = spi_transmit_byte(NOP);
		}
		CSN_SS_1();

//			fifo_status = nrf_read_register(FIFO_STATUS); //read FIFO_STATUS register

//		} while ((fifo_status & (1 << RX_EMPTY)) == 0); //if RX_EMPTY bit is LOW then loop must execute.

		nrf_flush_rx();

		if (nRF_IRQ_EVENT_Callback && length && pipe_no < 7)
			(*nRF_IRQ_EVENT_Callback)(nrf_rx_buffer, length, pipe_no);
	}
*/

	if (TX_flag) {

		//po wyjsciu z przerwania po nadaniu układ przestawiony w trybie Standby I (CE->0)
		TX_flag = 0;

//		nrf_set_rx_mode();
		//todo: przełącz w tryb power down.
		nrf_power_down();
	}

	if (MAX_RT_flag) {


		//flaga maxrt ustawiona w przerwaniu, poddajemy się, czekamy 1 minute na nowe dane z czujnika temp,
		//czyścimy bufor nadawczy
		nrf_flush_tx();
		MAX_RT_flag = 0;
//		nrf_set_rx_mode();
		nrf_power_down();

	}
}

//TODO: ISR(INT0_vect)
ISR(INT0_vect) {

	register uint8_t status;

	//getstatus()
	CSN_SS_0();
	status = spi_transmit_byte(NOP);
	CSN_SS_1();

/*
	if ((status & (1 << RX_DR))) {
		status |= (1 << RX_DR);     //reset interrupt bit on status variable
		nrf_write_register(STATUS, status);
		RX_flag = 1;            //RX flag equals to 1
	}
*/

	if (status & (1 << TX_DS)) {
		CE_0();							// po nadaniu przejscie do stryby Standby I
		status |= (1 << TX_DS);         //clear TX_DS bit in status variable
		nrf_write_register(STATUS, status);
		TX_flag = 1;
	}

	if (status & (1 << MAX_RT)) {
		CE_0();							// tryb Standby I
		status |= (1 << MAX_RT) | (1 << TX_DS); //clear MAX_RT and TX_DS bits in status variable

		nrf_write_register(STATUS, status);

		TX_flag = 0;    //clear TX_flag
		MAX_RT_flag = 1;
	}
}

///////////////////////////////////////////////////////////////////////////////
//                  SPI access                                               //
///////////////////////////////////////////////////////////////////////////////
//nrf24.c

void nrf_write_register(uint8_t reg_name, uint8_t value) {
	CSN_SS_0();
	spi_send_byte(W_REGISTER | reg_name);
	spi_send_byte(value);
	CSN_SS_1();
}

uint8_t nrf_read_register(uint8_t reg_name) {
	uint8_t value;

	CSN_SS_0();
	spi_send_byte(reg_name);
	value = spi_transmit_byte(NOP);
//	value = spi_transmit_byte(reg_name);

	CSN_SS_1();

	return value;
}

void nrf_read(uint8_t reg, uint8_t *pBuf, uint8_t length) {

	CSN_SS_0();
	spi_send_byte(reg);
	spi_transmit_buffer(pBuf, pBuf, length);
	CSN_SS_1();
}

void nrf_write(uint8_t reg, uint8_t *pBuf, uint8_t length) {

	CSN_SS_0();
	spi_send_byte(W_REGISTER | reg);
	spi_send_buffer(pBuf, length);
	CSN_SS_1();
}
