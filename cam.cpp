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
#include <chrono>
#include <cmath>

//using namespace cv;
using namespace std;
using namespace std::chrono;

void preProcessFrame();
void processVideo5();
/*void processVideo4();
void processVideo3();
void processVideo2();
void processVideo();
void processStream();
void splitFrame();*/

int main(int argc, char *argv[]){
	//processVideo4();

	processVideo5();

	cv::destroyAllWindows();

	return EXIT_SUCCESS;
}

void preProcessFrame(cv::Mat *frame){
	//cv::cvtColor(*frame, *frame, CV_RGB2GRAY);
	//cv::resize(*frame, *frame, cv::Size(640, 360));
	cv::blur(*frame, *frame, cv::Size(8, 8));
	//frame->convertTo(10, 1, CV_8UC1, 10);
}

void processVideo5(){
	const char *fname = "VID_20170629_214429.mp4";
	//cv::Ptr<cv::Tracker> tracker = cv::Tracker::create("KCF");
	cv::Ptr<cv::BackgroundSubtractor> pMOG2;
	cv::Mat frame;
	cv::Mat pFrame;
	cv::Mat fgMaskMOG2;
	//Rect2d target;
	char keyboard;

	pMOG2 = cv::createBackgroundSubtractorKNN(
		64,	// history (500)
		64,		// threshold (16)
		true	// detectShadows (true)
	);

	cv::VideoCapture capture(fname);

	// --- Timing stuff

	milliseconds t0 = duration_cast<milliseconds>(
    system_clock::now().time_since_epoch());

	// --- Main loop

	while(1){
		const int frameNum = capture.get(cv::CAP_PROP_POS_FRAMES);
		cv::Mat procFrame;

		if(!capture.read(frame)){
			cerr << "Unable to read next frame." << endl;
			return;
		}

		// --- Get and process frame

		procFrame  = frame.clone();

		preProcessFrame(&procFrame);

		// --- Motion detection

		pMOG2->apply(procFrame, fgMaskMOG2);
		threshold(fgMaskMOG2, fgMaskMOG2, 128, 255, CV_THRESH_BINARY);	
		dilate(fgMaskMOG2, fgMaskMOG2, cv::MORPH_RECT);	

		// --- Find bounding boxes 

		// Find contours

		vector<vector<cv::Point>> contours;
		vector<cv::Vec4i> hierarchy;
		cv::findContours(fgMaskMOG2, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		// Group contours

		cv::Mat structuringElement = cv::getStructuringElement(cv::MORPH_ELLIPSE	, cv::Size(32, 32));
		cv::morphologyEx(fgMaskMOG2, fgMaskMOG2, cv::MORPH_CLOSE, structuringElement);

		// Find bounding box for each polygon (from contours)

		vector<vector<cv::Point>> contours_poly(contours.size());
  		vector<cv::Rect> boundRect(contours.size());
  		vector<cv::Point2f> center(contours.size());
		vector<float> radius(contours.size());

		for(int i = 0; i < contours.size(); i++){
			cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect(cv::Mat(contours_poly[i]));
			cv::minEnclosingCircle(contours_poly[i], center[i], radius[i]);
		}

		cv::Mat drawing = cv::Mat::zeros(procFrame.size(), CV_8UC3);

		for(int i = 0; i < contours.size(); i++){
			cv::Scalar color = cv::Scalar(0, 255, 0);
			cv::Scalar color2 = cv::Scalar(0, 0, 255);
			cv::drawContours(drawing, contours_poly, (int)i, color, 1, 8, vector<cv::Vec4i>(), 0, cv::Point());

			// Set min size threshold

			cv::Point tl = boundRect[i].tl();
			cv::Point br = boundRect[i].br();

			int xDst = abs(br.x - tl.x);
			int yDst = abs(tl.y - br.y);

			if(xDst >= 12 && yDst >= 12){
				cv::rectangle(frame, tl, br, color2, 2, 8, 0);
				//cv::circle(frame, center[i], (int)radius[i], color, 2, 8, 0);
			}
  		}

		// --- Show frame 

		cv::imshow("raw", frame);
		//cv::imshow("processed", procFrame);
		cv::imshow("mask", fgMaskMOG2);
		cv::imshow("drawing", drawing);

		// --- Timing stuff

		if(frameNum % 30 == 0){
			milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			printf("%d\n", (now - t0) / 30);
			t0 = now;
		}

		cv::waitKey(10);
	}
}

/******************************************************************************
 * RETARDED
 *****************************************************************************

// Tracking attempt 2
void processVideo4(){
	const char *fname = "VID_20170615_172809.mp4";
	Ptr<Tracker> tracker = Tracker::create("KCF");
	Ptr<BackgroundSubtractor> pMOG2;
	Mat frame;
	Mat pFrame;
	Mat fgMaskMOG2;
	//Rect2d target;
	char keyboard;

	VideoCapture capture(fname);

	pMOG2 = createBackgroundSubtractorMOG2(
		500,	// history
		16,		// threshold
		true	// detectShadows
	);

	// Init
	if(!capture.read(frame)){
		cerr << "Unable to read next frame." << endl;
		return;
	}
	//resize(frame, frame, Size(640, 360));	

	//target = selectROI("tracker", frame);
	Rect2d target(64, 64, 16, 16);
	if(target.width == 0 || target.width == 0){
		return;
	}

	tracker->init(frame, target);

	while(1){
		if(!capture.read(frame)){
			cerr << "Unable to read next frame." << endl;
			return;
		}

		pFrame = frame.clone();
		cvtColor(pFrame, pFrame, CV_BGR2GRAY);
		GaussianBlur(pFrame, pFrame, Size(15, 15), 0, 0);

		pMOG2->apply(pFrame, fgMaskMOG2);
		threshold(fgMaskMOG2, fgMaskMOG2, 204, 255, CV_THRESH_BINARY);

		//splitFrame(fgMaskMOG2);
		
		//resize(frame, frame, Size(640, 360));

		tracker->update(frame, target);
		rectangle(frame, target, Scalar(255, 0, 0), 2, 1);

		imshow("raw", frame);
		imshow("mask", fgMaskMOG2);

		keyboard = waitKey(16);
		if((char)keyboard == 113){
			while((char)keyboard != 0x20){
				if(keyboard == 's'){
					imwrite("./cap.bmp", fgMaskMOG2);
				}
				keyboard = waitKey(16);
			}
		}
	}
}

// idk
void processVideo3(){
	const char *fname = "VID_20170615_172809.mp4";
	Ptr<BackgroundSubtractor> pMOG2;
	Mat frame;
	Mat procFrame;
	Mat maskCopy;
	Mat fgMaskMOG2;
	Rect2d target;

	VideoCapture capture(fname);

	pMOG2 = createBackgroundSubtractorMOG2(
		500,	// history
		16,		// threshold
		true	// detectShadows
	);

	while(1){
		if(!capture.read(frame)){
			cerr << "Unable to read next frame." << endl;
			break;
		}

		procFrame = frame.clone();

		// Grayscale + blur to filter noise
		cvtColor(procFrame, procFrame, CV_BGR2GRAY);
		GaussianBlur(procFrame, procFrame, Size(15, 15), 0, 0);

		// Update background model
		pMOG2->apply(procFrame, fgMaskMOG2);
		threshold(fgMaskMOG2, fgMaskMOG2, 128, 255, CV_THRESH_BINARY);

		// Display result
        imshow("Raw", frame);
		imshow("Proc", procFrame);
		imshow("Mask", fgMaskMOG2);

        waitKey(1);
	}
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

void splitFrame(){
	static const int _H = 0, _W = 1, BLK_SIZE = 16;
	int bc_w = 0, bc_h = 0; 

	Mat frame = imread("./cap.bmp");
	MatSize dimens = frame.size;

	bc_w = dimens[_W] / BLK_SIZE; // use truncation as floor
	bc_h = dimens[_H] / BLK_SIZE;

	printf("bc_w: %d, bc_h: %d\n\n", bc_w, bc_h);

	for(int i = 0; i < bc_w; i++){
		int left_edge = i * BLK_SIZE;
		printf("%d > ", left_edge);
	}

	printf("\n\n");

	for(int i = 0; i < bc_h; i++){
		int top_edge = i * BLK_SIZE;
		printf("%d > ", top_edge);
	}

	for(int i = 0; i < bc_h; i++){
		for(int j = 0; j < bc_w; j++){
			int r = j * BLK_SIZE;
			int c = i * BLK_SIZE;
			int r_offset = r + BLK_SIZE;
			int c_offset = r + BLK_SIZE;
			printf("TL: (%d, %d) - BR: (%d, %d)\n", r, c, r_offset, c_offset);
			rectangle(frame, Point(r, c), Point(r_offset, c_offset), Scalar(255, 0, 0), 1, 1);
		}
	}

	imshow("test", frame);
	waitKey(0);

	//Rect2d target(64, 64, 16, 16);
	//rectangle(frame, target, Scalar(255, 0, 0), 1, 1);

	printf("\n\n");

	printf("h: %d, w: %d\n", dimens[_H], dimens[_W]);
}																		      */