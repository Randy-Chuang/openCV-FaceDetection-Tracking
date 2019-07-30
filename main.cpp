#include <iostream>
#include <string>
#include <vector>
#include<conio.h>
using namespace std;

//all
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
//dnn model
#include <opencv2/dnn.hpp>
//tracking
#include <opencv2/tracking.hpp>

//webcam_setting
#include"webcam_setting.hpp"

using namespace cv;
using namespace cv::dnn;

/*******************Global variables**********************/
typedef struct FaceID {
	bool valid;
	Rect2d Roi;
}FaceID;

class FaceDetector {
private:
	const size_t inWidth = 300;
	const size_t inHeight = 300;
	const double inScaleFactor = 1.0;
	const float confidenceThreshold = 0.8;
	const Scalar meanVal = cv::Scalar(104.0, 177.0, 123.0);
	static const int numFaceID = 10;

	bool DetectionFlag;
	bool TrackingFlag;

	Mat frame;

	Net net;
	int netType;
	FaceID FaceRoi[numFaceID];

	Ptr<Tracker> tracker;
	int trackerType;
	Rect2d trackingRect2d;
	int trackingID;
	int trackingFailureCount;
	const int trackingFailureThreshold = 40;

	double processingFreq;

public:
	/*Initialize the Dnn model and Tracker in FaceDetector.
	* DnnPrototxtFilePath: the prototype text file path of the Dnn model.
	* DnnModelFilePath: the binary file path of the Dnn model.
	* netSelection: indicate which type of Dnn model is adapted, 1 => caffe, 2 => tensorflow.
	* TrackerSelection: indicate which type of tracker is adapted;
	* 1 => KCF
	* 2 => MedianFlow
	* 3 => CSRT
	*/
	FaceDetector(const string DnnPrototxtFilePath, const string DnnModelFilePath, const int netSelection, const int trackerSelection) {

		setDnnModel(DnnPrototxtFilePath, DnnModelFilePath, netSelection);

		if (netSelection < 1 || netSelection>2) {
			cout << "netSelection: Input selection is not an option!\n";
			system("pause");
			exit(-1);
		}
		if (trackerSelection < 1 || trackerSelection>3) {
			cout << "trackerSelection: Input selection is not an option!\n";
			system("pause");
			exit(-1);
		}

		netType = netSelection;
		trackerType = trackerSelection;

		DetectionFlag = true;
		TrackingFlag = false;
	}

	void setDnnModel(const string DnnPrototxtFilePath, const string DnnModelFilePath, const int netSelection) {

		if (!net.empty())	return;	// the Dnn model has been already loaded

		switch (netSelection) {
		case 1:
			net = readNetFromCaffe(DnnPrototxtFilePath, DnnModelFilePath);
			break;

		case 2:
			net = readNetFromTensorflow(DnnModelFilePath, DnnPrototxtFilePath);
			break;

		default:
			cout << "Input selection is not an option!\n";
			system("pause");
			exit(-1);
			break;
		}

		net.setPreferableBackend(DNN_BACKEND_DEFAULT);//DNN_BACKEND_DEFAULT or DNN_BACKEND_OPENCV
		net.setPreferableTarget(DNN_TARGET_CPU);	//DNN_TARGET_CPU or DNN_TARGET_OPENCL or DNN_TARGET_OPENCL_FP16

		if (net.empty()) {	//Checking dnn model 
			cerr << "Can't load the neural network." << endl;
			cerr << "Maybe choose other dnn model to try?" << endl;
			cerr << "https://github.com/opencv/opencv_extra/blob/master/testdata/dnn/download_models.py#L225-L239" << endl;
			system("pause");
			exit(-1);
		}
	}

	bool setTracker(Mat &InputFrame, Rect2d &TrackingRoi) {
		if (InputFrame.empty()) {
			cout << "Tracker initialization error: input frame is empty!\n";
			return false;
		}
		if (TrackingRoi.width == 0 || TrackingRoi.height == 0) {
			cout << "Tracker initialization error: sidelength of input Roi is zero!\n";
			return false;
		}
		cout << "Tracker pointer:" << tracker << endl;
		if (tracker) {
			cout << "Release current tracker!\n";
			tracker.release();
			cout << "Tracker pointer:" << tracker << endl;
		}
		cout << "Re-create the tracker!\n";

		switch (trackerType)
		{
		case 1:
			tracker = TrackerKCF::create();
			break;
		case 2:
			tracker = TrackerMedianFlow::create();
			break;
		case 3:
			tracker = TrackerCSRT::create();
			break;
		default:
			cout << "Input selection is not an option!\n";
			system("pause");
			exit(-1);
			break;
		}

		cout << "Tracker pointer:" << tracker << endl;

		if (!tracker->init(InputFrame, TrackingRoi)) {
			cout << "Tracker initialization error!\n";
			return false;
		}

		return true;
	}

	void changeMode() {
		DetectionFlag = !DetectionFlag;
		TrackingFlag = !TrackingFlag;
	}

	bool IsDetectionMode() {
		if (DetectionFlag == true && TrackingFlag == false)	return true;
		else if (DetectionFlag == false && TrackingFlag == true)	return false;
		else {
			cout << "Something wrong with the flag of state!\n";
			return false;
		}
	}

	bool IsTrackingMode() {
		if (DetectionFlag == false && TrackingFlag == true)	return true;
		else if (DetectionFlag == true && TrackingFlag == false)	return false;
		else {
			cout << "Something wrong with the flag of state!\n";
			return false;
		}
	}

	bool selectRoi(int id) {
		if (!IsDetectionMode()) {
			cout << "Not in the detection mode! And there won't be any detected face.\n";
			return false;
		}

		if (!FaceRoi[id].valid) {
			cout << "ID " << id << " is not found!\n";
			return false;
		}

		if (!setTracker(frame, FaceRoi[id].Roi))	return false;

		trackingID = id;
		trackingFailureCount = 0;

		changeMode();

		return true;
	}

	bool deteck(Mat &InputFrame) {
		int64 t = cv::getTickCount();

		bool returnValue = false;

		frame = InputFrame.clone();

		if (IsDetectionMode()) {

			memset(FaceRoi, 0, numFaceID * sizeof(FaceID));

			int frameHeight = InputFrame.rows;
			int frameWidth = InputFrame.cols;

			if (InputFrame.channels() == 4)	cvtColor(InputFrame, InputFrame, COLOR_BGRA2BGR);

			//! [Prepare blob]	Convert Mat to batch of images
			Mat inputBlob;

			switch (netType)
			{
			case 1:
				inputBlob = blobFromImage(InputFrame, inScaleFactor, cv::Size(inWidth, inHeight), meanVal, false, false);
				break;
			case 2:
				inputBlob = blobFromImage(InputFrame, inScaleFactor, cv::Size(inWidth, inHeight), meanVal, true, false);
				break;
			default:
				cout << "Input selection is not an option!\n";
				system("pause");
				exit(-1);
				break;
			}
			//! [Prepare blob]

			//! [Set input blob]
			net.setInput(inputBlob, "data"); //set the network input
			//! [Set input blob]

			//! [Make forward pass]
			Mat detection = net.forward("detection_out"); //compute output
			//! [Make forward pass]

			Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

			int count = 0;
			for (int i = 0; i < detectionMat.rows; i++)
			{
				float confidence = detectionMat.at<float>(i, 2);

				if (confidence > confidenceThreshold && count < numFaceID - 1)
				{
					returnValue = true;
					count++;
					int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frameWidth);
					int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frameHeight);
					int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frameWidth);
					int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frameHeight);

					FaceRoi[count].valid = 1;
					FaceRoi[count].Roi = cv::Rect2d(cv::Point(x1, y1), cv::Point(x2, y2));

					cv::rectangle(InputFrame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2, 4);

					string text = "ID: " + to_string(count);
					putText(InputFrame, text, cv::Point(x1, y1), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 2);
				}
			}
		}
		else if (IsTrackingMode()) {
			if (tracker->update(InputFrame, trackingRect2d))	trackingFailureCount = 0;
			else												trackingFailureCount++;

			if (trackingFailureCount >= trackingFailureThreshold) {
				cout << "Tracker fails to track the face!\n";
				changeMode();
				return false;
			}
			returnValue = true;

			// draw the tracked object
			rectangle(InputFrame, trackingRect2d, Scalar(0, 255, 0), 2, 1);
			string text = "ID: " + to_string(trackingID);
			putText(InputFrame, text, cv::Point(trackingRect2d.x, trackingRect2d.y), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 2);
		}

		double tt_opencvDNN = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		processingFreq = 1 / tt_opencvDNN;

		return returnValue;
	}

	double queryProcessingFreq() {
		return processingFreq;
	}
};


const String caffe_res10_modelConfiguration = "..\\..\\Face_Detection_model_file\\deploy_res10.prototxt.txt";
const String caffe_res10_modelBinary = "..\\..\\Face_Detection_model_file\\res10_300x300_ssd_iter_140000.caffemodel";

const string caffe_res10_fp16_modelConfiguration = "..\\..\\Face_Detection_model_file\\deploy_res10_fp16.prototxt.txt";
const string caffe_res10_fp16_modelBinary = "..\\..\\Face_Detection_model_file\\res10_300x300_ssd_iter_140000_fp16.caffemodel";

const string tensorflow_modelConfiguration = "..\\..\\Face_Detection_model_file\\opencv_face_detector.pbtxt.txt";
const string tensorflow_modelBinary = "..\\..\\Face_Detection_model_file\\opencv_face_detector_uint8.pb";


/*******************Function pre-declarations**********************/


/*******************Frequently used function**********************/
template <typename T>
string to_string_with_precision(const T a_value, const int n = 2)
{
	ostringstream out;
	//out.precision(n);
	out << fixed << setprecision(n) << a_value;
	return out.str();
}


int main() {
	/***************************Variables*****************************************/
	Mat frame;
	VideoCapture cap;	// define a variable of the class "VideoCapture", so that we can use the member function to set the webcam properties even to get the frames
	FaceDetector detector(caffe_res10_modelConfiguration, caffe_res10_modelBinary, 1, 2);

	namedWindow("Camera implementation", WINDOW_AUTOSIZE);


	/**********************Webcam properties setting*****************************/
	int set = webcam_set(cap);
	if (set == 0) {
		cout << "Error occured while setting webcam properties." << endl;
		system("pause");
		return -1;
	}


	/*****************************Processing************************************/
	int64 tPrevious = 0, t;
	double tt = 0;
	double fps = 0;

	while (cap.read(frame)) {		//read() return a bool value to check if the reading operation succeeds or not
									//and read() put the output frame to the Mat datatype variable
									//cap >> frame;
		//detector.deteck(frame);
		if (detector.deteck(frame)) {
			putText(frame, "detected", Point(10, 30), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 2);
		}

		if (detector.IsDetectionMode()) {
			putText(frame, format("Detection mode: f=%.2f/s|", detector.queryProcessingFreq()), Point(10, 470), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 2);
		}
		else if (detector.IsTrackingMode()) {
			putText(frame, format("Tracking mode: f=%.2f/s|", detector.queryProcessingFreq()), Point(10, 470), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 2);
		}

		t = cv::getTickCount();
		if (tPrevious) {
			tt = (double)(t - tPrevious) / cv::getTickFrequency();
			fps = 1 / tt;
			putText(frame, format("FPS: %.1f", fps), Point(480, 470), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255), 2, 8);
		}
		tPrevious = t;

		imshow("Camera implementation", frame);

		char key = waitKey(1);
		if (key == 27 || key == 'q')	break;
		else if (key == 's') {
			if (detector.IsTrackingMode()) {
				cout << "Stop tracking!\n";
				detector.changeMode();
			}
		}
		else if (key >= '1' && key <= '9') {
			if (detector.selectRoi(key - '0'))	cout << "Successfully initialize tracker!\n";
			else								cout << "Fail to initialize tracker!\n";
		}

	}

	cap.release();	//release the device

	destroyAllWindows();

	return 0;
}