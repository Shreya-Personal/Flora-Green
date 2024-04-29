/*
 * File:   ahu_timer.c
 * Author: Shreya Singh
 *
 * Created on March 2, 2024, 8:00AM
 * Description: This file contains the implementation of functions used in
 *              the AHU driver.
 */
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <version.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>


// Timer Timeout 
int period = 8.64e+7;
/*Logging Node*/
//LOG_MODULE_REGISTER(AHU_TIMER_MODULE, LOG_LEVEL_DBG, LOG_LEVEL_ERR, LOG_LEVEL_WRN, LOG_LEVEL_INF);

/**
 * @brief Timeout callback function for the my_status_timer.
 *
 * This function is called when the my_status_timer expires, and it logs an error message "Timeout".
 *
 * @param t Pointer to the k_timer structure associated with the timer.
 */
void timeout(struct k_timer *t)
{
	LOG_ERR("Timeout");
}
K_TIMER_DEFINE(my_status_timer, timeout, NULL);

void ahu_system_timer_init(){ 
    k_timer_start(&my_status_timer, K_MSEC(period), K_NO_WAIT);
}

/**
 * @brief Get the system time based on the my_status_timer remaining time.
 *
 * This function calculates the system time based on the remaining time of the my_status_timer,
 * which is measured in milliseconds. It returns the system time in seconds.
 *
 * @return The system time in seconds.
 */
int get_system_time_null(){ 
    int system_time;
	system_time = (period-k_timer_remaining_get(&my_status_timer))/1000;
	return system_time;
}

/**
 * @brief Get the system time as hours, minutes, and seconds based on the my_status_timer remaining time.
 *
 * This function calculates the system time based on the remaining time of the my_status_timer,
 * which is measured in milliseconds. It returns an integer array containing the system time
 * in hours, minutes, and seconds, respectively.
 *
 * @return Pointer to an integer array containing the system time in hours, minutes, and seconds.
 *         The array has a length of 3, where index 0 corresponds to hours, index 1 to minutes,
 *         and index 2 to seconds.
 *         Note: The caller is responsible for freeing the memory allocated for the returned array.
 */
int* get_system_time_f(){ 
    int system_time;
	system_time = (period-k_timer_remaining_get(&my_status_timer))/1000;
    int* result = (int*)malloc(3 * sizeof(int));
	result[0] = system_time / 3600;
    result[1] = (system_time % 3600) / 60;
    result[2] = system_time % 60;
	return result; 
}

