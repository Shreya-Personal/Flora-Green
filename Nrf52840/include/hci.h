/*
 * File:   hci.h
 * Author: Shreya Singh
 *
 * Created on March 2, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the hci.c.
 * 
 */

#ifndef HCI_H
#define HCI_H
#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "../mylib/hci.c"

// Preamble Definition
#define PREAMBLE 0xAA

// Type Definition
#define REQUEST 0x2
#define RESPONSE 0x1

// Device ID
#define GCU_LED 0x1
#define GCU_BUTTON 0x2
#define GCU_DAC 0x3
#define GCU_LJ 0x4
#define TEMP 0x5
#define HUMID 0x6
#define AP 0x7
#define TVOC 0x8
#define SD 13

// Cmd Definition
#define cmd_point 'p'
#define cmd_circle 'c'
#define cmd_lissajous 'j'
#define cmd_dac 'd'
#define cmd_led 'l'
#define cmd_button 'b'
#define gcugraph_cmd 'g'
#define gcumeter_cmd 'm'
#define gcunumeric_cmd 'n'
#define gcurec_cmd 'r'

// Function Definition
void hci_data_packet_poll();
void blink(); 

#endif /* HCI_H*/