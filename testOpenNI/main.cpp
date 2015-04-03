#include <stdio.h>
#include <time.h>
#include <iostream>
#include <string>

// 1. include OpenNI Header
#include <OpenNI.h>

// OpenCV Header
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "OniSampleUtilities.h"

// #define F_RECORDVIDEO

using namespace std;
using namespace openni;

int main()
{
	// 2. initialize OpenNI
	Status rc = OpenNI::initialize();
	if (rc != STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}

	// 3. open a device
	Device device;
	rc = device.open(ANY_DEVICE);
	if (rc != STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
		return 2;
	}

	// 4. create depth stream
	VideoStream depth;
	if (device.getSensorInfo(SENSOR_DEPTH) != NULL){
		rc = depth.create(device, SENSOR_DEPTH);
		if (rc != STATUS_OK){
			printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
			return 3;
		}
	}
	VideoStream color;
	if (device.getSensorInfo(SENSOR_COLOR) != NULL){
		rc = color.create(device, SENSOR_COLOR);
		if (rc != STATUS_OK){
			printf("Couldn't create color stream\n%s\n", OpenNI::getExtendedError());
			return 4;
		}
	}

	// 5. create OpenCV Window
	cv::namedWindow("Depth Image", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("Color Image", CV_WINDOW_AUTOSIZE);

	// 6. start
	rc = depth.start();
	if (rc != STATUS_OK)
	{
		printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
		return 5;
	}
	rc = color.start();
	if (rc != STATUS_OK){
		printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
		return 6;
	}
	
	VideoFrameRef colorframe;
	VideoFrameRef depthframe;
	int iMaxDepth = depth.getMaxPixelValue();
	int iColorFps = color.getVideoMode().getFps();
	cv::Size iColorFrameSize = cv::Size(color.getVideoMode().getResolutionX(), color.getVideoMode().getResolutionY());

	cv::Mat colorimageRGB;
	cv::Mat colorimageBGR;
	cv::Mat depthimage;
	cv::Mat depthimageScaled;

#ifdef F_RECORDVIDEO

	cv::VideoWriter outputvideo_color;
	cv::FileStorage outputfile_depth;
	
	time_t timenow = time(0);
	tm ltime;
	localtime_s(&ltime, &timenow);
	int tyear = 1900 + ltime.tm_year;
	int tmouth = 1 + ltime.tm_mon;
	int tday = ltime.tm_mday;
	int thour = ltime.tm_hour;
	int tmin = ltime.tm_min;
	int tsecond = ltime.tm_sec;

	string filename_rgb = "RGB/rgb_" + to_string(tyear) + "_" + to_string(tmouth) + "_" + to_string(tday)
		+ "_" + to_string(thour) + "_" + to_string(tmin) + "_" + to_string(tsecond) + ".avi";
	string filename_d = "D/d_" + to_string(tyear) + "_" + to_string(tmouth) + "_" + to_string(tday)
		+ "_" + to_string(thour) + "_" + to_string(tmin) + "_" + to_string(tsecond) + ".yml";

	outputvideo_color.open(filename_rgb, CV_FOURCC('I', '4', '2', '0'), iColorFps, iColorFrameSize, true);
	if (!outputvideo_color.isOpened()){
		cout << "Could not open the output color video for write: " << endl;
		return 7;
	}
	outputfile_depth.open(filename_d, cv::FileStorage::WRITE);
	if (!outputfile_depth.isOpened()){
		cout << "Could not open the output depth file for write: " << endl;
		return 8;
	}

#endif // F_RECORDVIDEO


	// 7. main loop, continue read
	while (!wasKeyboardHit())
	{
		// 8. check is color stream is available
		if (color.isValid()){
			if (color.readFrame(&colorframe) == STATUS_OK){
				colorimageRGB = { colorframe.getHeight(), colorframe.getWidth(), CV_8UC3, (void*)colorframe.getData() };
				cv::cvtColor(colorimageRGB, colorimageBGR, CV_RGB2BGR);
			}
		}

		// 9. check is depth stream is available
		if (depth.isValid()){
			if (depth.readFrame(&depthframe) == STATUS_OK){
				depthimage = { depthframe.getHeight(), depthframe.getWidth(), CV_16UC1, (void*)depthframe.getData() };
				depthimage.convertTo(depthimageScaled, CV_8U, 255.0 / iMaxDepth);
			}
		}

		cv::imshow("Color Image", colorimageBGR);
		cv::imshow("Depth Image", depthimageScaled);

#ifdef F_RECORDVIDEO
		
		outputvideo_color << colorimageBGR;
		outputfile_depth << "Mat" << depthimage;

#endif // F_RECORDVIDEO

		cv::waitKey(10);
	}

	color.stop();
	depth.stop();

	color.destroy();
	depth.destroy();

	device.close();
	OpenNI::shutdown();

	return 0;
}
