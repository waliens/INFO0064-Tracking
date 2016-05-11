#include <plib/usart.h>
#include <stdbool.h>
#include "protocol.h"

#define START_COORD 0x16
#define START_DEBUG 0x02
#define END_DEBUG 0x03

/**
 * Check whether the given char is reserved by the communication protocol
 */
static inline bool is_reserved(char c);

static inline void putc_when_ready(char c);

static inline bool is_reserved(char c) {
	return c == START_DEBUG || c == START_COORD || c == END_DEBUG;
} 

static inline void putc_when_ready(char c) {
    while(Busy1USART());
    putc1USART(c);
}

void send_coord(unsigned int x_cm, unsigned int y_cm) {
	putc_when_ready(START_COORD);
	putc_when_ready((x_cm >> 8) & 0xFF);
	putc_when_ready((x_cm) & 0xFF);
	putc_when_ready((y_cm >> 8) & 0xFF);
	putc_when_ready((y_cm) & 0XFF); 
}

void send_debug_nchar(const char* buffer, unsigned int buffer_len) {
	putc_when_ready(START_DEBUG);
	for (unsigned int i = 0; i < buffer_len; ++i) {
		if (is_reserved(buffer[i])) { // prevent protocol to enter in invalid mode
			break;  
		}
		putc_when_ready(buffer[i]);
	}
	putc_when_ready(END_DEBUG);
}

void send_debug(const char* buffer) {
	putc_when_ready(START_DEBUG);
    puts1USART(buffer);
	putc_when_ready(END_DEBUG);
}

void initUART() {
	BRGH = 0; // low speed baud rate
    SPBRG = 25; //Writing SPBRG Register (baud rate: 9600)
    SYNC = 0; //Setting Asynchronous Mode, ie UART
    SPEN = 1; //Enables Serial Port
    TRISC7 = 1; //As Prescribed in Datasheet
    TRISC6 = 1; //As Prescribed in Datasheet
    LATC6 = 0;
    LATC7 = 0;
    CREN = 1; //Enables Continuous Reception
    TXEN = 1; //Enables Transmission
}