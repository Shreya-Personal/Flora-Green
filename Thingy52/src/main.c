/*
 * Copyright (c) 2018 Peter Bigot Consulting, LLC
 * Copyright (c) 2018 Linaro Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor/ccs811.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>


// Define ADC Channels 
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

static bool app_fw_2;

// Define LED node for bluetooth transmission flashing flag
#define LED_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

// Define manufacturer data for bluetooth transmission 
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

//Provides time as a string
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

/**
 * @brief Retrieves CO2 and related sensor data from a CO2 sensor device.
 *
 * This function fetches CO2 concentration data, as well as related sensor data
 * such as VOC (volatile organic compounds) concentration, voltage, and current,
 * from the specified CO2 sensor device. It updates the CO2 concentration data in
 * a global data array for further processing or transmission.
 *
 * @param dev Pointer to the CO2 sensor device.
 * @return 0 if successful, non-zero otherwise.
 */
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

/**
 * @brief Retrieves temperature data from a temperature sensor device.
 *
 * This function fetches temperature data from the specified temperature sensor device
 * using the AMBIENT_TEMP channel. It updates the temperature data in a global data array
 * for further processing or transmission.
 *
 * @param dev Pointer to the temperature sensor device.
 */
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

/**
 * @brief Calculates dB SPL (Sound Pressure Level) from ADC readings.
 *
 * This function calculates the peak-to-peak amplitude of an audio signal from ADC readings
 * over a specified duration (50 milliseconds). It then transforms the peak-to-peak amplitude
 * into decibels (dB SPL) using a specified reference level (SPLref) and scaling factor (PeakRef).
 *
 * @param sequence Pointer to the ADC sequence structure.
 * @param adc_channels Array of ADC channels to be used for reading.
 * @param SPLref Reference level for dB SPL calculation.
 * @param PeakRef Reference level for peak amplitude.
 */
void calculate_dBSPL(struct adc_sequence* sequence, struct adc_sequence* adc_channels,
                     int SPLref, int PeakRef) {
    int peakToPeak = 0;          // Peak-to-peak level
    int dBSPLPeak = 0;           // Peak dB SPL reading
    int signalMax = 0;
    int signalMin = 4096;        // Initialize to the maximum possible ADC value

    uint32_t start = k_uptime_get_32();

    // Collect data for 50 milliseconds
    while (k_uptime_get_32() - start < 50) {
        int err = adc_read_dt(&adc_channels[0], sequence);
        int sample = *(uint16_t*)sequence->buffer;
        if (sample < 4096)  // Toss out spurious readings
        {
            if (sample > signalMax) {
                signalMax = sample;  // save just the max levels
            } else if (sample < signalMin) {
                signalMin = sample;  // save just the min levels
            }
        }
    }

    peakToPeak = signalMax - signalMin;  // Calculate peak-peak amplitude

    // Transform to decibels using the new voltage-based formula
    dBSPLPeak = 4 * (20 * log10((float)abs(peakToPeak)) - SPLref);  // Adjust scaling if needed

    printk("AUDIO %d Max:%d, min%d", dBSPLPeak, signalMax, signalMin);
	mfg_data[3] = (int8_t)dBSPLPeak;
}

/**
 * @brief Performs main application tasks including updating CO2 and temperature sensor values, and handling Bluetooth advertising.
 *
 * This function updates the CO2 sensor value using the specified device pointer, updates the temperature sensor value using
 * the specified temperature device pointer, starts Bluetooth advertising, toggles an LED GPIO pin, and stops Bluetooth advertising
 * after a delay. This function encapsulates the main functionality of the application.
 *
 * @param dev Pointer to the CO2 sensor device.
 * @param temp_dev Pointer to the temperature sensor device.
 */
static void do_main(const struct device *dev, const struct device *temp_dev )
{
	
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
	gpio_pin_toggle_dt(&led);

	k_msleep(1000);
	gpio_pin_toggle_dt(&led);
	err = bt_le_adv_stop();
	k_msleep(1000);
	
}

/**
 * @brief Main function for initializing devices, updating sensor values, and running the application.
 *
 * This function initializes the Bluetooth subsystem, fetches configuration versions,
 * configures ADC channels, and runs the main application loop. The loop continuously
 * calculates dB SPL readings, updates sensor values for temperature and CO2, and performs
 * other main application tasks.
 *
 * @return 0 if successful, non-zero otherwise.
 */
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

	uint32_t count = 0;
	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!adc_is_ready_dt(&adc_channels[i])) {
			printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}

	(void)adc_sequence_init_dt(&adc_channels[0], &sequence);

	//Main
	if (rc == 0) {
		while(true){ 
			calculate_dBSPL(&sequence, adc_channels, 32, 20);
			do_main(dev, temp_dev);
		}
	}
	
	return 0;
}
