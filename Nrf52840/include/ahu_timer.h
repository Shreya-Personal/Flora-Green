/*
 * File:   ahu_timer.h
 * Author: Shreya Singh
 *
 * Created on March 2, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the ahu_timer.c.
 * 
 */

#ifndef AHU_TIMER_H
#define AHU_TIMER_H
#include "../mylib/ahu_timer.c"

void ahu_system_timer_init(); 
int get_system_time_null();
int* get_system_time_f();


#endif /* AHU_TIMER_H*/