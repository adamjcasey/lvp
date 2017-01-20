#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <cstdio>
#include <iostream>
#include <chrono>
#include <cmath>


class BenchmarkCase
{
    public:
        BenchmarkCase(int w, int h, int z) : width(w), height(h), zoom(z) {}

        static constexpr int NUM_FRAMES_PER_CASE = 5;

        std::chrono::high_resolution_clock::time_point start_time[NUM_FRAMES_PER_CASE];
        std::chrono::high_resolution_clock::time_point capture_time[NUM_FRAMES_PER_CASE];
        std::chrono::high_resolution_clock::time_point zoom_time[NUM_FRAMES_PER_CASE];
        std::chrono::high_resolution_clock::time_point display_time[NUM_FRAMES_PER_CASE];
        std::chrono::high_resolution_clock::time_point contrast_time[NUM_FRAMES_PER_CASE];
        std::chrono::high_resolution_clock::time_point cb_time[NUM_FRAMES_PER_CASE];



        float width;
        float height;
        float zoom;
};

BenchmarkCase testcases[] = 
{
    BenchmarkCase(640.0f, 480.0f, 1.0f),
    BenchmarkCase(640.0f, 480.0f, 2.0f),
    BenchmarkCase(640.0f, 480.0f, 3.0f),
    BenchmarkCase(640.0f, 480.0f, 4.0f),
    BenchmarkCase(640.0f, 480.0f, 5.0f),
    BenchmarkCase(640.0f, 480.0f, 6.0f),
    BenchmarkCase(640.0f, 480.0f, 7.0f),
    BenchmarkCase(640.0f, 480.0f, 8.0f),
    BenchmarkCase(1280.0f, 720.0f, 1.0f),
    BenchmarkCase(1280.0f, 720.0f, 2.0f),
    BenchmarkCase(1280.0f, 720.0f, 3.0f),
    BenchmarkCase(1280.0f, 720.0f, 4.0f),
    BenchmarkCase(1280.0f, 720.0f, 5.0f),
    BenchmarkCase(1280.0f, 720.0f, 6.0f),
    BenchmarkCase(1280.0f, 720.0f, 7.0f),
    BenchmarkCase(1280.0f, 720.0f, 8.0f)
    // BenchmarkCase(1920.0f, 1080.0f, 1.0f),
    // BenchmarkCase(1920.0f, 1080.0f, 2.0f),
    // BenchmarkCase(1920.0f, 1080.0f, 3.0f),
    // BenchmarkCase(1920.0f, 1080.0f, 4.0f),
    // BenchmarkCase(1920.0f, 1080.0f, 5.0f),
    // BenchmarkCase(1920.0f, 1080.0f, 6.0f),
    // BenchmarkCase(1920.0f, 1080.0f, 7.0f),
    // BenchmarkCase(1920.0f, 1080.0f, 8.0f)
};

int num_test_cases = sizeof(testcases)/sizeof(BenchmarkCase);

int main()
{
    std::cout << "Built with OpenCV " << CV_VERSION << std::endl;
    cv::Mat image;
    cv::VideoCapture capture;
    capture.open(0);
    if (capture.isOpened())
    {
        std::cout << "Capture is opened" << std::endl;

        // Cycle through the test cases
        for (int i=0; i<num_test_cases; i++)
        {
            // Set the height and the width
            capture.set(cv::CAP_PROP_FRAME_WIDTH, testcases[i].width);
            capture.set(cv::CAP_PROP_FRAME_HEIGHT, testcases[i].height);


            // Loop through the captures            
            for (int j=0; j<BenchmarkCase::NUM_FRAMES_PER_CASE; j++)
            {
                testcases[i].start_time[j] = std::chrono::high_resolution_clock::now();

                //-- CAPTURE --------------------------------------------------------------------------
                capture >> image;
                testcases[i].capture_time[j] = std::chrono::high_resolution_clock::now();

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

                testcases[i].zoom_time[j] = std::chrono::high_resolution_clock::now();

                //-- CONTRAST --------------------------------------------------------------------------
                float alpha = 2.0f;
                int beta = 0;
                cv::Mat tmp2 = cv::Mat::zeros(tmp.size(), tmp.type());
                tmp.convertTo(tmp2, -1, alpha, beta);

                testcases[i].contrast_time[j] = std::chrono::high_resolution_clock::now();

                //-- COLOR BALANCE ---------------------------------------------------------------------
                float percent = 1.0;
                float half_percent = percent / 200.0f;

                std::vector<cv::Mat> tmpsplit; 
                split(tmp2, tmpsplit);
                for (int i=0; i<3; i++) 
                {
                    // Find the low and high precentile values (based on the input percentile)
                    cv::Mat flat; 
                    tmpsplit[i].reshape(1, 1).copyTo(flat);
                    cv::sort(flat, flat, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
                    int lowval = flat.at<uchar>(cvFloor(((float)flat.cols) * half_percent));
                    int highval = flat.at<uchar>(cvCeil(((float)flat.cols) * (1.0 - half_percent)));
        
                    //saturate below the low percentile and above the high percentile
                    tmpsplit[i].setTo(lowval,tmpsplit[i] < lowval);
                    tmpsplit[i].setTo(highval,tmpsplit[i] > highval);
                    
                    //scale the channel
                    cv::normalize(tmpsplit[i], tmpsplit[i], 0, 255, cv::NORM_MINMAX);
                }
                merge(tmpsplit, tmp2);

                testcases[i].cb_time[j] = std::chrono::high_resolution_clock::now();

                //-- INVERT -----------------------------------------------------------------------------


                //-- DISPLAY -----------------------------------------------------------------------------
                cv::imshow("Sample", tmp2);
                testcases[i].display_time[j] = std::chrono::high_resolution_clock::now();



                cv::waitKey(1);

            }
        }
    }

    // Display the test cases
    for (int i=0; i<num_test_cases; i++)
    {    
        printf("------------\r\n------------\r\nTest Case %d: Width: %.0f Height: %.0f Zoom: %.0f\r\n", i, testcases[i].width, testcases[i].height, testcases[i].zoom);

        for (int j=0; j<BenchmarkCase::NUM_FRAMES_PER_CASE; j++)
        {
            auto cap = std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].capture_time[j].time_since_epoch()).count() - 
                        std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].start_time[j].time_since_epoch()).count();

            auto zoom = std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].zoom_time[j].time_since_epoch()).count() - 
                        std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].capture_time[j].time_since_epoch()).count();

            auto contrast = std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].contrast_time[j].time_since_epoch()).count() - 
                        std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].zoom_time[j].time_since_epoch()).count();

            auto cb = std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].cb_time[j].time_since_epoch()).count() - 
                        std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].contrast_time[j].time_since_epoch()).count();

            auto display = std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].display_time[j].time_since_epoch()).count() - 
                        std::chrono::duration_cast<std::chrono::milliseconds>(testcases[i].cb_time[j].time_since_epoch()).count();

            printf("------------\r\nTiming Case %d\r\n", j+1);
            printf("Capture Time: %ld\r\n", cap);
            printf("Zoom Time: %ld\r\n", zoom);
            printf("Contrast Time: %ld\r\n", contrast);
            printf("Color Balance Time: %ld\r\n", cb);
            printf("Display Time: %ld\r\n", display);
        }

    }
    return 0;
}
