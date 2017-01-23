#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <cstdio>
#include <iostream>
#include <cmath>
#include <raspicam/raspicam_cv.h>


int main()
{
	bool contrast = false;
    raspicam::RaspiCam_Cv camera;
    cv::Mat image;
	camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 );
	if (!camera.open()) 
	{
		std::cout << "Error opening the camera" << std::endl;
		return -1;
	}
    cvNamedWindow("Name", CV_WINDOW_NORMAL);
    cvSetWindowProperty("Name", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	while (1)
	{
		
		//-- CAPTURE --------------------------------------------------------------------------
		camera.grab();
		camera.retrieve(image);
		
		auto capture_time = std::chrono::high_resolution_clock::now();
		
		testcases[i].capture_time[j] = std::chrono::duration_cast<std::chrono::milliseconds>(capture_time.time_since_epoch()).count() -
					std::chrono::duration_cast<std::chrono::milliseconds>(start_time.time_since_epoch()).count();
	
		

		//-- ZOOM -----------------------------------------------------------------------------
		// Calculate the region of interest of the zoom
		int x = floor((((testcases[i].width / testcases[i].zoom) * (testcases[i].zoom / 2.0)) - ((testcases[i].width / testcases[i].zoom) / 2.0)));
		int y = floor((((testcases[i].height / testcases[i].zoom) * (testcases[i].zoom / 2.0))- ((testcases[i].height / testcases[i].zoom) / 2.0)));
		int width = floor((testcases[i].width / testcases[i].zoom));
		int height = floor((testcases[i].height / testcases[i].zoom));

		// Perform the zoom
		cv::Rect new_size(x, y, width, height);
		cv::Mat tmp = image(new_size);
		cv::resize(tmp, tmp, cv::Size(testcases[i].width, testcases[i].height), 0, 0, CV_INTER_LINEAR);

		auto zoom_time = std::chrono::high_resolution_clock::now();
		testcases[i].zoom_time[j] = std::chrono::duration_cast<std::chrono::milliseconds>(zoom_time.time_since_epoch()).count() -
					std::chrono::duration_cast<std::chrono::milliseconds>(capture_time.time_since_epoch()).count();

		

		//-- CONTRAST --------------------------------------------------------------------------
		if (contrast) 
		{
			float alpha = 2.0f;
			int beta = 20;
			cv::Mat tmp = cv::Mat::zeros(tmp.size(), tmp.type());
			tmp.convertTo(tmp, -1, alpha, beta);
		}

		auto contrast_time = std::chrono::high_resolution_clock::now();
		testcases[i].contrast_time[j] = std::chrono::duration_cast<std::chrono::milliseconds>(contrast_time.time_since_epoch()).count() -
					std::chrono::duration_cast<std::chrono::milliseconds>(zoom_time.time_since_epoch()).count();
					
		 
		//-- COLOR BALANCE ---------------------------------------------------------------------
		float percent = 1.0;
		float half_percent = percent / 200.0f;

		//std::vector<cv::Mat> tmpsplit; 
		//split(tmp, tmpsplit);
		//for (int i=0; i<3; i++) 
		//{
			// Find the low and high precentile values (based on the input percentile)
			//cv::Mat flat; 
			//tmpsplit[i].reshape(1, 1).copyTo(flat);
			//cv::sort(flat, flat, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
			//int lowval = flat.at<uchar>(cvFloor(((float)flat.cols) * half_percent));
			//int highval = flat.at<uchar>(cvCeil(((float)flat.cols) * (1.0 - half_percent)));

			//saturate below the low percentile and above the high percentile
			//tmpsplit[i].setTo(lowval,tmpsplit[i] < lowval);
			//tmpsplit[i].setTo(highval,tmpsplit[i] > highval);
			
			//scale the channel
			//cv::normalize(tmpsplit[i], tmpsplit[i], 0, 255, cv::NORM_MINMAX);
		//}
		//merge(tmpsplit, tmp);

		testcases[i].cb_time[j] = std::chrono::high_resolution_clock::now();

		//-- INVERT -----------------------------------------------------------------------------.time_since_epoch()).count()


		//-- DISPLAY -----------------------------------------------------------------------------
		cv::imshow("Name", tmp);
		testcases[i].display_time[j] = std::chrono::high_resolution_clock::now();



		auto key = cv::waitKey(1);
    }
    

    return 0;
}
