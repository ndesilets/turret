#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/tracking.hpp>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>

using namespace cv;
using namespace std;

void processVideo2();
void processVideo();
void processStream();

int main(int argc, char *argv[]){
	namedWindow("Frame");
	namedWindow("FG Mask MOG 2");

	processVideo2();

	destroyAllWindows();

	return EXIT_SUCCESS;
}

void processVideo3(){
	
}

void processVideo2(){
	const char *fname = "VID_20170615_172809.mp4";
	Ptr<Tracker> tracker = Tracker::create("KCF");
	Ptr<BackgroundSubtractor> pMOG2;
	Mat frame;
	Mat procFrame;
	Mat fgMaskMOG2;

	VideoCapture capture(fname);

	pMOG2 = createBackgroundSubtractorMOG2(
		500,	// history
		16,		// threshold
		true	// detectShadows
	);

	// Define an initial bounding box
    Rect2d bbox(640, 0, 48, 48);

	// Read first frame
	if(!capture.read(frame)){
		cerr << "Unable to read next frame." << endl;
		exit(EXIT_FAILURE);
	}

	// Initialize tracker with first frame and bounding box
    tracker->init(frame, bbox);

	while(1){
		if(!capture.read(frame)){
			cerr << "Unable to read next frame." << endl;
			break;
		}

		procFrame = frame.clone();

		cvtColor(procFrame, procFrame, CV_BGR2GRAY);
		
		GaussianBlur(procFrame, procFrame, Size(15, 15), 0, 0);

		// Update background model
		pMOG2->apply(procFrame, fgMaskMOG2);

		// Update tracking results
        tracker->update(frame, bbox);

		// Draw bounding box
        rectangle(frame, bbox, Scalar(0, 0, 255), 2, 1);

		// Display result
        imshow("Raw", frame);
		imshow("Mask", fgMaskMOG2);

        waitKey(1);
	}
}

void processVideo(){
	Mat frame;
	Mat fgMaskMOG2;
	Ptr<BackgroundSubtractor> pMOG2;
	const char *fname = "VID_20170615_172809.mp4";
	char keyboard = 0;

	pMOG2 = createBackgroundSubtractorMOG2(
		500,	// history
		16,		// threshold
		true	// detectShadows
	);

	VideoCapture capture(fname);

	printf("capture\n");

	if(!capture.isOpened()){
		cerr << "Unable to open file." << endl;
		exit(EXIT_FAILURE);
	}

	while(1){
		stringstream ss;
		string frameNum;
		Mat procFrame;

		if(!capture.read(frame)){
			cerr << "Unable to read next frame." << endl;
			break;
		}

		cvtColor(frame, frame, CV_BGR2GRAY);

		GaussianBlur(frame, procFrame, Size(15, 15), 0, 0);

		// Update background model
		pMOG2->apply(procFrame, fgMaskMOG2);

		// Draw bounding box


		imshow("Raw", frame);
		//imshow("Frame", procFrame);
		imshow("FG Mask MOG 2", fgMaskMOG2);

		waitKey(1);
	}

	capture.release();
}

void processStream(){
    Mat frame;
    VideoCapture capture;
    Mat fgMaskMOG2;
    Ptr<BackgroundSubtractor> pMOG2;
    int keyboard;
    unsigned int frameNumber = 0;

    // Create bg subtractor objects
    pMOG2 = createBackgroundSubtractorMOG2(500, 8, true); //500, 16, true

	// Create capture object
	if(!capture.open(0)){ // Open webcam (default)
		cerr << "Unable to open cam" << endl;

		exit(EXIT_FAILURE);
	}

	// Read input data while not quit
	while((char)keyboard != 'q' && (char)keyboard != 27){
        char frameNumberString[10];

        // Write frame from capture into frame
		capture >> frame;
		if(frame.empty()){
			break;
		}

        frameNumber++;

		// Update the background model
		pMOG2->apply(frame, fgMaskMOG2);

		// Get frame number and write to frame
		rectangle(fgMaskMOG2, cv::Point(10, 2), cv::Point(100,20), cv::Scalar(255,255,255), -1);
        sprintf(frameNumberString, "%d", frameNumber);
		putText(fgMaskMOG2, frameNumberString, cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0));

		// Show current frame and the fg masks
		imshow("Frame", frame);
		imshow("FG Mask MOG 2", fgMaskMOG2);

		keyboard = waitKey(30);
	}

	// Delete the capture object
	capture.release();
}
