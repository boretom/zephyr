.. _lis2dh:

LIS2DH & LIS3DH: Accelerometer data Monitor
#######################################################

Overview
********
This sample application periodically reads accelerometer data from the
LIS2DH and LIS3DH module's sensors, and displays the sensor data on the
console.

Requirements
************

This sample uses the LIS2DH or LIS2DH, ST MEMS system-in-package featuring
a 3D digital linear acceleration sensor, controlled using the I2C interface.

References
**********

For more information about the LIS2DH & LIS3DH eCompass module, see
https://www.st.com/en/mems-and-sensors/lis2dh.html
https://www.st.com/en/mems-and-sensors/lis3dh.html

Building and Running
********************

This project outputs sensor data to the console. It requires a LIS2DH or
LIS3DH system-in-package, which is present on the :ref:`96b_wistrio board`

.. zephyr-app-commands::
   :zephyr-app: samples/sensors/lis2dh
   :board: 96b_wistrio
   :goals: build
   :compact:

Sample Output
=============

.. code-block:: console

   Accelerometer data:
   ( x y z ) = ( -0.078127  -0.347666  1.105502 )
   Accelerometer data:
   ( x y z ) = ( 0.074221  -0.304696  0.972685 )

   <repeats endlessly every 2 seconds>
