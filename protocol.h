#ifndef PROTOCOL_HPP_DEFINED
#define PROTOCOL_HPP_DEFINED

/**
 * Coordinates communication and debug protocols
 * 
 * This file provides methods for initializing the UART and sending data through
 * the serial channel. Especially, two functions are provided : send_coord for 
 * sending the coordinates of the tracked device and send_debug* for sending a debug
 * message as a string.
 * 
 * The protocol reserves the byte values 0x16, 0x02 and 0x03. 
 * 
 * Coordinates protocol:
 *  -> START  Coordinate 1    Coordinate 2
 *  -> [0x16] [byte1] [byte2] [byte3] [byte4]
 *
 * coord 1 = (byte1 << 8) | (byte2)
 * coord 2 = (byte3 << 8) | (byte4) 
 *
 * Debug protocol: 
 *  -> START  Message (N bytes)...        END
 *  -> [0x02] [byte1] [byte2] ... [byteN] [0x03] 
 */

/** 
 * Send given coordinates through UART
 * - x_cm : x coordinate in centimeters
 * - y_cm : y coordinate in centimeters
 */
void send_coord(unsigned int x_cm, unsigned int y_cm);

/**
 * Send a debug message through the UART.
 * - buffer : the buffer containing the message
 * - buffer_len : the number of bytes to send starting from the first byte of the buffer
 */
void send_debug_nchar(const char* buffer, unsigned int buffer_len);

/**
 * Send a debug message through the UART.
 * Message is expected to be delimited by the null character '\0'
 * - buffer : the buffer containing the message
 */
void send_debug(const char* buffer);

/** 
 * Initialize the UART so that it 
 */
void initUART();

#endif // PROTOCOL_HPP_DEFINED