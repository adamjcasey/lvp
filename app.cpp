#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <cstdio>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
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
#define PIN_UP 			"/sys/class/gpio/gpio4/value"
#define PIN_DOWN 		"/sys/class/gpio/gpio23/value"
#define PIN_INVERT 		"/sys/class/gpio/gpio24/value"

uint8_t getval_gpio(const char * pin)
{
	std::string val;
	uint8_t returnval = 0;
    std::ifstream getvalgpio(pin);
    getvalgpio >> val ;  //read gpio value

    if (val != "0")
        returnval = 1;

    getvalgpio.close(); //close the value file
    return returnval;
}

void debounce()
{
	uint8_t button_up = getval_gpio(PIN_UP);
	if (button_up != last_button_up)
	{
		last_button_up = button_up;
		if (button_up == 0)
		{
			event_button_up = true;
		}
	}
	uint8_t button_down = getval_gpio(PIN_DOWN);
	if (button_down != last_button_down)
	{
		last_button_down = button_down;
		if (button_down == 0)
		{
			event_button_down = true;
		}
	}
	uint8_t button_invert = getval_gpio(PIN_INVERT);
	if (button_invert != last_button_invert)
	{
		last_button_invert = button_invert;
		if (button_invert == 0)
		{
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

int main()
{
    raspicam::RaspiCam_Cv camera;
    cv::Mat image;
	camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
	camera.set(CV_CAP_PROP_WHITE_BALANCE_RED_V, 0);
	camera.set(CV_CAP_PROP_FRAME_WIDTH, 1440);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

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
			std::cout << "Invert ON" << std::endl;
			camera.set(CV_CAP_PROP_MODE, raspicam::RASPICAM_IMAGE_EFFECT_NEGATIVE);
			invert_change = false;
		}
		if (normal_change) 
		{
			std::cout << "Invert OFF" << std::endl;
			camera.set(CV_CAP_PROP_MODE, raspicam::RASPICAM_IMAGE_EFFECT_NONE);
			normal_change = false;
		}
		if (zoom_change)
		{
			std::cout << "Zoom: " << zoom << std::endl;

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
		handle_events();
    }
    

    return 0;
}
