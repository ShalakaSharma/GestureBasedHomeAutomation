#ifndef UNICODE
#define UNICODE
#endif

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <algorithm>
#include <vector>

#include <Winsock2.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>


using namespace cv;
using namespace std;

Mat sig;


IplImage* GetThresholdedImage(IplImage* imgHSV)
{
	IplImage* imgThresh = cvCreateImage(cvGetSize(imgHSV), IPL_DEPTH_8U, 1);
	cvInRangeS(imgHSV, cvScalar(0, 120, 33), cvScalar(180, 255, 255), imgThresh);  //// hsvs
	return imgThresh;
}


double dist(Point x, Point y)
{
	return (x.x - y.x)*(x.x - y.x) + (x.y - y.y)*(x.y - y.y);
}

char motion(Point curr, Point prev)
{
	char detect = '0';
	cout << "MOTION CALLED" << endl;
	if (sqrt(dist(curr, prev)) > 200)
	{
		cout << "MOTION CALLED::: first if" << endl;
		//cout << "diff: " << abs(curr.x - curr.x > 50) << endl;
		for (int j = 0; j < 20; j++)
			cout << curr.x << " and " << prev.x << endl;
		int d1, d2;
		d1 = abs(curr.x - prev.x);
		d2 = abs(curr.y - prev.y);
		cout << "D1 and D2:" << d1 << "  " << d2 << endl;
		//if (d1>d2)
		//{
		if (d1 > 250)
		if ((curr.x < prev.x))// && (abs(curr.x - prev.x>50)))
		{
			cout << "LEFT TO RIGHT" << endl;
			cout << curr.x << " and " << prev.x << endl;
			putText(sig, "LEFT TO RIGHT", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(0, 0, 250), 1, CV_AA);
			circle(sig, curr, 5, Scalar(45, 215, 255), 2);
			circle(sig, prev, 5, Scalar(45, 215, 255), 2);
			detect = '1';
			//return true;
		}
		else if ((curr.x > prev.x))//&& (abs(curr.x - prev.x>50)))
		{
			cout << "---------------------------- RIGHT TO LEFT-------" << endl;
			//for (int j = 0; j < 20;j++)
			//cout << curr.x << " and " << prev.x << endl;
			putText(sig, "RIGHT TO LEFT", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(0, 0, 250), 1, CV_AA);
			circle(sig, curr, 5, Scalar(45, 215, 255), 2);
			circle(sig, prev, 5, Scalar(45, 215, 255), 2);
			detect = '2';
			//return true;
		}
	}
	/*if (d2 > d1)
	{
	if (d2 > 100)
	if ((curr.y > prev.y))//&& (abs(curr.y - prev.y>50)))
	{
	cout << " HAND WENT DOWN" << endl;
	cout << curr.y << " and " << prev.y << endl;
	putText(sig, " HAND WENT DOWN", cvPoint(100, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
	circle(sig, curr, 5, Scalar(45, 215, 255), 2);
	circle(sig, prev, 5, Scalar(45, 215, 255), 2);
	detect = '3';
	//return true;


	}
	else if ((curr.y < prev.y))// && (abs(curr.y - prev.y>50)))
	{
	cout << " HAND WENT UP" << endl;
	cout << curr.y << " and " << prev.y << endl;
	putText(sig, "HAND WENT UP", cvPoint(100, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
	circle(sig, curr, 5, Scalar(45, 215, 255), 2);
	circle(sig, prev, 5, Scalar(45, 215, 255), 2);
	detect = '4';
	//return true;
	}
	}*/
//}
	return detect;
	//cout <<"x.x = "<< x.x << endl;
}

IplImage* frame = 0;
IplImage* imgHSV = 0;
IplImage* imgThresh = 0;

int main()
{


	CvCapture* capture = 0;
	vector<pair<Point, double> > palm_centers;
	cvNamedWindow("Webcam");
	cvNamedWindow("Significant");


	vector<Vec2i> gest;
	int countgest = 0;


	HANDLE hRadio, hDeviceFind;
	BLUETOOTH_DEVICE_INFO_STRUCT deviceInfo;
	BLUETOOTH_DEVICE_SEARCH_PARAMS deviceSearchParams;
	BLUETOOTH_RADIO_INFO radioInfo;
	GUID guidServices[10];
	DWORD numServices, result;

	BLUETOOTH_FIND_RADIO_PARAMS btfrp = { sizeof(BLUETOOTH_FIND_RADIO_PARAMS) };
	HBLUETOOTH_RADIO_FIND hFind = BluetoothFindFirstRadio(&btfrp, &hRadio);

	int iResult;
	WSADATA wsaData;
	int err = SOCKET_ERROR;
	SOCKET SendSocket = INVALID_SOCKET;
	SOCKADDR_BTH btSockAddr;

	if (hFind != NULL)
	{
		printf("BluetoothFindFirstRadio() is working!\n");

		radioInfo.dwSize = sizeof(radioInfo);
		//cout << sizeof(radioInfo) << endl;
		if (BluetoothGetRadioInfo(hRadio, &radioInfo) == ERROR_SUCCESS)
		{
			printf("BluetoothGetRadioInfo() is OK!\n");
			printf("---Radio found: %S\n", radioInfo.szName);
		}
		else
			printf("BluetoothGetRadioInfo() failed with error code %d\n", GetLastError());

		deviceInfo.dwSize = sizeof(deviceInfo);
		memset(&deviceSearchParams, 0, sizeof(deviceSearchParams));
		deviceSearchParams.dwSize = sizeof(deviceSearchParams);


		deviceSearchParams.fReturnRemembered = TRUE;

		deviceSearchParams.hRadio = hRadio;

		hDeviceFind = BluetoothFindFirstDevice(&deviceSearchParams, &deviceInfo);

		if (hDeviceFind != NULL)
		{
			printf("BluetoothFindFirstDevice() is OK!\n");
			printf(" ---Device found - Name: %S\n", deviceInfo.szName);
			printf(" ---Device found - Address: %X\n", deviceInfo.Address);
			printf(" ---Device found - Device Class: %ul\n", deviceInfo.ulClassofDevice);
		}





		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != NO_ERROR) {
			wprintf(L"WSAStartup failed with error: %d\n", iResult);
			char g = getchar();
			return 1;
		}
		else
			cout << "started" << endl;


		SendSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

		if (SendSocket == INVALID_SOCKET)
		{
			wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			char g = getchar();
			return 1;
		}
		else
			cout << "Socket worked" << endl;


		ZeroMemory(&btSockAddr, sizeof(SOCKADDR_BTH));

		btSockAddr.addressFamily = AF_BTH;
		btSockAddr.btAddr = deviceInfo.Address.ullLong;
		btSockAddr.serviceClassId = RFCOMM_PROTOCOL_UUID; //SerialPortServiceClass_UUID
		btSockAddr.port = BT_PORT_ANY;


		for (;;)
		{
			// notyet = true;
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
			sig = framemat;
			for (int i = 0; i < contours.size(); i++)
			{


				if (contourArea(contours[i])>8000)
				{
					cout << "contour area: " << contourArea(contours[i]) << endl;
					tcontours.push_back(contours[i]);
					convexHull(Mat(tcontours[0]), hulls[0], false);
					convexHull(Mat(tcontours[0]), hullsI[0], false);
					drawContours(sig, hulls, -1, cv::Scalar(255, 0, 0), 2);
					drawContours(sig, tcontours, -1, cv::Scalar(0, 0, 0), 2);

					/// finding the rectangle around the contour using the area
					RotatedRect rect = minAreaRect(Mat(tcontours[0]));

					vector<Vec4i> defects;

					if (hullsI[0].size() > 0)
					{
						Point2f rect_points[4];
						// storing the four corners of the rectangle 'rect' in rect_points
						rect.points(rect_points);

						/// drawing the rectangle by connecting the points found in 'rect'
						cout << "FOUR:" << endl;
						int maxright = -10000;
						Point2f inter[4];
						int maxleft = 10000;

						for (int j = 0; j < 4; j++)
						{
							cout << "FOR LOOP" << endl;
							line(sig, rect_points[j], rect_points[(j + 1) % 4], Scalar(240, 32, 160), 3, 8);
							double s = (rect_points[(j + 1) % 4].y - rect_points[j].y) / (rect_points[(j + 1) % 4].x - rect_points[j].x);
							Point2f cent;
							if (s < 0)
							{

								if ((rect_points[j].x < rect_points[(j + 1) % 4].x) && (rect_points[j].y > rect_points[(j + 1) % 4].y))
								{
									cent.x = rect_points[j].x + abs((rect_points[(j + 1) % 4].x - rect_points[j].x) / 2);
									cent.y = rect_points[(j + 1) % 4].y + abs((rect_points[(j + 1) % 4].y - rect_points[j].y) / 2);
									inter[j].x = cent.x;
									inter[j].y = cent.y;
								}
								else if ((rect_points[j].x > rect_points[(j + 1) % 4].x) && (rect_points[j].y < rect_points[(j + 1) % 4].y))
								{
									cent.x = rect_points[(j + 1) % 4].x + abs((rect_points[(j + 1) % 4].x - rect_points[j].x) / 2);
									cent.y = rect_points[j].y + abs((rect_points[(j + 1) % 4].y - rect_points[j].y) / 2);
									inter[j].x = cent.x;
									inter[j].y = cent.y;
								}
							}
							else if (s > 0)
							{
								if ((rect_points[j].x < rect_points[(j + 1) % 4].x) && (rect_points[j].y < rect_points[(j + 1) % 4].y))
								{
									cent.x = rect_points[j].x + abs((rect_points[(j + 1) % 4].x - rect_points[j].x) / 2);
									cent.y = rect_points[j].y + abs((rect_points[(j + 1) % 4].y - rect_points[j].y) / 2);
									inter[j].x = cent.x;
									inter[j].y = cent.y;
								}
								else if ((rect_points[j].x > rect_points[(j + 1) % 4].x) && (rect_points[j].y > rect_points[(j + 1) % 4].y))
								{
									cent.x = rect_points[(j + 1) % 4].x + abs((rect_points[(j + 1) % 4].x - rect_points[j].x) / 2);
									cent.y = rect_points[(j + 1) % 4].y + abs((rect_points[(j + 1) % 4].y - rect_points[j].y) / 2);
									inter[j].x = cent.x;
									inter[j].y = cent.y;
								}
							}
							circle(sig, cent, 5, Scalar(0, 215, 255), 2);
						}
						cout << "FOR LOOP ENDED" << endl;
						Point fc;

						fc.x = inter[0].x + ((inter[2].x - inter[0].x) / 2);
						fc.y = inter[3].y + ((inter[1].y - inter[3].y) / 2);
						countgest = countgest + 1;
						gest.push_back(fc);
						cout << "pushed" << endl;
						int h;

						for (h = 0; h < gest.size(); h++)
							cout << gest[h] << endl;

						h--;
						cout << "h=" << h << endl;
						circle(sig, fc, 5, Scalar(0, 215, 255), 2);
						cout << "circle drawn" << endl;
						cout << "Centre : " << fc << endl;
						if (countgest != 1)
						{
							cout << "Entered if" << endl;
							cout << "Centres: " << gest[h] << " and " << gest[h - 1] << endl;
							cout << "Distance: " << sqrt(dist(gest[h], gest[h - 1])) << endl;
							int diff1 = 0;
							int diff2 = 0;
							char detect = '0';
							detect = motion(gest[h], gest[h - 1]);
							if (detect != '0')
							{
								if (err == SOCKET_ERROR)
									err = connect(SendSocket, reinterpret_cast<SOCKADDR*>(&btSockAddr), sizeof(SOCKADDR_BTH));

								if (err == SOCKET_ERROR) {
									wprintf(L"connect failed with error: %d\n", WSAGetLastError());
									closesocket(SendSocket);
									WSACleanup();
									cout << "Press any key to exit..." << endl;
									char g = getchar();
									return 1;
								}
								else
									cout << "connect worked" << endl;


								iResult = send(SendSocket, &detect, 1, 0);


								if (iResult == SOCKET_ERROR)
								{
									wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
									closesocket(SendSocket);
									WSACleanup();
									char g = getchar();
									return 1;
								}
								else
									cout << "Sendto worked and value sent is: " << detect << endl;
							}

							/*if (sqrt(dist(gest[h], gest[h - 1])) > 200)
							{
							putText(sig, "HAND MOVED", cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
							}*/
							/*if (detect == true)
							{
							fc.x = 0;
							fc.y = 0;
							gest.push_back(fc);
							gest.push_back(fc);
							}*/
						}
					}
				}
			}

			cvShowImage("Webcam", frame);
			imshow("Significant", sig);
			int c = cvWaitKey(1000);
			if ((char)c == 27)
				break;


			cvReleaseImage(&frame);

		}

		cvDestroyAllWindows();
		cvReleaseCapture(&capture);
		closesocket(SendSocket);
		WSACleanup();
	}
	return 0;
}




///// blue hsv  59 108 0    ---- 140 256 256
