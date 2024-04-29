/*
 * File:   ble_devices.h
 * Author: Shreya Singh
 *
 * Created on March 19, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the BLE
 * 
 */

#ifndef BLE_DEVICES_H
#define BLE_DEVICES_H
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


#include <zephyr/sys/util.h>
#include <zephyr/sys/slist.h>
#include <zephyr/sys/rb.h>
#include <stdio.h>
#include "../mylib/ble_devices.c"


void remove_node_by_name(const char *name);
void add_node(char *name, uint16_t mac_address, uint16_t major, uint16_t minor,
              uint16_t x_coord, uint16_t y_coord, uint16_t left_nb, uint16_t right_nb);
void view_node_by_name(char *name);
void view_all_nodes(void);

#endif /* BLE_DEVICES_H*/