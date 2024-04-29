/*
 * File:   ahu_shell.h
 * Author: Shreya Singh
 *
 * Created on March 2, 2023, 8:00 AM
 * Description: This header file contains declarations for functions and
 *              data structures used in the ahu_shell.c.
 * 
 */

#ifndef AHU_SHELL_H
#define AHU_SHELL_H
#include "../mylib/ahu_shell.c"

static int cmd_time_null(const struct shell *sh, size_t argc, char **argv);
static int cmd_time_f(const struct shell *sh, size_t argc, char **argv);
static int cmd_led_s(const struct shell *sh, size_t argc, char **argv);
static int cmd_led_tog(const struct shell *sh, size_t argc, char **argv);
static int cmd_circle_rxy(const struct shell *sh, size_t argc, char **argv);
static int cmd_point_xy(const struct shell *sh, size_t argc, char **argv);
static int cmd_gpd(const struct shell *sh, size_t argc, char **argv);
static int cmd_gled_rgb(const struct shell *sh, size_t argc, char **argv);
static int cmd_dac_xy(const struct shell *sh, size_t argc, char **argv);
static int cmd_lj_abt(const struct shell *sh, size_t argc, char **argv);
static int cmd_blescan_s(const struct shell *sh, size_t argc, char **argv);
static int cmd_blescan_p(const struct shell *sh, size_t argc, char **argv);
static int cmd_blecon_s(const struct shell *sh, size_t argc, char **argv);
static int cmd_blecon_p(const struct shell *sh, size_t argc, char **argv);
static int cmd_gcugraph(const struct shell *sh, size_t argc, char **argv);
static int cmd_gcumeter(const struct shell *sh, size_t argc, char **argv);
static int cmd_gcunumeric(const struct shell *sh, size_t argc, char **argv);
static int cmd_gcurec_s(const struct shell *sh, size_t argc, char **argv);
static int cmd_gcurec_p(const struct shell *sh, size_t argc, char **argv);
void blink(void); 
static int cmd_view_device(const struct shell *sh, size_t argc, char **argv);
static int cmd_viewall_device(const struct shell *sh, size_t argc, char **argv);
static int cmd_add_device(const struct shell *sh, size_t argc, char **argv);
static int cmd_remove_device(const struct shell *sh, size_t argc, char **argv);

//SHELL NODE DEFINITIONS
SHELL_STATIC_SUBCMD_SET_CREATE(sub_viewdevice,
	SHELL_CMD(-a, NULL, "View Device", cmd_view_device),

	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(view, &sub_viewdevice, "Device Commands",cmd_viewall_device);


SHELL_CMD_REGISTER(add, NULL, "Add Device", cmd_add_device);
SHELL_CMD_REGISTER(remove, NULL, "Remove Device", cmd_remove_device);


SHELL_STATIC_SUBCMD_SET_CREATE(sub_time,
	SHELL_CMD(f, NULL, "Format -H -M -S", cmd_time_f),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(time, &sub_time, "Time Commands", cmd_time_null);



SHELL_STATIC_SUBCMD_SET_CREATE(sub_led,
	SHELL_CMD(s, NULL, "Set LEDs", cmd_led_s),
	SHELL_CMD(t, NULL, "Toggle LEDs", cmd_led_tog),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(led, &sub_led, "LED Commands",NULL);


SHELL_STATIC_SUBCMD_SET_CREATE(sub_blescan,
	SHELL_CMD(-s, NULL, "Start Scanning", cmd_blescan_s),
	SHELL_CMD(-p, NULL, "Stop Scanning", cmd_blescan_p),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(blescan, &sub_blescan, "BLE Scan Commands",NULL);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_blecon,
	SHELL_CMD(-s, NULL, "Start Scanning", cmd_blecon_s),
	SHELL_CMD(-p, NULL, "Stop Scanning", cmd_blecon_p),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(blecon, &sub_blecon, "BLE Sensor Commands",NULL);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_gcurec,
	SHELL_CMD(-s, NULL, "Start Recording Sensor", cmd_gcurec_s),
	SHELL_CMD(-p, NULL, "Stop Recording Sensor", cmd_gcurec_p),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);
SHELL_CMD_REGISTER(gcurec, &sub_gcurec, "BLE Record Sensors",NULL);


SHELL_CMD_REGISTER(point, NULL, "Draw a point - point x y", cmd_point_xy);
SHELL_CMD_REGISTER(circle, NULL, "Draw a circle - circle r, x, y", cmd_circle_rxy);
SHELL_CMD_REGISTER(gpd, NULL, "GCU Pushbutton status", cmd_gpd);
SHELL_CMD_REGISTER(gled, NULL, "Toggle GCU LEDs - gled r g b", cmd_gled_rgb);
SHELL_CMD_REGISTER(dac, NULL, "Set DAC values - dac x y",cmd_dac_xy);
SHELL_CMD_REGISTER(lj, NULL, "Set the a and b parameters for the Lissajous curve",cmd_lj_abt);
SHELL_CMD_REGISTER(gcugraph, NULL, "Send sensor value to GCU Graph",cmd_gcugraph);
SHELL_CMD_REGISTER(gcumeter, NULL, "Send sensor value to GCU Meter",cmd_gcumeter);
SHELL_CMD_REGISTER(gcunumeric, NULL, "Send sensor value to GCU DAC",cmd_gcunumeric);

#endif /* AHU_SHELL_H*/