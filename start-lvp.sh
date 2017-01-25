#!/bin/bash

# Setup GPIO
echo "4" > /sys/class/gpio/export
echo "23" > /sys/class/gpio/export
echo "24" > /sys/class/gpio/export

raspi-gpio set 4 pu # up
raspi-gpio set 23 pu # up
raspi-gpio set 24 pu # up
