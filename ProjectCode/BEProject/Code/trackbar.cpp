/*#include <opencv\cv.h>
#include <opencv\cxcore.h>


int lowerH = 0;
int lowerS = 0;
int lowerV = 0;

int upperH = 180;
int upperS = 256;
int upperV = 256;


#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <iostream>
using namespace cv;
using namespace std;

IplImage* imgTracking;



//This function threshold the HSV image and create a binary image
IplImage* GetThresholdedImage(IplImage* imgHSV)
{
	IplImage* imgThresh = cvCreateImage(cvGetSize(imgHSV), IPL_DEPTH_8U, 1);

	cvInRangeS(imgHSV, cvScalar(lowerH, lowerS, lowerV), cvScalar(upperH, upperS, upperV), imgThresh);

	cvSmooth(imgHSV, imgHSV, CV_MEDIAN, 7, 7);

	return imgThresh;
}

void setwindowSettings()
{
	cvNamedWindow("Video");
	cvNamedWindow("Ball");

	cvCreateTrackbar("LowerH", "Ball", &lowerH, 180, NULL);
	cvCreateTrackbar("UpperH", "Ball", &upperH, 180, NULL);

	cvCreateTrackbar("LowerS", "Ball", &lowerS, 256, NULL);
	cvCreateTrackbar("UpperS", "Ball", &upperS, 256, NULL);

	cvCreateTrackbar("LowerV", "Ball", &lowerV, 256, NULL);
	cvCreateTrackbar("UpperV", "Ball", &upperV, 256, NULL);
}



int main()
{
	cout << "Main started" << endl;
	CvCapture* capture = 0;
	capture = cvCaptureFromCAM(0);
	if (!capture)
	{
		printf("Capture failure\n");
		return -1;
	}
	cout << "capture done" << endl;

	IplImage* frame = 0;

	frame = cvQueryFrame(capture);
	if (!frame) return -1;
	setwindowSettings();

	imgTracking = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
	cvZero(imgTracking);


	while (true)
	{

		frame = cvQueryFrame(capture);
		if (!frame) break;
		frame = cvCloneImage(frame);

		cvSmooth(frame, frame, CV_GAUSSIAN, 3, 3); //smooth the original image using Gaussian kernel

		IplImage* imgHSV = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
		cvCvtColor(frame, imgHSV, CV_BGR2HSV); //Change the color format from BGR to HSV

		//cvShowImage("HSV", imgHSV);

		IplImage* imgThresh = GetThresholdedImage(imgHSV);

		cvSmooth(imgThresh, imgThresh, CV_GAUSSIAN, 3, 3); //smooth the binary image using Gaussian kernel


		cvShowImage("Ball", imgThresh);
		cvShowImage("Video", frame);

		//Clean up used images
		cvReleaseImage(&imgHSV);
		cvReleaseImage(&imgThresh);
		cvReleaseImage(&frame);

		//Wait 10mS
		int c = cvWaitKey(10);
		//If 'ESC' is pressed, break the loop
		if ((char)c == 27) break;
	}

	cvDestroyAllWindows();
	cvReleaseImage(&imgTracking);
	cvReleaseCapture(&capture);

	return 0;
}


*/