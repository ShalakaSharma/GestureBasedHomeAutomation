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

IplImage* GetThresholdedImage(IplImage* imgHSV)
{
	IplImage* imgThresh = cvCreateImage(cvGetSize(imgHSV), IPL_DEPTH_8U, 1);
	cvInRangeS(imgHSV, cvScalar(0, 41, 26), cvScalar(64, 255, 255), imgThresh);
	return imgThresh;
}

double dist(Point x, Point y)
{

	return (x.x - y.x)*(x.x - y.x) + (x.y - y.y)*(x.y - y.y);
}

//This function returns the radius and the center of the circle given 3 points
//If a circle cannot be formed , it returns a zero radius circle centered at (0,0)
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
		
		
		vector<pair<Point, double> > palm_centers;
		cvNamedWindow("Webcam");
		cvNamedWindow("Threshold");


		frame = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);

		frame = cvLoadImage("pointer.jpg");

		if (!frame)
			cout << "NOT LOADED" << endl;




		frame = cvCloneImage(frame);

		cvSmooth(frame, frame, CV_GAUSSIAN, 9);

		imgHSV = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
		cvCvtColor(frame, imgHSV, CV_BGR2HSV);
		imgThresh = GetThresholdedImage(imgHSV);

		cvSmooth(imgThresh, imgThresh, CV_GAUSSIAN, 9);

		vector<vector<Point>> contours;


		Mat matCon(imgThresh);
		//imwrite("try1.threshold.jpg", matCon);
		Mat framemat(frame);
		Mat framemat2(frame);

		findContours(matCon, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		//drawContours(framemat, contours, -1, cv::Scalar(0, 0, 255), 2);

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
				
				drawContours(sig, tcontours, -1, cv::Scalar(0, 0, 0), 2);
			//	imwrite("try2.justcontour.jpg", sig);
				drawContours(sig, hulls, -1, cv::Scalar(255, 0, 0), 2);
			//	imwrite("try3.withhull.jpg", sig);
				/// finding the rectangle around the contour using the area
				RotatedRect rect = minAreaRect(Mat(tcontours[0]));  //// hullsI === tcontours

				vector<Vec4i> defects;

				if (hullsI[0].size() > 0)
				{
					Point2f rect_points[4];
					// storing the four corners of the rectangle 'rect' in rect_points
					rect.points(rect_points);

					/// drawing the rectangle by connecting the points found in 'rect'

					for (int j = 0; j < 4; j++)
					{
						line(sig, rect_points[j], rect_points[(j + 1) % 4], Scalar(240, 32, 160), 3, 8);

					}

				//	imwrite("try4.withrectangle.jpg", sig);
					Point rough_palm_center;

					convexityDefects(tcontours[0], hullsI[0], defects);

					if (defects.size() >= 3)
					{
						vector<Point> palm_points;

						for (int j = 0; j < defects.size(); j++)
						{
							int startidx = defects[j][0];               // starting index for each convexity defect
							Point ptStart(tcontours[0][startidx]);		// starting point for each convexity defect

							int endidx = defects[j][1];					// ending index for each convexity defect
							Point ptEnd(tcontours[0][endidx]);			 // ending point for each convexity defect

							int faridx = defects[j][2];					 // farthest index for each convexity defect
							Point ptFar(tcontours[0][faridx]);			 // farthest point for each convexity defect

							//Sum up all the hull and defect points to compute average
							rough_palm_center += ptFar + ptStart + ptEnd;

							palm_points.push_back(ptFar);
							palm_points.push_back(ptStart);
							palm_points.push_back(ptEnd);
						}


						//drawContours(sig, palm_points, -1, cv::Scalar(255, 255, 255), 2);


						//Get palm center by 1st getting the average of all defect points, this is the rough palm center,
						//Then U chose the closest 3 points and get the circle radius and center formed from them which is the palm center.
						rough_palm_center.x /= defects.size() * 3;
						rough_palm_center.y /= defects.size() * 3;

						Point closest_pt = palm_points[0];

						vector<pair<double, int> > distvec;
						
						for (int i = 0; i < palm_points.size(); i++)
							distvec.push_back(make_pair(dist(rough_palm_center, palm_points[i]), i));

						sort(distvec.begin(), distvec.end());

						//Keep choosing 3 points till you find a circle with a valid radius
						//As there is a high chance that the closes points might be in a linear line or too close that it forms a very large circle

						pair<Point, double> soln_circle;


						for (int i = 0; i + 2 < distvec.size(); i++)
						{
							Point p1 = palm_points[distvec[i + 0].second];
							Point p2 = palm_points[distvec[i + 1].second];
							Point p3 = palm_points[distvec[i + 2].second];
							soln_circle = circleFromPoints(p1, p2, p3);//Final palm center,radius
							if (soln_circle.second != 0)
								break;
						}


						//Find avg palm centers for the last few frames to stabilize its centers, also find the avg radius
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

					//Draw the palm center and the palm circle
					//The size of the palm gives the depth of the hand
					circle(sig, palm_center, 5, Scalar(144, 144, 255), 3);  /// showing the center
					circle(sig, palm_center, radius, Scalar(144, 144, 255), 2);  // showing the main circle
					//imwrite("try5.withcircle.jpg", sig);
					String ch = "";
					///finger detection
					int no_of_fingers = 0;
					int count = 0;
					for (int j = 0; j < defects.size(); j++)
					{
						int startidx = defects[j][0]; Point ptStart(tcontours[0][startidx]);
						int endidx = defects[j][1]; Point ptEnd(tcontours[0][endidx]);
						int faridx = defects[j][2]; Point ptFar(tcontours[0][faridx]);

						//X o--------------------------o Y
						double Xdist = sqrt(dist(palm_center, ptFar));
						double Ydist = sqrt(dist(palm_center, ptStart));

						double length1 = sqrt(dist(ptFar, ptEnd));
						double length2 = sqrt(dist(ptFar, ptStart));
						double length3 = sqrt(dist(ptEnd, ptStart));



						if (length2 > 50)
						{
							line(sig, ptStart, ptFar, Scalar(0, 0, 255), 2);
							circle(sig, ptFar, 5, Scalar(144, 144, 255), 3);  

							cout << "---------------------" << endl;
							cout << "Start-Far: " << length2 << endl;
							count++;
						}
					//	imwrite("try6.with defects.jpg", sig);
					}

					char ans;
					int k = 0;
					if (count == 0 || count == 1 || count == 2)
					{
						cout << "No. of fingers: " << count << endl;
						if (count == 0)
						{
							ans = '2';
							ch = "PALM CLOSE";
						}
						else if (count == 1)
						{
							ans = '3';
							ch = "THUMBS UP";
						}
						else if (count == 2)
						{
							ans = '3';
							ch = "THUMBS UP";
						}
						else
						{
						}
						putText(sig, ch, cvPoint(30, 30 + k), FONT_HERSHEY_PLAIN, 2, cvScalar(200, 200, 250), 1, CV_AA);
						k = k + 5;
					}

					else if (count == 3 || count == 4 || count == 5 || count == 6)
					{
						cout << "No. of fingers: " << (count - 1) << endl;
						if (count == 3 || count == 4)
						{
							ans = '4';
							ch = "THUMB + FORE FINGER";
						}
						else if (count >= 5)
						{
							ans = '1';
							ch = "PALM OPEN";
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


					iResult = send(SendSocket, &ans, 1, 0); //successful, sentSize = 1


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


		//imwrite("pointerresult.jpg", sig);
		cvShowImage("Webcam", frame);
		cvShowImage("Threshold", imgThresh);
		imshow("Significant", sig);

		cvWaitKey(0);

		cvReleaseImage(&frame);
		cvReleaseImage(&imgHSV);
		cvReleaseImage(&imgThresh);
		cvDestroyAllWindows();

		closesocket(SendSocket);
		WSACleanup();
	}
	

	return 0;
}


///   0 - 37   37 - 150 44-255      /// 0-37 37-192 44-255
*/