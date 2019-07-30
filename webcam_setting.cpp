/*
* author: Randy Chuang
* created time: May 31, 2019
* E-mail: sharmane578@gmail.com
*/

#include <iostream>
#include<opencv2/opencv.hpp>
#include"webcam_setting.hpp"

using namespace std;
using namespace cv;

//This function take the reference when it is called and use it as the setting object (the setting properties can be changed in "Config.hpp").
//It will output some error messange when the setting is failed. If all the setting is done, this function will return 1 as success.
int webcam_set(VideoCapture &cap) {
	int propId;
	int retval;

	switch (webcam_Capture_API)	// open the webcam with VideoCaptureAPIs (CAP_DSHOW for windows, CAP_V4L2 for linux)
	{
	case 0:
		cap.open(webcam_Selection, CAP_DSHOW);
		break;
	case 1:
		cap.open(webcam_Selection, CAP_V4L2);
		break;
	default:
		break;
	}

	if (!cap.isOpened()) {  // check if we succeeded
		cout << "Couldn't open capture. Check if there is something worong or not (id for selecting webcam or VideoCaptureAPI)." << endl;
		return 0;
	}

	switch (webcam_FOURCC)		//set the data format we want to retrieve from webcam
	{
	case 0:
		cap.set(CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', '2'));
		propId = CAP_PROP_FOURCC;
		retval = cap.get(propId);
		cout << "CAP_PROP_FOURCC :" << hex << retval << endl;
		if (retval != 0x32595559) {
			cout << "Error occurs while setting the data format as YUY2." << endl;
			return 0;
		}
		break;
	default:
		cout << "There are no appropriate data format for webcam_FOURCC flag. Please change the flag or add some statements above." << endl;
		return 0;
		break;
	}

	cap.set(CAP_PROP_FRAME_WIDTH, webcam_Width);		//set camera resolution
	cap.set(CAP_PROP_FRAME_HEIGHT, webcam_Height);

	propId = CAP_PROP_FRAME_WIDTH;
	retval = cap.get(propId);
	cout << "CV_CAP_PROP_FRAME_WIDTH :" << dec << retval << endl;
	if (retval != webcam_Width) {
		cout << "The webcam can't support to your resolution setting." << endl;
		return 0;
	}

	propId = CAP_PROP_FRAME_HEIGHT;
	retval = cap.get(propId);
	cout << "CV_CAP_PROP_FRAME_HEIGHT :" << dec << retval << endl;
	if (retval != webcam_Height) {
		cout << "The webcam can't support to your resolution setting." << endl;
		return 0;
	}

	cap.set(CAP_PROP_FPS, webcam_FPS);					// set the FPS of camera

	propId = CAP_PROP_FPS;
	retval = cap.get(propId);
	cout << "CAP_PROP_FPS :" << dec << retval << endl;
	if (retval != webcam_FPS) {
		cout << "The webcam can't support to your fps setting." << endl;
		return 0;
	}

	/*propId = CAP_PROP_EXPOSURE;		//the parameter of exposure in webcam 
	retval = cap.get(propId);
	cout << "CAP_PROP_EXPOSURE  :" << dec << retval << endl;*/

	return 1;
}
