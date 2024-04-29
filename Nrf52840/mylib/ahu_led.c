/*
 * File:   ahu_led.c
 * Author: Shreya Singh
 *
 * Created on March 2, 2024, 8:00AM
 * Description: This file contains the implementation of functions used in
 *              the AHU driver.
 * 
 */
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

/*status flag: 0 - off, 1 - on */ 
int led0_flag = 0; int led1_flag = 0; int led2_flag = 0; int led3_flag =0; 


/*Logging Node*/
LOG_MODULE_REGISTER(AHU_LED_MODULE, LOG_LEVEL_DBG, LOG_LEVEL_ERR, LOG_LEVEL_WRN, LOG_LEVEL_INF);

/* Initialise AHU LEDs*/
void ahu_led_init(void){ 
    int ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    int ret2 = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    int ret3 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    int ret4 = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    if (ret && ret2 && ret3 && ret4 == 0){ 
        LOG_DBG("LED INIT:OK");
    }
    return;
}

/**
 * @brief Set the status of an AHU (Air Handling Unit) LED.
 *
 * This function controls the status (on/off) of an AHU LED based on the provided parameters.
 *
 * @param led The LED number to control (0-3).
 * @param status The status of the LED ("on" or "off").
 */

void set_ahu_led(int led, const char *status){ 
    if (strcmp(status, "on") == 0) { 
        switch(led){ 
            case 0:
                if (led0_flag == 0) { 
                   gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE); 
                   led0_flag = 1; 
                   LOG_DBG("LED 0 is on");
                   break;
                }else if (led0_flag == 1){
                    LOG_WRN("LED 0 already on");
                    break;
                }      
            case 1: 
                if (led1_flag == 0) { 
                   gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE); 
                   led1_flag = 1; 
                   LOG_DBG("LED 1 is on");
                   break;
                }else if (led1_flag == 1){
                    LOG_WRN("LED 1 already on");
                    break;
                }
            case 2: 
                if (led2_flag == 0) { 
                   gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE); 
                   led2_flag = 1; 
                   LOG_DBG("LED 2 is on");
                   break;
                }else if (led2_flag == 1){
                    LOG_WRN("LED 2 already on");
                    break;
                }
            case 3: 
                if (led3_flag == 0) { 
                   gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE); 
                   led3_flag = 1; 
                   LOG_DBG("LED 3 is on");
                   break;
                }else if (led3_flag == 1){
                    LOG_WRN("LED 3 already on");
                    break; 
                }

        }
            
    } else { 
       switch(led){ 
            case 0:
                if (led0_flag == 1) { 
                   gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE); 
                   led0_flag = 0; 
                   LOG_DBG("LED 0 is off");
                   break;
                }else{
                    LOG_WRN("LED 0 already off");
                    break;
                }               
            case 1: 
                if (led1_flag == 1) { 
                   gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE); 
                   led1_flag = 0; 
                   LOG_DBG("LED 1 is off");
                   break;
                }else{
                    LOG_WRN("LED 1 already off");
                    break;
                }
            case 2: 
                if (led2_flag == 1) { 
                   gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE); 
                   led2_flag = 0; 
                   LOG_DBG("LED 2 is off");
                   break;
                }else{
                    LOG_WRN("LED 2 already off");
                    break;
                }
            case 3: 
                if (led3_flag == 1) { 
                   gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE); 
                   led3_flag = 0; 
                   LOG_DBG("LED 3 is off");
                   break;
                }else{
                    LOG_WRN("LED 3 already off");
                    break;
                }
        }
    }
}

/**
 * @brief Get the status of an AHU (Air Handling Unit) LED.
 *
 * This function retrieves the current status (on/off) of an AHU LED.
 *
 * @param led The LED number to check status for (0-3).
 *
 * @return 1 if the LED is on, 0 if the LED is off or the LED number is invalid.
 */

int get_ahu_led_status(int led){ 
    switch(led){ 
        case 0: 
            return led0_flag; 
        case 1: 
            return led1_flag;
        case 2: 
            return led2_flag;
        case 3: 
            return led3_flag;
    }
    return 0; 
}

/**
 * @brief Toggle the status of an AHU (Air Handling Unit) LED.
 *
 * This function toggles the status (on/off) of an AHU LED based on the provided LED number.
 *
 * @param led The LED number to toggle (0-3).
 */
void toggle_ahu_led(int led){ 
    switch(led){ 
            case 0:
                gpio_pin_toggle_dt(&led0);
                led0_flag = !led0_flag;  
                LOG_DBG("LED 0 was toggled");
                break;       
            case 1: 
                gpio_pin_toggle_dt(&led1);
                led1_flag = !led1_flag; 
                LOG_DBG("LED 1 was toggled");
                break;
            case 2: 
                gpio_pin_toggle_dt(&led2);
                led2_flag = !led2_flag; 
                LOG_DBG("LED 2 was toggled");
                break;
            case 3: 
                gpio_pin_toggle_dt(&led3);
                led3_flag = !led3_flag;
                LOG_DBG("LED 3 was toggled");
                break;
    }
} 
