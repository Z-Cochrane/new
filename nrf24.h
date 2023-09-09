#ifndef NRF24
#define NRF24

#include <stdint.h>

#include "spi.h"
#include "nrf24l01.h"

#define USE_IRQ	1
#define NRF_TRANSMITTER_ID	0xF1

#define NRF_DDR		DDRC
#define NRF_PORT	PORTC
#define	NRF_CE		PC0

#define CE_0()	(NRF_PORT &= ~(1<<NRF_CE))
#define CE_1()	(NRF_PORT |= (1<<NRF_CE))

#if USE_IRQ == 1
#define NRF_IRQ 	PD2			//IRQ PIN	D2, int0

#define NRF_IRQ_DDR     DDRD
#define NRF_IRQ_PORT    PORTD
#define NRF_IRQ_PIN     PIND
#endif

//--------------------------------//
// SET TX and RX addresses length //
//--------------------------------//

#define TX_ADDRESS_LENGTH 5
#define RX_ADDRESS_LENGTH 5

#define RX_DATA_LENGTH	2
#define TX_DATA_LENGTH	21

#define MAX_PAYLOAD_LENGTH	32

#define KANAL	2

#define ID				0
#define ONEW_N			1
#define ONEW_FLAG		2
#define TEMP_SUBZERO	3
#define TEMP			4
#define TEMP_FRAC		5
#define T_MIN_SUBZERO	6
#define T_MIN			7
#define T_MIN_FRAC		8
#define T_MAX_SUBZERO	9
#define T_MAX			10
#define T_MAX_FRAC		11
#define OBS_TX			12
#define NR				13//nr kolejny nadanego pakietu
#define RX_S			14//nr kolejny odebranego pakietu
#define MAXRT15			15

//lion data
#define LION_INFO_FL	16	//0:LION; 1:LBAT
#define VBAT_U			17
#define VBAT_F			18
#define LION_STATUS_FL	19	//0:OFF; 1:ON

uint8_t nRF_TX_buf[TX_DATA_LENGTH];


extern volatile uint8_t /*RX_flag, */TX_flag, MAX_RT_flag;
extern uint8_t nrf_ch;

uint8_t nrf_get_status(void);
void nrf_set_channel(uint8_t ch);
void nrf_set_r_address(uint8_t pipe_a, uint8_t adr);
void nrf_set_rx_address(uint8_t pipe_a, uint8_t * adr);
void nrf_set_tx_address(uint8_t* adr);
uint8_t nrf_get_fifo_status(void);
uint8_t nrf_get_payload_length(void);
uint8_t nrf_get_retransmission_count(void);

#if USE_IRQ == 0
uint8_t nrf_is_sending(void);
uint8_t DataReady(void);
void GetData(uint8_t* data, uint8_t length);
#endif

void nrf_flush_rx(void);
void nrf_flush_tx(void);

void nrf_set_rx_mode(void);
void nrf_set_tx_mode(void);
void nrf_power_down(void);
void nrf_power_up(void);


void nrf_send_int(uint8_t* data);
void nrf_send_string(char *data);

void nrf_init(void);

#if USE_IRQ == 1
void nRF_IRQ_EVENT(void);

/*void register_nRF_IRQ_EVENT_Callback(
		void (*callback)(void * nRF_RX_buff, uint8_t len, uint8_t rx_pipe));*/
#endif

void nrf_write_register(uint8_t reg_name, uint8_t value);
uint8_t nrf_read_register(uint8_t reg_name);
void nrf_read(uint8_t reg, uint8_t *pBuf, uint8_t length);
void nrf_write(uint8_t reg, uint8_t *pBuf, uint8_t length);

#endif
