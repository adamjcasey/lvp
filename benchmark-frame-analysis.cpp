#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <cstdio>
#include <iostream>
#include <chrono>
#include <string>
#include <cmath>
#include <raspicam/raspicam.h>
#include <raspicam/raspicam_cv.h>

class BenchmarkCase
{
    public:
        BenchmarkCase(int w, int h, int z, bool inv = false) : width(w), height(h), zoom(z), invert(inv) {}

        static constexpr int NUM_FRAMES_PER_CASE = 15;

        int capture_time[NUM_FRAMES_PER_CASE];
        int zoom_time[NUM_FRAMES_PER_CASE];
        int display_time[NUM_FRAMES_PER_CASE];
        int contrast_time[NUM_FRAMES_PER_CASE];

        float width;
        float height;
        float zoom;
        bool invert;
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
    BenchmarkCase(1280.0f, 960.0f, 8.0f),
    BenchmarkCase(1280.0f, 960.0f, 1.0f, true),
    BenchmarkCase(1280.0f, 960.0f, 2.0f, true),
    BenchmarkCase(1280.0f, 960.0f, 3.0f, true),
    BenchmarkCase(1280.0f, 960.0f, 4.0f, true),
    BenchmarkCase(1280.0f, 960.0f, 5.0f, true),
    BenchmarkCase(1280.0f, 960.0f, 6.0f, true),
    BenchmarkCase(1280.0f, 960.0f, 7.0f, true),
    BenchmarkCase(1280.0f, 960.0f, 8.0f, true)
};

int num_test_cases = sizeof(testcases)/sizeof(BenchmarkCase);

int findParam(std::string param, int argc, char **argv) 
{
    int idx=-1;
    for (int i=0; i<argc && idx==-1; i++)
        if (std::string(argv[i]) == param) 
			idx = i;
    return idx;
}

int main(int argc,char **argv)
{
 	bool sharpen = false;

    std::cout << "Built with OpenCV " << CV_VERSION << std::endl;
    raspicam::RaspiCam_Cv camera;
    raspicam::RaspiCam camerabasic;
    cv::Mat image;
	camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
	camera.set(CV_CAP_PROP_WHITE_BALANCE_RED_V, 0);
	
    if (findParam ("-s", argc, argv) != -1)
		sharpen = true;
		
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
		if (testcases[i].invert)
			camera.set(CV_CAP_PROP_MODE, raspicam::RASPICAM_IMAGE_EFFECT_NEGATIVE);
			
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


			// Perform the zoom
			int x = floor((((testcases[i].width / testcases[i].zoom) * (testcases[i].zoom / 2.0)) - ((testcases[i].width / testcases[i].zoom) / 2.0)));
			int y = floor((((testcases[i].height / testcases[i].zoom) * (testcases[i].zoom / 2.0))- ((testcases[i].height / testcases[i].zoom) / 2.0)));
			int width = floor((testcases[i].width / testcases[i].zoom));
			int height = floor((testcases[i].height / testcases[i].zoom));
			cv::Rect new_size(x, y, width, height);
			cv::Mat tmp = image(new_size);
			cv::resize(tmp, tmp, cv::Size(testcases[i].width, testcases[i].height), 0, 0, CV_INTER_LINEAR);

			auto zoom_time = std::chrono::high_resolution_clock::now();
			testcases[i].zoom_time[j] = std::chrono::duration_cast<std::chrono::milliseconds>(zoom_time.time_since_epoch()).count() -
                        std::chrono::duration_cast<std::chrono::milliseconds>(capture_time.time_since_epoch()).count();

			

			//-- CONTRAST --------------------------------------------------------------------------
			// handled by Pi Camera
			
			//-- SHARPEN ---------------------------------------------------------------------------
			if (sharpen) 
			{
				cv::Matx33d sharpen_kernel( -0.1, -0.1, -0.1,
									 -0.1,  0.9, -0.1,
									 -0.1, -0.1, -0.1);
				cv::filter2D(image, image, -1, sharpen_kernel, cv::Point2i(-1,-1), 0.0, cv::BORDER_REPLICATE);
			}
			auto contrast_time = std::chrono::high_resolution_clock::now();
			if (sharpen)
				testcases[i].contrast_time[j] = std::chrono::duration_cast<std::chrono::milliseconds>(contrast_time.time_since_epoch()).count() -
                        std::chrono::duration_cast<std::chrono::milliseconds>(zoom_time.time_since_epoch()).count();
            else
                testcases[i].contrast_time[j] = 0;
             
			//-- COLOR BALANCE ---------------------------------------------------------------------
			// handled by Pi Camera

			//-- INVERT -----------------------------------------------------------------------------
			// handled by Pi Camera


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
		float total_sharpen = 0;
		float total_display = 0;
		
        for (int j=0; j<BenchmarkCase::NUM_FRAMES_PER_CASE; j++)
        {
			total_capture += testcases[i].capture_time[j];
			total_sharpen += testcases[i].contrast_time[j];
 			total_zoom += testcases[i].zoom_time[j];
 			total_display += testcases[i].display_time[j];
        }
        
        float average_capture = total_capture / static_cast<float>(BenchmarkCase::NUM_FRAMES_PER_CASE);
        float average_zoom = total_zoom / static_cast<float>(BenchmarkCase::NUM_FRAMES_PER_CASE);
        float average_sharpen = total_sharpen / static_cast<float>(BenchmarkCase::NUM_FRAMES_PER_CASE);
        float average_display = total_display / static_cast<float>(BenchmarkCase::NUM_FRAMES_PER_CASE);
        float average_frame = (total_capture + total_zoom + total_display + total_sharpen) / static_cast<float>(BenchmarkCase::NUM_FRAMES_PER_CASE);
        float average_fps = 1000 / average_frame;
        
        
		printf("Average Capture Time: %f\r\n", average_capture);
		printf("Average Zoom Time: %f\r\n", average_zoom);
		printf("Average Sharpen Time: %f\r\n", average_sharpen);
		printf("Average Display Time: %f\r\n", average_display);
		printf("Average Frame Time: %f\r\n", average_frame);
		printf("%f FPS\r\n", average_fps);
    }
    return 0;
}
