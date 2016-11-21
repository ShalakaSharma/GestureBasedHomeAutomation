/*#include <opencv2\opencv.hpp>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <vector>

using namespace cv;
using namespace std;


IplImage* GetThresholdedImage(IplImage* imgHSV)
{
IplImage* imgThresh = cvCreateImage(cvGetSize(imgHSV), IPL_DEPTH_8U, 1);
cvInRangeS(imgHSV, cvScalar(0, 81, 78), cvScalar(182, 256, 256), imgThresh);
return imgThresh;
}


IplImage* frame = 0;
IplImage* imgHSV = 0;
IplImage* imgThresh = 0;


int main()
{
	CvCapture* capture = 0;
	cvNamedWindow("Webcam");
	cvNamedWindow("Threshold");

	for (;;)
	{
		capture = cvCaptureFromCAM(0);
		if (!capture)
		{
			printf("Capture failure\n");
			return -1;
		}

		frame = cvQueryFrame(capture);

		if (!frame)
			break;


		frame = cvCloneImage(frame);
		cvSmooth(frame, frame, CV_GAUSSIAN, 9);

		imgHSV = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
		cvCvtColor(frame, imgHSV, CV_BGR2HSV);
		imgThresh = GetThresholdedImage(imgHSV);

		cvSmooth(imgThresh, imgThresh, CV_GAUSSIAN, 9);

		vector<vector<Point>> contours;


		Mat matCon(imgThresh);
		Mat framemat(frame);
		Mat framemat2(frame);

		findContours(matCon, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		//drawContours(framemat, contours, -1, cv::Scalar(0, 0, 255), 2);

		vector<vector<Point>> tcontours;
		vector<vector<Point>> hulls(1);
		vector<vector<int>> hullsI(1);

		Mat sig = framemat2;

		cout << "size of contours: " << contours.size() << endl;



		for (int i = 0; i < contours.size(); i++)
		{


			if (contourArea(contours[i])>8000)
			{
				tcontours.push_back(contours[i]);
				convexHull(Mat(tcontours[0]), hulls[0], false);
				convexHull(Mat(tcontours[0]), hullsI[0], false);
				drawContours(sig, hulls, -1, cv::Scalar(255, 0, 0), 2);
				drawContours(sig, tcontours, -1, cv::Scalar(0, 0, 0), 2);

				cout << "Area of contour " << i << " : " << contourArea(contours[i]) << endl;

				if (contourArea(contours[i]) > 8000 && contourArea(contours[i]) < 11000)
				{
					putText(sig, "0", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
					cout << "The number of fingers: 0" << endl;
				}
				else if (contourArea(contours[i]) > 11000 && contourArea(contours[i]) < 15000)
				{
					putText(sig, "1", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
					cout << "The number of fingers: 1" << endl;
				}
				else if (contourArea(contours[i]) > 15000 && contourArea(contours[i]) < 17000)
				{
					putText(sig, "2", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
					cout << "The number of fingers: 2" << endl;
				}
				else if (contourArea(contours[i]) > 17000 && contourArea(contours[i]) < 19000)
				{
					putText(sig, "3", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
					cout << "The number of fingers: 3" << endl;
				}
				else if (contourArea(contours[i]) > 19000 && contourArea(contours[i]) < 22000)
				{
					putText(sig, "4", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
					cout << "The number of fingers: 4" << endl;
				}
				else if (contourArea(contours[i]) > 22000)
				{
					putText(sig, "5", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
					cout << "The number of fingers: 5" << endl;
				}
				else
				{

				}



			}
		}

		imwrite("try.jpg", sig);
		cvShowImage("Webcam", frame);
		cvShowImage("Threshold", imgThresh);
		imshow("Significant", sig);

		int c = cvWaitKey(1000);
		if ((char)c == 27)
			break;


		cvReleaseImage(&frame);
		cvReleaseImage(&imgHSV);
		cvReleaseImage(&imgThresh);


	}


	cvDestroyAllWindows();
	cvReleaseCapture(&capture);

	return 0;
}
*/