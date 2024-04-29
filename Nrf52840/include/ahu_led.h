/*
 * File:   ahu_led.h
 * Author: Shreya Singh
 *
 * Created on March 2, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the ahu_led.c.
 * 
 */
#ifndef AHU_LED_H
#define AHU_LED_H

#include <ctype.h>
#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include "../mylib/ahu_led.c"


void ahu_led_init(void);
void set_ahu_led(int led, const char *status); 
int get_ahu_led_status(int led);
void toggle_ahu_led(int led); 

#endif /* AHU_LED_H*/