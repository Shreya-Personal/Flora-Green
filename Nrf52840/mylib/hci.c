/*
 * File:   hci.c
 * Author: Shreya Singh
 *
 * Created on March 2, 2024, 8:00AM
 * Description: This file contains the implementation of functions used in
 *              the AHU driver.
 */
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <version.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/sys/printk.h>

#include "../include/ahu_uart.h"
#include "../include/ahu_shell.h"
struct uart_item_type {
    int preamble;
    int type;
    int device_id;
	char cmd; 
	char **params; 
    int sensor_value; 
};

char uart_buffer[10 * sizeof(struct uart_item_type)];
struct k_msgq my_uartmsgq;
struct uart_item_type uart_poll;


/*
 * Function: hci_data_packet_poll
 * ------------------------------
 * Sends HCI data packets over UART based on the data in the uart_poll struct.
 * Handles different types of commands and constructs the data packets accordingly.
 * 
 * No parameters or return values.
 */
void hci_data_packet_poll(){
    while(1){ 
        // Variable definitions
        int preamble = uart_poll.preamble; 
        int type = uart_poll.type; 
        int device_id = uart_poll.device_id; 
        char cmd = uart_poll.cmd;

        uart_poll_out(uart1, '\n');
        k_sleep(K_MSEC(100));
        int param_args = 0;
        while (uart_poll.params[param_args] != NULL) {
            param_args++;
        }
        //Find data length
        int data_length; 
        if (cmd == 'l'){ 
            data_length = param_args;
        }else if(cmd == 'r'){
            data_length = strlen(file) + 2;  
        }else { 
            data_length = 1 + (param_args-1)*2;
        }

        // First 16 bits 
        uart_poll_out(uart1, preamble);
        k_sleep(K_MSEC(100));
        uart_poll_out(uart1, (type << 4) | device_id);
        // printk("One : %d , %d \n", preamble, (type << 4) | device_id );
        k_sleep(K_MSEC(100));

        // Second 16 bits 
        // printk("Two : %d , %d \n", data_length, cmd);
        uart_poll_out(uart1, data_length); 
        k_sleep(K_MSEC(100));
        uart_poll_out(uart1, cmd);
        k_sleep(K_MSEC(100));

        // Data Poll out
        if (cmd == 'g'||cmd == 'm'||cmd == 'n'||cmd == 'r' ){ // IF GCU GRAPH, skip params 
            uart_poll_out(uart1, uart_poll.sensor_value);
            k_sleep(K_MSEC(100));
            for (int i = 0; i< strlen(file); i++){ 
                uart_poll_out(uart1, file[i]);
                k_sleep(K_MSEC(100));
            }
            //Add null terminator
            uart_poll_out(uart1, '\n');
            k_sleep(K_MSEC(100));
        }
        else if (cmd != 'l'){ 
            for (int i = 1; i < param_args ; i++){ 
                int num = atoi(uart_poll.params[i]);
                int remainder = (int)(atof(uart_poll.params[i])*100 - num*100);
                uart_poll_out(uart1, num);
                k_sleep(K_MSEC(100));
                uart_poll_out(uart1, remainder);
                k_sleep(K_MSEC(100));      
            }
            // Add null terminator
            uart_poll_out(uart1, '\n');
            k_sleep(K_MSEC(100));
        } else if (cmd == 'd'){ 
            for (int i = 1; i < param_args ; i++){ 
                int num = atoi(uart_poll.params[i]);
                int higherEightBits = (num >> 8) & 0xFF;
                int lowerEightBits = num & 0xFF;
                uart_poll_out(uart1, higherEightBits);
                k_sleep(K_MSEC(100));
                uart_poll_out(uart1, lowerEightBits);
                k_sleep(K_MSEC(100));      
            }
            // Add null terminator
            uart_poll_out(uart1, '\n');
            k_sleep(K_MSEC(100));

        }else{
            for (int i = 1; i < param_args; i++){ 
                char p = *uart_poll.params[i];
                uart_poll_out(uart1,p);
                if (i % 2 == 0){ 
                    k_sleep(K_MSEC(100));
                }
            }
            //Add null terminator
            uart_poll_out(uart1, '\n');
            k_sleep(K_MSEC(100));
        }
        k_sleep(K_SECONDS(1));
    }
}

/**
 * @brief Continuously retrieves data items from the message queue and processes them.
 * 
 * This function runs in an infinite loop, getting data items from the message queue
 * `my_uartmsgq` using the `k_msgq_get` function with a wait time of `K_FOREVER`.
 * 
 * @note Make sure to initialize and configure the message queue `my_uartmsgq` before
 *       using this function.
 */
void uart_update(void)
{
    while (1) {
        /* get a data item */
        k_msgq_get(&my_uartmsgq, &uart_poll, K_FOREVER);
        //k_msgq_get(&my_uartmsgq, &uart_poll, K_FOREVER);
    }
}