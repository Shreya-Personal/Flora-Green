/*
 * File:   ble_defs.h
 * Author: Shreya Singh
 *
 * Created on March 2, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the the thingy52 driver.
 * 
 */

#ifndef BLE_DEFS_H
#define BLE_DEFS_H

// recv ble definitions 
#define MAX_DEVICES 12
#define ADDRESS_LENGTH 20
#define MAC_LENGTH 17
#define LED_NODE DT_ALIAS(led1)
#define DISCRIMINATOR 0xFF

//Send ble definitions 
#define IBEACON_RSSI 0xc8
#define UUID_SHREYANODE 0x73
#define UUID_SHREYANODE_NUM 0x56
#define STACKSIZE 1024


// Accepted Addresses
char *addresses[] = {
    "2F:53:CF:D0:6C:2D", 
    "F5:75:FE:85:34:67",
    "E5:73:87:06:1E:86",
    "CA:99:9E:FD:98:B1", 
    "CB:1B:89:82:FF:FE", 
    "D4:D2:A0:A4:5C:AC", 
    "C1:13:27:E9:B7:7C",
    "F1:04:48:06:39:A0", 
    "CA:0C:E0:DB:CE:60", 
    "D4:7F:D4:7C:20:13", 
    "F7:0B:21:F1:C8:E1", 
    "FD:E0:8D:FA:3E:4A", 
    "EE:32:F7:28:FA:AC",
};

// Bluetooth Send Functions 
struct Device {
    char address[ADDRESS_LENGTH];  // Assuming addresses are strings of up to 19 characters
    uint8_t address_num[6];  // Array of 6 uint8_t for the address
    int rssi;                      // RSSI value
};

struct Device devices[MAX_DEVICES]; // Global array to hold device information
int num_devices = 0;  

// Struct of Devices 
static uint8_t mfg_data[] ={DISCRIMINATOR, 0x00, /* Apple */
    0x02, 0x15, /* iBeacon */
    UUID_SHREYANODE,UUID_SHREYANODE_NUM, // UUID for IBEACON MOBILE
    0x00, 0xFF, //ADDR1_1, ADDR1_2, 
    0x00, 0x00, //ADDR1_3, ADDR1_4,
    0x00, 0x00, //ADDR1_5, ADDR1_6,
    0x00, UUID_SHREYANODE, //ADDR1_RSSI, UUID_SHREYANODE, 
    0x00, 0xFF,//ADDR2_1, ADDR2_2,s
    0x00, 0x00, //ADDR2_3, ADDR2_4, 
    0x00, 0x00, //ADDR2_5, ADDR2_6, 
    0x00, UUID_SHREYANODE, //ADDR2_RSSI, UUID_SHREYANODE, 
    0x00, 0x00, //CURRENT PACKET , PACKET_NUM, 
    UUID_SHREYANODE_NUM, // END of transmission
};

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR), // Preamble 
    BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 25),
};



#endif /* BLE_DEFS_H*/