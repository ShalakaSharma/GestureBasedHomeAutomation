/*#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

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

#pragma comment(lib, "Ws2_32.lib")

IplImage* GetThresholdedImage(IplImage* imgHSV)
{
	IplImage* imgThresh = cvCreateImage(cvGetSize(imgHSV), IPL_DEPTH_8U, 1);
	cvInRangeS(imgHSV, cvScalar(0, 28, 0), cvScalar(22, 255, 255), imgThresh);
	return imgThresh;
}

double dist(Point x, Point y)
{
	return (x.x - y.x)*(x.x - y.x) + (x.y - y.y)*(x.y - y.y);
}


pair<Point, double> circleFromPoints(Point p1, Point p2, Point p3)
{
	double offset = pow(p2.x, 2) + pow(p2.y, 2);
	double bc = (pow(p1.x, 2) + pow(p1.y, 2) - offset) / 2.0;
	double cd = (offset - pow(p3.x, 2) - pow(p3.y, 2)) / 2.0;
	double det = (p1.x - p2.x) * (p2.y - p3.y) - (p2.x - p3.x)* (p1.y - p2.y);
	double TOL = 0.0000001;
	if (abs(det) < TOL) { cout << "POINTS TOO CLOSE" << endl; return make_pair(Point(0, 0), 0); }

	double idet = 1 / det;
	double centerx = (bc * (p2.y - p3.y) - cd * (p1.y - p2.y)) * idet;
	double centery = (cd * (p1.x - p2.x) - bc * (p2.x - p3.x)) * idet;
	double radius = sqrt(pow(p2.x - centerx, 2) + pow(p2.y - centery, 2));

	return make_pair(Point(centerx, centery), radius);
}

IplImage* frame = 0;
IplImage* imgHSV = 0;
IplImage* imgThresh = 0;

int main()
{

	CvCapture* capture = 0;
	vector<pair<Point, double> > palm_centers;
	cvNamedWindow("Webcam");
	cvNamedWindow("Threshold");


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
		btSockAddr.serviceClassId = RFCOMM_PROTOCOL_UUID; 
		btSockAddr.port = BT_PORT_ANY;



		

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
			
			vector<vector<Point>> tcontours;
			vector<vector<Point>> hulls(1);
			vector<vector<int>> hullsI(1);

			Mat sig = framemat2;

			for (int i = 0; i < contours.size(); i++)
			{


				if (contourArea(contours[i])>7000)
				{
					tcontours.push_back(contours[i]);
					convexHull(Mat(tcontours[0]), hulls[0], false);
					convexHull(Mat(tcontours[0]), hullsI[0], false);
					drawContours(sig, hulls, -1, cv::Scalar(255, 0, 0), 2);
					drawContours(sig, tcontours, -1, cv::Scalar(0, 0, 0), 2);

					
					RotatedRect rect = minAreaRect(Mat(tcontours[0]));

					vector<Vec4i> defects;

					if (hullsI[0].size() > 0)
					{
						Point2f rect_points[4];
						
						rect.points(rect_points);

						

						for (int j = 0; j < 4; j++)
						{
							line(sig, rect_points[j], rect_points[(j + 1) % 4], Scalar(240, 32, 160), 3, 8);

						}


						Point rough_palm_center;

						convexityDefects(tcontours[0], hullsI[0], defects);

						if (defects.size() >= 3)
						{
							vector<Point> palm_points;

							for (int j = 0; j < defects.size(); j++)
							{
								int startidx = defects[j][0];               
								Point ptStart(tcontours[0][startidx]);		

								int endidx = defects[j][1];					
								Point ptEnd(tcontours[0][endidx]);			 

								int faridx = defects[j][2];					
								Point ptFar(tcontours[0][faridx]);			

								
								rough_palm_center += ptFar + ptStart + ptEnd;

								palm_points.push_back(ptFar);
								palm_points.push_back(ptStart);
								palm_points.push_back(ptEnd);
							}


							


							
							rough_palm_center.x /= defects.size() * 3;
							rough_palm_center.y /= defects.size() * 3;

							Point closest_pt = palm_points[0];

							vector<pair<double, int> > distvec;

							for (int i = 0; i < palm_points.size(); i++)
								distvec.push_back(make_pair(dist(rough_palm_center, palm_points[i]), i));

							sort(distvec.begin(), distvec.end());

						

							pair<Point, double> soln_circle;


							for (int i = 0; i + 2 < distvec.size(); i++)
							{
								Point p1 = palm_points[distvec[i + 0].second];
								Point p2 = palm_points[distvec[i + 1].second];
								Point p3 = palm_points[distvec[i + 2].second];
								soln_circle = circleFromPoints(p1, p2, p3);
								if (soln_circle.second != 0)
									break;
							}


							
							palm_centers.push_back(soln_circle);
							if (palm_centers.size() > 20)
								palm_centers.erase(palm_centers.begin());

						}
						Point palm_center;
						double radius = 0;
						for (int i = 0; i < palm_centers.size(); i++)
						{
							palm_center += palm_centers[i].first;
							radius += palm_centers[i].second;
						}
						palm_center.x /= palm_centers.size();
						palm_center.y /= palm_centers.size();
						radius /= palm_centers.size();

					
						circle(sig, palm_center, 5, Scalar(144, 144, 255), 3); 
						circle(sig, palm_center, radius, Scalar(144, 144, 255), 2);  

						String ch = "";
						
						int no_of_fingers = 0;
						int count = 0;
						for (int j = 0; j < defects.size(); j++)
						{
							int startidx = defects[j][0]; Point ptStart(tcontours[0][startidx]);
							int endidx = defects[j][1]; Point ptEnd(tcontours[0][endidx]);
							int faridx = defects[j][2]; Point ptFar(tcontours[0][faridx]);

							
							double Xdist = sqrt(dist(palm_center, ptFar));
							double Ydist = sqrt(dist(palm_center, ptStart));
							

							double length1 = sqrt(dist(ptFar, ptEnd));
							double length2 = sqrt(dist(ptFar, ptStart));
							
							double length3 = sqrt(dist(ptEnd, ptStart));



							if (length2 > 50)
							{
								line(sig, ptStart, ptFar, Scalar(0, 0, 255), 2);
								circle(sig, ptFar, 5, Scalar(144, 144, 255), 3);
								count++;
							}
						}
						char ans;
						

						if (count == 0 || count == 1 || count == 2)
						{
							cout << "No. of fingers: " << count << endl;
							if (count == 0)
							{
								ch = "PALM CLOSE";
								ans = '2';
							}
							else if (count == 1)
							{
								ch = "THUMBS UP";
								ans = '3';
							}
							else if (count == 2)
							{
								ch = "THUMBS UP";
								ans = '3';
							}
							else
							{
							}
							putText(sig, ch, cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);

						}

						else if (count == 3 || count == 4 || count == 5 || count == 6)
						{
							cout << "No. of fingers: " << (count - 1) << endl;
							if (count == 3 || count == 4)
							{
								ch = "THUMB + FORE FINGER";
								ans = '4';
							}
							else if (count >= 5)
							{
								ch = "PALM OPEN";
								ans = '1';
							}
							else {}
							putText(sig, ch, cvPoint(30, 30), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);

						}

						else {}
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


						iResult = send(SendSocket, &ans, 1, 0);


						if (iResult == SOCKET_ERROR)
						{
							wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
							closesocket(SendSocket);
							WSACleanup();
							char g = getchar(); 
							return 1;
						}
						else
							cout << "Sendto worked and value sent is: " << ans << endl;
						
					}
				}
			}



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
		closesocket(SendSocket);
		WSACleanup();
	}
	return 0;
}

*/