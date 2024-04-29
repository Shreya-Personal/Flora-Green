/*
 * File:   ahu_shell.c
 * Author: Shreya Singh
 *
 * Created on March 2, 2024, 8:00AM
 * Description: This file contains the implementation of functions used in
 *              the AHU driver.
 * 
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <version.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include "../include/ahu_led.h"
#include "../include/ahu_timer.h"
#include "../include/ahu_uart.h"
#include "../include/hci.h"
#include "../include/ble_devices.h"

#ifdef CONFIG_ARCH_POSIX
#include <unistd.h>
#else
#include <zephyr/posix/unistd.h>
#endif

/**
 * @brief Structure representing a data item for Bluetooth or device communication.
 *
 * This structure contains fields to store information such as command, filter, address, GCU command,
 * and device ID for communication purposes.
 */
struct data_item_type {
    uint8_t command;
    uint8_t filter;
    char addr[30];
	char gcu_cmd; 
	uint8_t device_id; 
};

char my_msgq_buffer[10 * sizeof(struct data_item_type)];
struct k_msgq my_msgq;

struct data_item_type data; 

/**
 * @brief Remove commas from strings in a char** array.
 *
 * This function iterates through each string in the char** array and removes commas from them.
 *
 * @param params Pointer to a char** array containing strings.
 */
void removecommas(char** params) {
    for (int i = 0; params[i] != NULL; i++) {
        char* src = params[i];
        char* dest = params[i];
        while (*src) {
            if (*src != ',') {
                *dest++ = *src;
            }
            src++;
        }
        *dest = '\0';  // Null-terminate the modified string
    }
    
}

/**
 * @brief Calculate the total length of strings in a char** array.
 *
 * This function calculates the total length of all strings in a char** array.
 *
 * @param arr Pointer to a char** array containing strings.
 * @return The total length of all strings in the array.
 */
int getstrlength(char** arr) {
    int length = 0;
    while (*arr != NULL) {
        length += strlen(*arr);
        arr++;
    }
    return length;
}

/**
 * @brief Handle time-related commands.
 *
 * This module includes functions to handle time-related commands in the shell.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_time_null(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	int system_time = get_system_time_null();
	shell_print(sh, "%d Seconds",system_time);

	return 0;
}
static int cmd_time_f(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	int* result = get_system_time_f();
	shell_print(sh, "%d Hours, %d Minutes, %d Seconds",result[0], result[1], result[2]);
	return 0;
}

/**
 * @brief Handle LED-related commands.
 *
 * This module includes functions to handle LED-related commands in the shell.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_led_s(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	
	//LED is set on 
	if (argv[1][0] == '1'){ 
		set_ahu_led(0, "on"); 
	} if (argv[1][1] == '1'){
		set_ahu_led(1, "on");
	} if (argv[1][2] == '1'){
		set_ahu_led(2, "on");
	}  if (argv[1][3] == '1'){
		set_ahu_led(3, "on");
	} 

	//LED is set off 
	if (argv[1][0] == '0'){ 
		set_ahu_led(0, "off");
	} if (argv[1][1] == '0'){
		set_ahu_led(1, "off");
	} if (argv[1][2] == '0'){
		set_ahu_led(2, "off");
	}  if (argv[1][3] == '0'){
		set_ahu_led(3, "off");
	} 
	return 0;
}

/* Toggles LED*/
static int cmd_led_tog(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	if (argv[1][0] == '1'){ 
		toggle_ahu_led(0);
	} if (argv[1][1] == '1'){
		toggle_ahu_led(1);
	} if (argv[1][2] == '1'){
		toggle_ahu_led(2);
	} if (argv[1][3] == '1'){
		toggle_ahu_led(3);
	} 
	return 0;
}


/**
 * @brief Handle commands to draw a point at [x, y].
 *
 * This function processes commands to draw a point at the specified coordinates [x, y].
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_point_xy(const struct shell *sh, size_t argc, char **argv){ 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	struct uart_item_type uart_poll;
	uart_poll.preamble = PREAMBLE; 
	uart_poll.type = RESPONSE; 
	uart_poll.device_id = GCU_DAC; 
	uart_poll.cmd = cmd_point; 
	uart_poll.params = (char **)malloc((argc + 1)* sizeof(char *)); 
	for (int i = 0; i< argc; i++){
		uart_poll.params[i] = argv[i];  
	}
	uart_poll.params[argc] = NULL; 

	while (k_msgq_put(&my_uartmsgq, &uart_poll, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_uartmsgq);
	}
	return 0;
}

/**
 * @brief Handle commands to draw a circle with radius and center coordinates.
 *
 * This function processes commands to draw a circle with the specified radius and center coordinates.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_circle_rxy(const struct shell *sh, size_t argc, char **argv){ 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	struct uart_item_type uart_poll;
	uart_poll.preamble = PREAMBLE; 
	uart_poll.type = RESPONSE; 
	uart_poll.device_id = GCU_DAC; 
	uart_poll.cmd = cmd_circle; 
	uart_poll.params = (char **)malloc((argc + 1)* sizeof(char *)); 
	for (int i = 0; i< argc; i++){
		uart_poll.params[i] = argv[i];  
	}
	uart_poll.params[argc] = NULL; 

	while (k_msgq_put(&my_uartmsgq, &uart_poll, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_uartmsgq);
	}


	return 0;
}

/**
 * @brief Set parameters for the Lissajous curve.
 *
 * This function processes commands to set the parameters a and b for the Lissajous curve.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_lj_abt(const struct shell *sh, size_t argc, char **argv){ 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	struct uart_item_type uart_poll;
	uart_poll.preamble = PREAMBLE; 
	uart_poll.type = RESPONSE; 
	uart_poll.device_id = GCU_DAC; 
	uart_poll.cmd = cmd_lissajous; 
	uart_poll.params = (char **)malloc((argc + 1)* sizeof(char *)); 
	for (int i = 0; i< argc; i++){
		uart_poll.params[i] = argv[i];  
	}
	uart_poll.params[argc] = NULL; 

	while (k_msgq_put(&my_uartmsgq, &uart_poll, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_uartmsgq);
	}
	// /hci_data_packet_poll(PREAMBLE, RESPONSE, GCU_LJ, cmd_lissajous, argv);

	return 0;
}

/**
 * @brief Set DAC (Digital-to-Analog Converter) coordinates.
 *
 * This function processes commands to set DAC coordinates for analog output.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_dac_xy(const struct shell *sh, size_t argc, char **argv){ 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	struct uart_item_type uart_poll;
	uart_poll.preamble = PREAMBLE; 
	uart_poll.type = RESPONSE; 
	uart_poll.device_id = GCU_DAC; 
	uart_poll.cmd = cmd_dac; 
	uart_poll.params = (char **)malloc((argc + 1)* sizeof(char *)); 
	for (int i = 0; i< argc; i++){
		uart_poll.params[i] = argv[i];  
	}
	uart_poll.params[argc] = NULL; 

	while (k_msgq_put(&my_uartmsgq, &uart_poll, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_uartmsgq);
	}
	// hci_data_packet_poll(PREAMBLE, RESPONSE, GCU_DAC, cmd_dac, argv);
	return 0;
}

/**
 * @brief Set RGB values for GCU LEDs.
 *
 * This function processes commands to set RGB values for a general LED.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */

static int cmd_gled_rgb(const struct shell *sh, size_t argc, char **argv){ 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	struct uart_item_type uart_poll;
	uart_poll.preamble = PREAMBLE; 
	uart_poll.type = RESPONSE; 
	uart_poll.device_id = GCU_DAC; 
	uart_poll.cmd = cmd_led; 
	uart_poll.params = (char **)malloc((argc + 1)* sizeof(char *)); 
	for (int i = 0; i< argc; i++){
		uart_poll.params[i] = argv[i];  
	}
	uart_poll.params[argc] = NULL; 

	while (k_msgq_put(&my_uartmsgq, &uart_poll, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_uartmsgq);
	}
	// hci_data_packet_poll(PREAMBLE, RESPONSE, GCU_LED, cmd_led, argv);

	return 0;
}

/**
 * @brief Get status of a general purpose device (GPD) or button.
 *
 * This function processes commands to get the status of a general purpose device (GPD) or button.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_gpd(const struct shell *sh, size_t argc, char **argv){ 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	struct uart_item_type uart_poll;
	uart_poll.preamble = PREAMBLE; 
	uart_poll.type = RESPONSE; 
	uart_poll.device_id = GCU_DAC; 
	uart_poll.cmd = cmd_button; 
	uart_poll.params = (char **)malloc((argc + 1)* sizeof(char *)); 
	for (int i = 0; i< argc; i++){
		uart_poll.params[i] = argv[i];  
	}
	uart_poll.params[argc] = NULL; 

	while (k_msgq_put(&my_uartmsgq, &uart_poll, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_uartmsgq);
	}
	// hci_data_packet_poll(PREAMBLE, REQUEST, GCU_BUTTON, cmd_button, argv);

	char recv_buf[64];
	recv_str(uart1, recv_buf);
	if (strcmp("1",recv_buf) == 0) { 
		shell_print(sh, "GCU Pushbutton On");
	} else if (strcmp("0",recv_buf) == 0) {
		shell_print(sh, "GCU Pushbutton Off"); 
	}else{ 
		shell_print(sh, "No Message Recieved");
	}
	return 0;
}

/**
 * @brief Start a Bluetooth LE scan with specified options.
 *
 * This function processes commands to start a Bluetooth Low Energy (LE) scan with specified options.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_blescan_s(const struct shell *sh, size_t argc, char **argv){ 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	shell_print(sh, "Start Scan %s %d", argv[1], strlen(argv));
	data.command = 1;
	if (strlen(argv)== 2 && strcmp(argv[1], "-f") == 0){ 
		data.filter = 'f';
		strcpy(data.addr, argv[2]);
		data.addr[18] = '\0';
		 
	}else if(strlen(argv) != 2){ 
		shell_print(sh, "Invalid Command -> blescan -<s/p> -f <ADDR> ");
	}else { 
		data.filter = '\0';
	}

	// /* send data to consumers */
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	return 0;
} 

/**
 * @brief Stop a Bluetooth LE scan with specified options.
 *
 * This function processes commands to start a Bluetooth Low Energy (LE) scan with specified options.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_blescan_p(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "Stop Scan ");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	data.command = 2;
	strcpy(data.addr, argv[1]);
	data.addr[18] = '\0';
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	// Check if -f command and filter address 
	return 0;
	
} 

/**
 * @brief Start receiving sensor data from a Bluetooth LE device.
 *
 * This function processes commands to start receiving sensor data from a specified Bluetooth Low Energy (LE) device.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_blecon_s(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "Start receiving sensor data ");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	struct data_item_type data;
	data.command = 3;
	strcpy(data.addr, argv[1]);
	data.addr[18] = '\0';
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	return 0;
	
} 

/**
 * @brief Stop receiving sensor data from a Bluetooth LE device.
 *
 * This function processes commands to start receiving sensor data from a specified Bluetooth Low Energy (LE) device.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_blecon_p(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "Stop receiving sensor data ");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	data.command = 4;
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	return 0;
	
} 

/**
 * @brief Display selected sensor values as a graph plot using the GCU.
 *
 * This function processes commands to display selected sensor values as a graph plot using the GCU (Graphical Control Unit).
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_gcugraph(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "displays the selected sensor values as a graph plot");
	// Check if -f command and filter address 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);	
	data.device_id = atoi(argv[1]);
	data.gcu_cmd = gcugraph_cmd;
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	return 0;
	
} 


/**
 * @brief Display selected sensor values as a meter widget using the GCU.
 *
 * This function processes commands to display selected sensor values as a meter widget using the GCU (Graphical Control Unit).
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_gcumeter(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "displays the selected sensor values as a meter widget");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);	
	data.device_id = atoi(argv[1]);
	data.gcu_cmd = gcumeter_cmd;
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	return 0;
	
} 

/**
 * @brief Display selected sensor values as a numeric value using the GCU.
 *
 * This function processes commands to display selected sensor values as a numeric value using the GCU (Graphical Control Unit).
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_gcunumeric(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "displays the selected sensor values as a numeric value");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);	
	data.device_id = atoi(argv[1]);
	data.gcu_cmd = gcunumeric_cmd;
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	// Check if -f command and filter address 
	return 0;
	
} 

/**
 * @brief Start continuously recording selected sensor values to a file on the micro SD Card connected to the GCU.
 *
 * This function processes commands to start continuously recording selected sensor values to a file on the micro SD Card connected to the GCU.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_gcurec_s(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "start continuously recording the selected sensor");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);	
	data.device_id = atoi(argv[1]);
	data.gcu_cmd = gcurec_cmd;
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	// for (int i = 0; i < strlen(argv[2]); i++){ 
	// 	file[i] = argv[2][i]; 
	// }
	// Check if -f command and filter address 
	return 0;
	
} 

/**
 * @brief Stop continuously recording selected sensor values to a file on the micro SD Card connected to the GCU.
 *
 * This function processes commands to start continuously recording selected sensor values to a file on the micro SD Card connected to the GCU.
 *
 * @param sh Pointer to the shell instance.
 * @param argc Number of arguments in the command.
 * @param argv Array of command arguments.
 * @return 0 on success, non-zero on failure.
 */
static int cmd_gcurec_p(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "stop recording");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);	
	data.device_id = atoi(argv[1]);
	data.command = 2; 
	data.gcu_cmd = gcumeter_cmd;
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0) {
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
	// Check if -f command and filter address
	return 0; 
	
} 


static int cmd_add_device(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "Add Device");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);	
	// char *name, uint8_t mac_address, uint16_t major, uint16_t minor,
    //           uint8_t x_coord, uint8_t y_coord, uint8_t left_nb, uint8_t right_nb
	// (b"add mydev 30 40 50 60 70 80 4000 4000 0 2 04 00\n")
	add_node(argv[1], strtol(argv[3], NULL, 16), atoi(argv[8]), atoi(argv[9]), atoi(argv[10]), atoi(argv[11]), strtol(argv[12], NULL, 16), strtol(argv[13], NULL, 16));

}

static int cmd_remove_device(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "Remove Device");
	remove_node_by_name(argv[1]);
	//view_all_nodes();
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);	

}

static int cmd_view_device(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "View All Devices");
	
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);	
	view_all_nodes();
	
}

static int cmd_viewall_device(const struct shell *sh, size_t argc, char **argv){ 
	shell_print(sh, "View Device");
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	view_node_by_name(argv[1]);	


}