#!/bin/sh

# the default directory the script runs in is /dev, so change to the drive
cd /media/sda1/

# create a file
touch foobar

# change the RX_LO to 2.4GHz
iio_attr -a -c  ad9361-phy RX_LO frequency 2400000000
