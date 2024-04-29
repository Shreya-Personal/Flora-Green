/*
 * File:   ble_devices.h
 * Author: Shreya Singh
 *
 * Created on March 19, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the BLE
 * 
 */

#ifndef BLE_RULES_H
#define BLE_RULES_H
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
#include "../../Thingy52/include/ble_defs.h"
#define MAX_DEVICES 12
#define UUID_LENGTH 6

// Define the struct for iBeacon node
struct ibeacon_node {
    char ble_name[32]; // BLE Name
    char ble_mac_address[18]; // BLE MAC address
    uint16_t ble_major_number; // BLE major number
    uint16_t ble_minor_number; // BLE minor number
    int16_t x_coordinate; // Fixed X coordinate
    int16_t y_coordinate; // Fixed Y coordinate
    int16_t left_neighbor; // hex position [x,y]
    int16_t right_neighbor; // hex position [x,y]
};
// Define the struct for iBeacon node
struct ibeacon_node2 {
    char ble_name[32]; // BLE Name
    uint16_t ble_mac_address; // BLE MAC address
    uint16_t ble_major_number; // BLE major number
    uint16_t ble_minor_number; // BLE minor number
    int16_t x_coordinate; // Fixed X coordinate
    int16_t y_coordinate; // Fixed Y coordinate
    int16_t left_neighbor; // hex position [x,y]
    int16_t right_neighbor; // hex position [x,y]
};

int num_nodes = 8; 

struct ibeacon_node nodes[MAX_DEVICES] = {
    [0] = {
        .ble_name = "4011-A",
        .ble_mac_address = "F5:75:FE:85:34:67",
        .ble_major_number = 2753,
        .ble_minor_number = 32998,
        .x_coordinate = 0,
        .y_coordinate = 0,
        .left_neighbor = 0x02,
        .right_neighbor = 0x20,
    },
    [1] = {
        .ble_name = "4011-B",
        .ble_mac_address = "E5:73:87:06:1E:86",
        .ble_major_number = 32975,
        .ble_minor_number = 20959,
        .x_coordinate = 2,
        .y_coordinate = 0,
        .left_neighbor = 0x00,
        .right_neighbor = 0x40,
    },
    [2] = {
        .ble_name = "4011-C",
        .ble_mac_address = "CA:99:9E:FD:98:B1",
        .ble_major_number = 26679,
        .ble_minor_number = 40363,
        .x_coordinate = 4,
        .y_coordinate = 0,
        .left_neighbor = 0x20,
        .right_neighbor = 0x42,
    },
    [3] = {
        .ble_name = "4011-D",
        .ble_mac_address = "CB:1B:89:82:FF:FE",
        .ble_major_number = 41747,
        .ble_minor_number = 38800,
        .x_coordinate = 4,
        .y_coordinate = 2,
        .left_neighbor = 0x44,
        .right_neighbor = 0x40,
    },
    [4] = {
        .ble_name = "4011-E",
        .ble_mac_address = "D4:D2:A0:A4:5C:AC",
        .ble_major_number = 30679,
        .ble_minor_number = 51963,
        .x_coordinate = 4,
        .y_coordinate = 4,
        .left_neighbor = 0x42,
        .right_neighbor = 0x24,
    },
    [5] = {
        .ble_name = "4011-F",
        .ble_mac_address = "C1:13:27:E9:B7:7C",
        .ble_major_number = 6195,
        .ble_minor_number = 18394,
        .x_coordinate = 2,
        .y_coordinate = 4,
        .left_neighbor = 0x44,
        .right_neighbor = 0x04,
    },
    [6] = {
        .ble_name = "4011-G",
        .ble_mac_address = "F1:04:48:06:39:A0",
        .ble_major_number = 30525,
        .ble_minor_number = 30544,
        .x_coordinate = 0,
        .y_coordinate = 4,
        .left_neighbor = 0x24,
        .right_neighbor = 0x02,
    },
    [7] = {
        .ble_name = "4011-H",
        .ble_mac_address = "CA:0C:E0:DB:CE:60",
        .ble_major_number = 57395,
        .ble_minor_number = 28931,
        .x_coordinate = 0,
        .y_coordinate = 2,
        .left_neighbor = 0x04,
        .right_neighbor = 0x00,
    },

};

struct ibeacon_node2 nodes2[MAX_DEVICES] = {
    [0] = {
        .ble_name = "4011-A",
        .ble_mac_address = 0x75,
        .ble_major_number = 2753,
        .ble_minor_number = 32998,
        .x_coordinate = 0,
        .y_coordinate = 0,
        .left_neighbor = 0x02,
        .right_neighbor = 0x20,
    },
    [1] = {
        .ble_name = "4011-B",
        .ble_mac_address = 0x73,
        .ble_major_number = 32975,
        .ble_minor_number = 20959,
        .x_coordinate = 2,
        .y_coordinate = 0,
        .left_neighbor = 0x00,
        .right_neighbor = 0x40,
    },
    [2] = {
        .ble_name = "4011-C",
        .ble_mac_address = 0x99,
        .ble_major_number = 26679,
        .ble_minor_number = 40363,
        .x_coordinate = 4,
        .y_coordinate = 0,
        .left_neighbor = 0x20,
        .right_neighbor = 0x42,
    },
    [3] = {
        .ble_name = "4011-D",
        .ble_mac_address = 0x1B,
        .ble_major_number = 41747,
        .ble_minor_number = 38800,
        .x_coordinate = 4,
        .y_coordinate = 2,
        .left_neighbor = 0x44,
        .right_neighbor = 0x40,
    },
    [4] = {
        .ble_name = "4011-E",
        .ble_mac_address = 0xD2,
        .ble_major_number = 30679,
        .ble_minor_number = 51963,
        .x_coordinate = 4,
        .y_coordinate = 4,
        .left_neighbor = 0x42,
        .right_neighbor = 0x24,
    },
    [5] = {
        .ble_name = "4011-F",
        .ble_mac_address = 0x13,
        .ble_major_number = 6195,
        .ble_minor_number = 18394,
        .x_coordinate = 2,
        .y_coordinate = 4,
        .left_neighbor = 0x44,
        .right_neighbor = 0x04,
    },
    [6] = {
        .ble_name = "4011-G",
        .ble_mac_address = 0x04,
        .ble_major_number = 30525,
        .ble_minor_number = 30544,
        .x_coordinate = 0,
        .y_coordinate = 4,
        .left_neighbor = 0x24,
        .right_neighbor = 0x02,
    },
    [7] = {
        .ble_name = "4011-H",
        .ble_mac_address = 0x0C,
        .ble_major_number = 57395,
        .ble_minor_number = 28931,
        .x_coordinate = 0,
        .y_coordinate = 2,
        .left_neighbor = 0x04,
        .right_neighbor = 0x00,
    },
    // [8] = {
    //     .ble_name = "4011-I",
    //     .ble_mac_address = 0xFF,
    //     .ble_major_number = 2753,
    //     .ble_minor_number = 32998,
    //     .x_coordinate = 2,
    //     .y_coordinate = 4,
    //     .left_neighbor = 0x02,
    //     .right_neighbor = 0x20,
    // },

};

// Struct of Devices 
static uint8_t UUID_data[] ={DISCRIMINATOR, 0x00, /* Apple */
    0x02, 0x15, /* iBeacon */
    UUID_SHREYANODE,UUID_SHREYANODE_NUM, // UUID for IBEACON MOBILE
    0x00, 0x00, //ADDR1_1, ADDR1_2, 
    0x00, 0x00, //ADDR1_3, ADDR1_4,
    0x00, 0x00, //ADDR1_5, ADDR1_6,
    0x00, UUID_SHREYANODE, //ADDR1_RSSI, UUID_SHREYANODE, 
    0x00, 0x00,//ADDR2_1, ADDR2_2,s
    0x00, 0x00, //ADDR2_3, ADDR2_4, 
    0x00, 0x00, //ADDR2_5, ADDR2_6, 
    0x00, UUID_SHREYANODE, //ADDR2_RSSI, UUID_SHREYANODE, 
    0x00, 0x00, //0x00, PACKET_NUM, 
    UUID_SHREYANODE_NUM, // END of transmission
};

    // [8] = {
    //     .ble_name = "4011-I",
    //     .ble_mac_address = "00:00:00:00:00:00",
    //     .ble_major_number = 2753,
    //     .ble_minor_number = 32998,
    //     .x_coordinate = 3,
    //     .y_coordinate = 4,
    //     .left_neighbor = 0x02,
    //     .right_neighbor = 0x20,
    // },
#endif /* BLE_RULES_H*/