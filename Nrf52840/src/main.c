/*
 * File:   main.c
 * Author: Shreya Singh
 *
 * Created on March 2, 2024, 8:00AM
 * Description: AHU Driver Main 
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
#include <zephyr/device.h>
#include <string.h>


#ifdef CONFIG_ARCH_POSIX
#include <unistd.h>
#else
#include <zephyr/posix/unistd.h>
#endif

//Library Files 
#include "../include/ahu_led.h"
#include "../include/ahu_shell.h"
#include "../include/ahu_timer.h"
#include "../include/ahu_uart.h"
#include "../include/hci.h"
#include <zephyr/sys/rb.h>
#include "../include/hci.h"
#include "../include/ble_devices.h"
#include "../include/ble_rules.h"
#include "../include/ble_central.h"


/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

//SHELL TEST FUNCTION 


// UART TEST FUNCTION
static int sendsend(const struct shell *sh, size_t argc, char **argv){ 
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	send_str(uart1, "ITS WORKING\n");
	return 0;
}

SHELL_CMD_REGISTER(send, NULL, "Send UART", sendsend);

/**
 * @brief Initializes the functionality of the system.
 * 
 * This function initializes the AHU LED and system timer functionality.
 * 
 * @note Make sure to call this function before using any other system functionalities.
 */
void init(void){ 
	ahu_led_init(); 
	ahu_system_timer_init();
}

/* 
 * Initializes the system and enables shell functionality.
 * This function sets up necessary components and configurations
 * to start the system and activate the shell for user interaction.
 * It includes initialization of system components like LEDs, timers,
 * and other peripherals required for system operation.
 * 
 * Returns:
 * - 0 if initialization is successful.
 */
int shell(void)
{
	init(); 
	
#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_shell_uart), zephyr_cdc_acm_uart)
	const struct device *dev;
	uint32_t dtr = 0;

	dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	if (!device_is_ready(dev) || usb_enable(NULL)) {
		return 0;
	}
	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		k_sleep(K_MSEC(100));
		

	}
#endif
	return 0;
}

// // Initialise Message Queues
// K_MSGQ_DEFINE(my_msgq, sizeof(struct data_item_type), 10, 1); // BLE msg queue
// // K_MSGQ_DEFINE(my_uartmsgq, sizeof(struct uart_item_type), 10, 1); // UART msg queue

// // // Initialise Threads 
K_THREAD_DEFINE(shell_id, STACKSIZE, shell, NULL, NULL, NULL,
			2, 0, 0); // Shell Thread 

K_THREAD_DEFINE(ble_id, STACKSIZE, ble, NULL, NULL, NULL,
7, 0, 0); // Bluetooth Thread 

// K_THREAD_DEFINE(message_update_id, STACKSIZE, send, NULL, NULL, NULL,
// 1, 0, 0); // BLE update message 

// K_THREAD_DEFINE(HCI_poll_id, STACKSIZE, hci_data_packet_poll, NULL, NULL, NULL,
// 9, 0, 0); // UART poll thread 

// K_THREAD_DEFINE(uart_id, STACKSIZE, uart_update, NULL, NULL, NULL,
// 7, 0, 0);// Update UART message thread 




