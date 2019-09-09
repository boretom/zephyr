/*
 * Copyright (c) 2018 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/util.h>

static inline float out_ev(struct sensor_value *val)
{
	return (val->val1 + (float)val->val2 / 1000000);
}

static int print_samples;
static int lis2dh_trig_cnt;

static struct sensor_value accel_x_out, accel_y_out, accel_z_out;
static struct sensor_value gyro_x_out, gyro_y_out, gyro_z_out;
#if defined(CONFIG_LIS2DH_EXT0_LIS2MDL)
static struct sensor_value magn_x_out, magn_y_out, magn_z_out;
#endif
#if defined(CONFIG_LIS2DH_EXT0_LPS22HB)
static struct sensor_value press_out, temp_out;
#endif

#ifdef CONFIG_LIS2DH_TRIGGER
static void lis2dh_trigger_handler(struct device *dev,
				    struct sensor_trigger *trig)
{
	static struct sensor_value accel_x, accel_y, accel_z;
	static struct sensor_value gyro_x, gyro_y, gyro_z;
#if defined(CONFIG_LIS2DH_EXT0_LIS2MDL)
	static struct sensor_value magn_x, magn_y, magn_z;
#endif
#if defined(CONFIG_LIS2DH_EXT0_LPS22HB)
	static struct sensor_value press, temp;
#endif
	lis2dh_trig_cnt++;

	sensor_sample_fetch_chan(dev, SENSOR_CHAN_ACCEL_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &accel_x);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &accel_z);

	/* lis2dh gyro */
	sensor_sample_fetch_chan(dev, SENSOR_CHAN_GYRO_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_X, &gyro_x);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_Y, &gyro_y);
	sensor_channel_get(dev, SENSOR_CHAN_GYRO_Z, &gyro_z);

#if defined(CONFIG_LIS2DH_EXT0_LIS2MDL)
	/* lis2dh external magn */
	sensor_sample_fetch_chan(dev, SENSOR_CHAN_MAGN_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_MAGN_X, &magn_x);
	sensor_channel_get(dev, SENSOR_CHAN_MAGN_Y, &magn_y);
	sensor_channel_get(dev, SENSOR_CHAN_MAGN_Z, &magn_z);
#endif

#if defined(CONFIG_LIS2DH_EXT0_LPS22HB)
	/* lis2dh external press/temp */
	sensor_sample_fetch_chan(dev, SENSOR_CHAN_PRESS);
	sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press);

	sensor_sample_fetch_chan(dev, SENSOR_CHAN_AMBIENT_TEMP);
	sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
#endif

	if (print_samples) {
		print_samples = 0;

		accel_x_out = accel_x;
		accel_y_out = accel_y;
		accel_z_out = accel_z;

		gyro_x_out = gyro_x;
		gyro_y_out = gyro_y;
		gyro_z_out = gyro_z;

#if defined(CONFIG_LIS2DH_EXT0_LIS2MDL)
		magn_x_out = magn_x;
		magn_y_out = magn_y;
		magn_z_out = magn_z;
#endif

#if defined(CONFIG_LIS2DH_EXT0_LPS22HB)
		press_out = press;
		temp_out = temp;
#endif
	}

}
#endif

void main(void)
{
	int cnt = 0;
	char out_str[64];
	struct sensor_value odr_attr;
	struct device *lis2dh_dev = device_get_binding(DT_INST_0_ST_LIS2DH_LABEL);

	if (lis2dh_dev == NULL) {
		printk("Could not get LIS2DH device\n");
		return;
	}

	/* set accel/gyro sampling frequency to 104 Hz */
	odr_attr.val1 = 100;
	odr_attr.val2 = 0;

	if (sensor_attr_set(lis2dh_dev, SENSOR_CHAN_ACCEL_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
		printk("Cannot set sampling frequency for accelerometer.\n");
		return;
	}

	if (sensor_attr_set(lis2dh_dev, SENSOR_CHAN_GYRO_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
		printk("Cannot set sampling frequency for gyro.\n");
		return;
	}

#ifdef CONFIG_LIS2DH_TRIGGER
	struct sensor_trigger trig;

	trig.type = SENSOR_TRIG_DATA_READY;
	trig.chan = SENSOR_CHAN_ACCEL_XYZ;

	if (sensor_trigger_set(lis2dh_dev, &trig, lis2dh_trigger_handler) != 0) {
		printk("Could not set sensor type and channel\n");
		return;
	}
#endif

	if (sensor_sample_fetch(lis2dh_dev) < 0) {
		printk("Sensor sample update error\n");
		return;
	}

	while (1) {
		/* Erase previous */
		printk("\0033\014");
		printf("LIS2DH sensor samples:\n\n");

		/* lis2dh accel */
		sprintf(out_str, "accel (%f %f %f) m/s2", out_ev(&accel_x_out),
							out_ev(&accel_y_out),
							out_ev(&accel_z_out));
		printk("%s\n", out_str);

		/* lis2dh gyro */
		sprintf(out_str, "gyro (%f %f %f) dps", out_ev(&gyro_x_out),
							out_ev(&gyro_y_out),
							out_ev(&gyro_z_out));
		printk("%s\n", out_str);

#if defined(CONFIG_LIS2DH_EXT0_LIS2MDL)
		/* lis2dh external magn */
		sprintf(out_str, "magn (%f %f %f) gauss", out_ev(&magn_x_out),
							 out_ev(&magn_y_out),
							 out_ev(&magn_z_out));
		printk("%s\n", out_str);
#endif

#if defined(CONFIG_LIS2DH_EXT0_LPS22HB)
		/* lis2dh external press/temp */
		sprintf(out_str, "press (%f) kPa - temp (%f) deg",
			out_ev(&press_out), out_ev(&temp_out));
		printk("%s\n", out_str);
#endif

		printk("- (%d) (trig_cnt: %d)\n\n", ++cnt, lis2dh_trig_cnt);
		print_samples = 1;
		k_sleep(2000);
	}
}
