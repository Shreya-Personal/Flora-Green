/*
 * File:   ble_devices.c
 * Author: Shreya Singh
 *
 * Created on March 2, 2024, 8:00AM
 * Description: This file contains the implementation of functions used in
 *              the BLE driver.
 */

#include "../include/ble_rules.h"
#include "../include/ahu_shell.h"

int compare_strings(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return 1;  // Strings don't match
        }
        str1++;
        str2++;
    }

    return 0;  // Strings match
}

// Function to remove a node by BLE name
void remove_node_by_name(const char *name) {
    int i, found = 0;

    // Search for the node with the specified BLE name
    for (i = 0; i < num_nodes; i++) {
        if (compare_strings(nodes[i].ble_name, name) == 0) {
            found = 1;
            break;
        }
    }

    if (found) {
        // Remove the node by shifting elements
        for (int j = i; j < num_nodes - 1; j++) {
            nodes[j] = nodes[j + 1];
        }

        // Decrement the number of nodes
        num_nodes--;

        printk("Node with BLE Name '%s' removed.\n", name);
    } else {
        printk("Node with BLE Name '%s' not found.\n", name);
    }
}

void add_node(char *name, uint16_t mac_address, uint16_t major, uint16_t minor,
              uint16_t x_coord, uint16_t y_coord, uint16_t left_nb, uint16_t right_nb) {
    if (num_nodes < MAX_DEVICES) {
        strcpy(nodes2[num_nodes].ble_name, name);
        nodes2[num_nodes].ble_mac_address = mac_address;
        nodes2[num_nodes].ble_major_number = major;
        nodes2[num_nodes].ble_minor_number = minor;
        nodes2[num_nodes].x_coordinate = x_coord;
        nodes2[num_nodes].y_coordinate = y_coord;
        nodes2[num_nodes].left_neighbor = left_nb;
        nodes2[num_nodes].right_neighbor = right_nb;
        num_nodes++;
    } else {
        printk("Cannot add more nodes, max limit reached.\n");
    }
}

void view_node_by_name(char *name) {
    for (int i = 0; i < num_nodes; i++) {
        if (compare_strings(nodes[i].ble_name, name) == 0) {
            printk("Node found:\n");
            printk("  Name: %s\n", nodes2[i].ble_name);
            printk("  MAC Address: 0x%02X\n", nodes2[i].ble_mac_address);
            printk("  Major Number: %d\n", nodes2[i].ble_major_number);
            printk("  Minor Number: %d\n", nodes2[i].ble_minor_number);
            printk("  X Coordinate: %d\n", nodes2[i].x_coordinate);
            printk("  Y Coordinate: %d\n", nodes2[i].y_coordinate);
            printk("  Left Neighbor: 0x%02X\n", nodes2[i].left_neighbor);
            printk("  Right Neighbor: 0x%02X\n", nodes2[i].right_neighbor);
            return; // Found the node, so exit the function
        }
    }
    printk("Node not found with name: %s\n", name);
}

void view_all_nodes(void) {
     printk("Nodes:\n");
    for (int i = 0; i < num_nodes; i++) {
        printk("Node %d:\n", i + 1);
        printk("  Name: %s\n", nodes2[i].ble_name);
        printk("  MAC Address: 0x%02X\n", nodes2[i].ble_mac_address);
        printk("  Major Number: %u\n", nodes2[i].ble_major_number);
        printk("  Minor Number: %u\n", nodes2[i].ble_minor_number);
        printk("  X Coordinate: %u\n", nodes2[i].x_coordinate);
        printk("  Y Coordinate: %u\n", nodes2[i].y_coordinate);
        printk("  Left Neighbor: 0x%02X\n", nodes2[i].left_neighbor);
        printk("  Right Neighbor: 0x%02X\n", nodes2[i].right_neighbor);
        printk("\n");
    }
}