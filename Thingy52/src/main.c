/*
 * Copyright (c) 2018 Peter Bigot Consulting, LLC
 * Copyright (c) 2018 Linaro Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor/ccs811.h>
#include <stdio.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor/ccs811.h>

static bool app_fw_2;
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
#define LED_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

static uint8_t mfg_data[] = {  0xFF, /*Preamble*/
		      0x73, 0x56, /* UUID */
		      0x00, /*Sound*/
			  0x00, 0x00, /*TempUB, TEMPLB*/
			  0x00, 0x00, /*CO2UB, CO2UB*/
			  0x73, /* End of Transmission*/
			
};
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR), // Preamble 
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 9),
};

static const char *now_str(void)
{
	static char buf[16]; /* ...HH:MM:SS.MMM */
	uint32_t now = k_uptime_get_32();
	unsigned int ms = now % MSEC_PER_SEC;
	unsigned int s;
	unsigned int min;
	unsigned int h;

	now /= MSEC_PER_SEC;
	s = now % 60U;
	now /= 60U;
	min = now % 60U;
	now /= 60U;
	h = now;

	snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u",
		 h, min, s, ms);
	return buf;
}

static int get_co2(const struct device *dev)
{
	struct sensor_value co2, tvoc, voltage, current;
	int rc = 0;
	int baseline = -1;
	if (rc == 0) {
		rc = sensor_sample_fetch(dev);
	}
	if (rc == 0) {
		const struct ccs811_result_type *rp = ccs811_result(dev);

		sensor_channel_get(dev, SENSOR_CHAN_CO2, &co2);
		sensor_channel_get(dev, SENSOR_CHAN_VOC, &tvoc);
		sensor_channel_get(dev, SENSOR_CHAN_VOLTAGE, &voltage);
		sensor_channel_get(dev, SENSOR_CHAN_CURRENT, &current);
		printk("\n[%s]: CCS811: %u ppm eCO2; %u ppb eTVOC\n",
		       now_str(), co2.val1, tvoc.val1);
		printk("Voltage: %d.%06dV; Current: %d.%06dA\n", voltage.val1,
		       voltage.val2, current.val1, current.val2);
		
		mfg_data[6] = (uint8_t)(co2.val1 >> 8);
		mfg_data[7] = (uint8_t)(co2.val1 & 0xFF);
		

		if (app_fw_2 && !(rp->status & CCS811_STATUS_DATA_READY)) {
			printk("STALE DATA\n");
		}

		if (rp->status & CCS811_STATUS_ERROR) {
			printk("ERROR: %02x\n", rp->error);
		}
	}
	return rc;
}

static void get_temp(const struct device *dev)
{
	static unsigned int obs;
	struct sensor_value temp;

	if (sensor_sample_fetch(dev) < 0) {
		printk("Sensor sample update error\n");
		return;
	}

	if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
		printk("Cannot read LPS22HB temperature channel\n");
		return;
	}

	// Byte 4 and 5: Temperature 
    uint16_t converted = (int)(sensor_value_to_double(&temp)*10);
    mfg_data[4] = (uint8_t)(converted >> 8);
    mfg_data[5] = (uint8_t)(converted & 0xFF);

}

static void do_main(const struct device *dev, const struct device *temp_dev )
{
	while (true) {
		//Update Co2
		int rc = get_co2(dev);
		//Update Temperature 
		get_temp(temp_dev);


		int err;
		/* Start advertising */
		err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad),
					NULL, 0);
		
		if (err) {
			printk("Advertising failed to start (err %d)\n", err);
			
			return 0;
		}

		k_msleep(1000);
		gpio_pin_toggle_dt(&led);
		err = bt_le_adv_stop();
		k_msleep(1000);
	}
}

int main(void)
{
	const struct device *const dev = DEVICE_DT_GET_ONE(ams_ccs811);
	const struct device *const temp_dev = DEVICE_DT_GET_ONE(st_lps22hb_press);
	struct ccs811_configver_type cfgver;
	int rc;
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	printk("Starting Broadcaster\n");
	int err; 
	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	if (!device_is_ready(dev)) {
		printk("Device %s is not ready\n", dev->name);
		return 0;
	}

	printk("device is %p, name is %s\n", dev, dev->name);

	rc = ccs811_configver_fetch(dev, &cfgver);
	if (rc == 0) {
		printk("HW %02x; FW Boot %04x App %04x ; mode %02x\n",
		       cfgver.hw_version, cfgver.fw_boot_version,
		       cfgver.fw_app_version, cfgver.mode);
		app_fw_2 = (cfgver.fw_app_version >> 8) > 0x11;
	}

	//Main
	if (rc == 0) {
		do_main(dev, temp_dev);
	}
	
	return 0;
}
