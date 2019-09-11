/*
 * Copyright (c) 2018, Yannis Damigos
 * Copyright (c) 2019, Thomas Kupper
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/util.h>

static int print_samples;
static int lis2dh_trig_cnt;

#ifdef CONFIG_LIS2DH_TRIGGER
static struct sensor_value accel_x_out, accel_y_out, accel_z_out;
static void lis2dh_trigger_handler(struct device *dev,
				    struct sensor_trigger *trig)
{
	static struct sensor_value accel_x, accel_y, accel_z;
	static struct sensor_value temp;

	lis2dh_trig_cnt++;

	sensor_sample_fetch_chan(dev, SENSOR_CHAN_ACCEL_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &accel_x);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &accel_z);

	sensor_sample_fetch_chan(dev, SENSOR_CHAN_AMBIENT_TEMP);
	sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);

	if (print_samples) {
		print_samples = 0;

		accel_x_out = accel_x;
		accel_y_out = accel_y;
		accel_z_out = accel_z;

		// temp_out = temp;
	}
}

static inline float out_ev(struct sensor_value *val)
{
	return (val->val1 + (float)val->val2 / 1000000);
}

#else
static s32_t read_sensor(struct device *sensor, enum sensor_channel channel)
{
	struct sensor_value val[3];
	s32_t ret = 0;

	ret = sensor_sample_fetch(sensor);
	if (ret < 0 && ret != -EBADMSG) {
		printf("Sensor sample update error\n");
		goto end;
	}

	ret = sensor_channel_get(sensor, channel, val);
	if (ret < 0) {
		printf("Cannot read sensor channels\n");
		goto end;
	}

	printf("( x y z ) = ( %f  %f  %f )\n", sensor_value_to_double(&val[0]),
					       sensor_value_to_double(&val[1]),
					       sensor_value_to_double(&val[2]));

end:
	return ret;
}
#endif


void main(void)
{
	lis2dh_trig_cnt = 0;
	struct device *accelerometer = device_get_binding(
						DT_INST_0_ST_LIS2DH_LABEL);

	if (accelerometer == NULL) {
		printf("Could not get %s device\n",
				DT_INST_0_ST_LIS2DH_LABEL);
		return;
	}

#ifdef CONFIG_LIS2DH_TRIGGER
	int cnt = 0;
	char out_str[64];
	struct sensor_trigger trig;

	trig.type = SENSOR_TRIG_DATA_READY;
	trig.chan = SENSOR_CHAN_ACCEL_XYZ;

	if (sensor_trigger_set(accelerometer, &trig, lis2dh_trigger_handler) != 0) {
		printk("Could not set sensor type and channel\n");
		return;
	}
#endif

	while (1) {
		printf("Accelerometer data:\n");

#ifdef CONFIG_LIS2DH_TRIGGER
		sprintf(out_str, "accel (%f %f %f) m/s2", out_ev(&accel_x_out),
							out_ev(&accel_y_out),
							out_ev(&accel_z_out));
		printk("%s\n", out_str);

		printk("- (%d) (trig_cnt: %d)\n\n", ++cnt, lis2dh_trig_cnt);
#else
	sensor_sample_fetch_chan(dev, SENSOR_CHAN_ACCEL_XYZ);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &accel_x);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
	sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &accel_z);

		if (read_sensor(accelerometer, SENSOR_CHAN_ACCEL_XYZ) < 0) {
			printf("Failed to read accelerometer data\n");
		}
#endif
		print_samples = 1;
		k_sleep(2000);
	}
}
