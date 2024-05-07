/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/rb.h>
#include "../include/hci.h"
#include "../include/ble_devices.h"
#include "../include/ble_rules.h"
#include "../../Thingy52/include/ble_defs.h"
#include "../include/ahu_shell.h"

#define FILTERENABLE 'f'
#define UUID_SHREYANODE 0x56
#define MAX_DEVICES 12
#define TEMP_BUF_SIZE 50  // Adjust the size as needed
#define SERIAL_BUF_SIZE 209 // Adjust the size as needed
// struct data_item_type data;
static void start_scan(void);

char oldbuf[SERIAL_BUF_SIZE]; 
char serialbuf[SERIAL_BUF_SIZE]; 

int compareStrings(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return 1;  // Strings are different
        }
        str1++;
        str2++;
    }
    if (strlen(str1) == strlen(str2)){ 
        return 0; 
    }
}


void data_redirect(const uint8_t *data_ptr){ 
    // printk("DEVICE FOUND\n"); 
    // char buf[64];
    uint16_t temperature = (uint16_t)((data_ptr[4] << 8) | data_ptr[5]);
    uint16_t co2 = (uint16_t)((data_ptr[6] << 8) | data_ptr[7]);
    uint16_t sound = data_ptr[3];
    printk("JSON{'Temp': %u, 'Co2': %u, 'Sound': %u}JSON\n", temperature, co2, sound); 
}


/**
 * @brief Handle Extended Inquiry Response (EIR) data when a Bluetooth device is found.
 *
 * This function is called when Extended Inquiry Response (EIR) data is found during a Bluetooth scan.
 * It processes the EIR data and performs specific actions based on the data received.
 *
 * @param mfg_data Pointer to the manufacturer-specific data in the EIR.
 * @param user_data User-specific data passed to the function (e.g., Bluetooth device address).
 *
 * @return true if the EIR data is processed successfully, false otherwise.
 */
static bool eir_found(struct bt_data *mfg_data, void *user_data)
{
	bt_addr_le_t *addr = user_data;
	int i;
	
	// Access data
    const uint8_t *data_ptr = mfg_data->data;
    if (data_ptr[1] == 0x73){ 
        data_redirect(data_ptr); 
    }
    return true;
}

/**
 * @brief Handle Bluetooth Low Energy (BLE) scan data.
 *
 * This function processes BLE scan data and performs specific actions based on the data received.
 *
 * @param data Data item containing information about the BLE device.
 * @param addr Bluetooth device address.
 * @param rssi Received Signal Strength Indication (RSSI) value.
 * @param type Type of BLE data.
 * @param ad Pointer to the advertisement data.
 */

void BLE_scan_handle(struct data_item_type data, const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad){ 
    char dev[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, dev, sizeof(dev));
    printk("Device: %s (RSSI %d), type %u, AD data len %u\n",
	            dev, rssi, type, ad->len);

}

/**
 * @brief Handle discovery of Bluetooth devices.
 *
 * This function is called when a Bluetooth device is discovered during scanning.
 * It processes the device information and performs specific actions based on the data received.
 *
 * @param addr Bluetooth device address.
 * @param rssi Received Signal Strength Indication (RSSI) value.
 * @param type Type of Bluetooth advertising data.
 * @param ad Pointer to the advertisement data.
 */

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
    //Parse the data
    bt_data_parse(ad, eir_found, (void *)addr);


}

/**
 * @brief Start Bluetooth Low Energy (BLE) scanning.
 *
 * This function starts scanning for BLE devices using active scanning mode and handles
 * discovered devices using the device_found callback function.
 */

static void start_scan(void)
{
	int err;

	/* Use active scanning and disable duplicate filtering to handle any
	 * devices that might update their advertising data at runtime. */
	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_ACTIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}


/**
 * @brief Initialize Bluetooth and start BLE scanning as a thread.
 *
 * This function initializes Bluetooth Low Energy (BLE) and starts scanning for BLE devices.
 * It is intended to be used as a thread to handle Bluetooth operations concurrently.
 *
 * @return 0 on success, non-zero on failure.
 */

int ble(void)
{   

	int err;
	err = bt_enable(NULL);
    
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	printk("Bluetooth initialized\n");

	start_scan();
	return 0;
}


/**
 * @brief Update messages continuously.
 *
 * This function continuously retrieves data items from a message queue and processes them.
 * It runs in an infinite loop and waits indefinitely for new data items to be available in the queue.
 */

void message_update(void)
{
    while (1) {
        /* get a data item */
        k_msgq_get(&my_msgq, &data, K_FOREVER);
        //k_msgq_get(&my_uartmsgq, &uart_poll, K_FOREVER);
    }
}

void send(void){ 
	while(1){ 
		printk("receieving %02X\n", mfg_data[0]); 
		k_msleep(1000); 
	}
	return; 
}