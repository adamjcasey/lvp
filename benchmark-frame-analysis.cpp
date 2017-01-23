#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <cstdio>
#include <iostream>
#include <chrono>
#include <cmath>
#include <raspicam/raspicam_cv.h>

class BenchmarkCase
{
    public:
        BenchmarkCase(int w, int h, int z) : width(w), height(h), zoom(z) {}

        static constexpr int NUM_FRAMES_PER_CASE = 15;

        std::chrono::high_resolution_clock::time_point start_time[NUM_FRAMES_PER_CASE];
        int capture_time[NUM_FRAMES_PER_CASE];
        int zoom_time[NUM_FRAMES_PER_CASE];
        int display_time[NUM_FRAMES_PER_CASE];
        int contrast_time[NUM_FRAMES_PER_CASE];
        std::chrono::high_resolution_clock::time_point cb_time[NUM_FRAMES_PER_CASE];



        float width;
        float height;
        float zoom;
};

BenchmarkCase testcases[] = 
{
    BenchmarkCase(1280.0f, 960.0f, 1.0f),
    BenchmarkCase(1280.0f, 960.0f, 2.0f),
    BenchmarkCase(1280.0f, 960.0f, 3.0f),
    BenchmarkCase(1280.0f, 960.0f, 4.0f),
    BenchmarkCase(1280.0f, 960.0f, 5.0f),
    BenchmarkCase(1280.0f, 960.0f, 6.0f),
    BenchmarkCase(1280.0f, 960.0f, 7.0f),
    BenchmarkCase(1280.0f, 960.0f, 8.0f)
};

int num_test_cases = sizeof(testcases)/sizeof(BenchmarkCase);

int main()
{
	bool contrast = false;
 	bool sharpen = false;
 	bool zoom = true;
    std::cout << "Built with OpenCV " << CV_VERSION << std::endl;
    raspicam::RaspiCam_Cv camera;
    cv::Mat image;
	camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 );
	if (!camera.open()) 
	{
		std::cout << "Error opening the camera" << std::endl;
		return -1;
	}
	std::cout << "Camera is opened" << std::endl;
    cvNamedWindow("Name", CV_WINDOW_NORMAL);
    cvSetWindowProperty("Name", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	// Cycle through the test cases
	for (int i=0; i<num_test_cases; i++)
	{
		// Loop through the captures            
		for (int j=0; j<BenchmarkCase::NUM_FRAMES_PER_CASE; j++)
		{
			auto start_time = std::chrono::high_resolution_clock::now();
			
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
			cv::resize(tmp, image, cv::Size(testcases[i].width, testcases[i].height), 0, 0, CV_INTER_LINEAR);

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
			
			//-- SHARPEN ---------------------------------------------------------------------------
			if (sharpen) 
			{
				cv::Matx33d sharpen_kernel( -0.1, -0.1, -0.1,
									 -0.1,  0.9, -0.1,
									 -0.1, -0.1, -0.1);
				cv::filter2D(image, image, -1, sharpen_kernel, cv::Point2i(-1,-1), 0.0, cv::BORDER_REPLICATE);
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
			auto display_time = std::chrono::high_resolution_clock::now();
			testcases[i].display_time[j] = std::chrono::duration_cast<std::chrono::milliseconds>(display_time.time_since_epoch()).count() -
                        std::chrono::duration_cast<std::chrono::milliseconds>(contrast_time.time_since_epoch()).count();



			cv::waitKey(1);
        }
    }
    
    // Display the test cases
    for (int i=0; i<num_test_cases; i++)
    {    
        printf("------------\r\n------------\r\nTest Case %d: Width: %.0f Height: %.0f Zoom: %.0f\r\n", i, testcases[i].width, testcases[i].height, testcases[i].zoom);

		float total_zoom = 0;
		float total_capture = 0;
		float total_display = 0;
		
        for (int j=0; j<BenchmarkCase::NUM_FRAMES_PER_CASE; j++)
        {
			total_capture += testcases[i].capture_time[j];
 			total_zoom += testcases[i].zoom_time[j];
 			total_display += testcases[i].display_time[j];
        }
		printf("Capture Time: %f\r\n", total_capture / static_cast<float>(BenchmarkCase::NUM_FRAMES_PER_CASE));
		printf("Zoom Time: %f\r\n", total_zoom / static_cast<float>(BenchmarkCase::NUM_FRAMES_PER_CASE));
		printf("Display Time: %f\r\n", total_display / static_cast<float>(BenchmarkCase::NUM_FRAMES_PER_CASE));
    }
    return 0;
}
