#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <cstdio>
#include <iostream>
#include <cmath>
#include <raspicam/raspicam.h>
#include <raspicam/raspicam.h>
#include <raspicam/raspicam_cv.h>

// Static variables for actions
bool invert = false;
bool normal = false;
bool zoom_change = false;

// Static variables for debouncing
bool last_button_up = true;
bool last_button_down = true;
bool last_button_invert = true;

int main()
{
	float width = 1280.0f;
	float height = 960.0f;
	float zoom = 1.0f;

	int x = floor((((width / zoom) * (zoom / 2.0)) - ((width / zoom) / 2.0)));
	int y = floor((((height / zoom) * (zoom / 2.0))- ((height / zoom) / 2.0)));
	int w = floor((width / zoom));
	int h = floor((height / zoom));

    raspicam::RaspiCam_Cv camera;
    cv::Mat image;
	camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 );
	camera.set(CV_CAP_PROP_WHITE_BALANCE_RED_V, 0);

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
		if (invert) 
		{
			camera.set(CV_CAP_PROP_MODE, raspicam::RASPICAM_IMAGE_EFFECT_NEGATIVE);
			invert = false;
		}
		if (normal) 
		{
			camera.set(CV_CAP_PROP_MODE, raspicam::RASPICAM_IMAGE_EFFECT_NONE);
			normal = false;
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
		debounce();
		auto key = cv::waitKey(1);
    }
    

    return 0;
}


