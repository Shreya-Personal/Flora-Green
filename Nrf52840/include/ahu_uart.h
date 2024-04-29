/*
 * File:   ahu_uart.h
 * Author: Shreya Singh
 *
 * Created on March 2, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the ahu_uart.c.
 * 
 */

#ifndef AHU_UART_H
#define AHU_UART_H
#include "../mylib/ahu_uart.c"
//UART NODE DEFINITION
const struct device *uart1 = DEVICE_DT_GET(DT_NODELABEL(uart1));

struct uart_config uart_cfg = {
	.baudrate = 115200,
	.parity = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	.data_bits = UART_CFG_DATA_BITS_8,
};

void send_str(const struct device *uart, char *str);
void recv_str(const struct device *uart, char *str);
extern char file[20]; 


#endif /* AHU_UART_H*/