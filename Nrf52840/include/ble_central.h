/*
 * File:   ble_central.h
 * Author: Shreya Singh
 *
 * Created on March 19, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the ble_central.c.
 * 
 */

#ifndef BLE_CENTRAL_H
#define BLE_CENTRAL_H
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

#include "../mylib/ble_central.c"


static void start_scan(void);
int ble(void); 
void send(void);

#endif /* BLE_CENTRAL_H*/