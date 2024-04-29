# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(shell_module)

target_sources(app PRIVATE src/main.c mylib/test_module.c)
target_sources_ifdef(CONFIG_SHELL_DYNAMIC_CMDS app PRIVATE mylib/dynamic_cmd.c)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE mylib/uart_reinit.c)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE include/ahu_led.h)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE include/ahu_shell.h)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE include/ahu_timer.h)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE include/ahu_uart.h)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE include/hci.h)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE include/ble_central.h)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE include/ble_devices.h)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE include/ble_rules.h)
target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE ../Thingy52/include/ble_defs.h)