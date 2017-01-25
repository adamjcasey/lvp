#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <cstdio>
#include <iostream>
#include <cmath>
#include <bcm2835.h>
#include <raspicam/raspicam.h>
#include <raspicam/raspicam_cv.h>

// Static variables for actions
bool invert_change = false;
bool normal_change = false;
bool zoom_change = false;

// Events
bool event_button_up = false;
bool event_button_down = false;
bool event_button_invert = false;

// Static variables for debouncing
uint8_t last_button_up = 1;
uint8_t last_button_down = 1;
uint8_t last_button_invert = 1;

// Capture and display variables
float width = 1280.0f;
float height = 960.0f;
float zoom = 1.0f;
bool invert = false;

int x = floor((((width / zoom) * (zoom / 2.0)) - ((width / zoom) / 2.0)));
int y = floor((((height / zoom) * (zoom / 2.0))- ((height / zoom) / 2.0)));
int w = floor((width / zoom));
int h = floor((height / zoom));

// efine RPi GPIO pins
#define PIN_UP 			RPI_GPIO_P1_4
#define PIN_DOWN 		RPI_GPIO_P1_18
#define PIN_CONTRAST 	RPI_GPIO_P1_24

void debounce()
{
	uint8_t button_up = bcm2835_gpio_lev(PIN_UP);
	if (button_up != last_button_up)
	{
		button_up = last_button_up;
		if (button_up == 0)
		{
			std::cout << "Button Up Pressed" << std::endl;
			event_button_up = true;
		}
	}
	uint8_t button_down = bcm2835_gpio_lev(PIN_DOWN);
	if (button_down != last_button_down)
	{
		button_down = last_button_down;
		if (button_down == 0)
		{
			std::cout << "Button Down Pressed" << std::endl;
			event_button_down = true;
		}
	}
	uint8_t button_invert = bcm2835_gpio_lev(PIN_INVERT);
	if (button_invert != last_button_invert)
	{
		button_invert = last_button_invert;
		if (button_invert == 0)
		{
			std::cout << "Button Invert Pressed" << std::endl;
			event_button_invert = true;
		}
	}
}

void handle_events()
{
	if (event_button_up)
	{
		event_button_up = false;
		if ((int)zoom < 8)
		{
			zoom = (float)((int)zoom + 1);
			zoom_change = true;
		}
	}
	if (event_button_down)
	{
		event_button_down = false;
		if ((int)zoom > 1)
		{
			zoom = (float)((int)zoom - 1);
			zoom_change = true;
		}
	}
	if (event_button_invert)
	{
		event_button_invert = false;
		if (invert)
		{
			invert = false;
			normal_change = true;
		}
		else
		{
			invert = true;
			invert_change = true;
		}
	}
}

bool initializeGpio()
{
	if (!bcm2835_init())
		return false;

	// Set pins to be inputs
    bcm2835_gpio_fsel(PIN_UP, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_DOWN, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_CONTRAST, BCM2835_GPIO_FSEL_INPT);

    // With a pullup
    bcm2835_gpio_set_pud(PIN_UP, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_set_pud(PIN_DOWN, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_set_pud(PIN_CONTRAST, BCM2835_GPIO_PUD_UP);

    return true;
}

int main()
{
    raspicam::RaspiCam_Cv camera;
    cv::Mat image;
	camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 );
	camera.set(CV_CAP_PROP_WHITE_BALANCE_RED_V, 0);

	if (!initializeGpio())
	{
		std::cout << "Error initializing the gpio" << std::endl;
		return -1;
	}

	if (!camera.open()) 
	{
		std::cout << "Error opening the camera" << std::endl;
		return -1;
	}

	std::cout << "Camera is opened" << std::endl;	
    cvNamedWindow("Name", CV_WINDOW_NORMAL);
    cvSetWindowProperty("Name", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	while (1)
	{
		//-- INVERT --------------------------------------
		if (invert_change) 
		{
			camera.set(CV_CAP_PROP_MODE, raspicam::RASPICAM_IMAGE_EFFECT_NEGATIVE);
			invert_change = false;
		}
		if (normal_change) 
		{
			camera.set(CV_CAP_PROP_MODE, raspicam::RASPICAM_IMAGE_EFFECT_NONE);
			normal_change = false;
		}
		if (zoom_change)
		{
			x = floor((((width / zoom) * (zoom / 2.0)) - ((width / zoom) / 2.0)));
			y = floor((((height / zoom) * (zoom / 2.0))- ((height / zoom) / 2.0)));
			w = floor((width / zoom));
			h = floor((height / zoom));
			zoom_change = false;
		}
			
		
		//-- CAPTURE --------------------------------------------------------------------------
		camera.grab();
		camera.retrieve(image);
	
		//-- ZOOM -----------------------------------------------------------------------------
		cv::Rect new_size(x, y, w, h);
		cv::Mat tmp = image(new_size);
		cv::resize(tmp, tmp, cv::Size(width, height), 0, 0, CV_INTER_LINEAR);

		//-- DISPLAY -----------------------------------------------------------------------------
		cv::imshow("Name", tmp);

		//-- USER INPUT -------------------------------------------------------
		auto key = cv::waitKey(1);
		if (key == 'q')
			break;
		else if (key == 'a')
			event_button_down = true;
		else if (key == 'z')
			event_button_up = true;
		else if (key == 'x')
			event_button_invert = true;
		debounce();
		handle_events()
    }
    

    return 0;
}
