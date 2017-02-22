# LVP Software Overview

## Platform

- The stable configuration runs on a Raspberry Pi 3
- The operating system is Raspbian Jessie

## Additional Libraries Required

- OpenCV 3.2.0 : [https://github.com/opencv/opencv/releases]
- RaspPiCam 0.1.5 : [https://www.uco.es/investiga/grupos/ava/node/40]

For ease of build and development, the gcc toolsuite is required.  The easiest way to make sure everything needed to compile and build the C++ is to follow the installation tutorial : [http://docs.opencv.org/trunk/d7/d9f/tutorial_linux_install.html]


This includes the following essential packages:
- GCC 4.4.x or later
- CMake 2.8.7 or higher
- Git
- GTK+2.x or higher, including headers (libgtk2.0-dev)
- pkg-config
- ffmpeg or libav development packages: libavcodec-dev, libavformat-dev, libswscale-dev
- libtbb2 libtbb-dev
- libdc1394 2.x
- libjpeg-dev, libpng-dev, libtiff-dev, libjasper-dev, libdc1394-22-dev


## GPIO

- All three GPIO pins are initialized as inputs, with the internal pullup enabled.  Thus, the pushbutton should be connected between the indicated GPIO and ground.
- GPIO 3 (pin xx) is ZOOM IN
- GPIO 21 (pin xx) is ZOOM OUT
- GPIO 22 (pin xx) is TOGGLE INVERT
- Raspian Jessie allows non sudo access to the GPIO pins by use of the system filesystem (sysfs).  The pins can be set for input or ouptut, and read and written to, using file IO.  However, the pullups cannot be set this way.  Therefore, a helper script is used to setup the GPIO and launch the `lvp-app` application.

## Location of Source

- The source for the lvp-app applications (including the test apps) are in `/home/pi/lvp`

## Building the Application

- `cd /home/pi/lvp/build`
- `cmake ..`
- `make`

## Start On Boot

- In order to start the application on boot, the following modification is made to 

## Stop Blanking of Screen

- Without keyboard or mouse activity, the screen will blank after 5 minutes.  The following modifications need to be made.

## Overview of Application

Here is an outline of the application



