// OpenCV
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
// C
#include <stdio.h>
// C++
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

// Globals
Mat frame;
Mat fgMaskMOG2;
Ptr<BackgroundSubtractor> pMOG2;
int keyboard;

// Fn prototypes
void processVideo();

int main(int argc, char *argv[]){
	// Create GUI windows
	namedWindow("Frame");
	namedWindow("FG Mask MOG 2");

	// Create bg subtractor objects
	pMOG2 = createBackgroundSubtractorMOG2();

	// Start processing video
	processVideo();

	// Cleanup
	destroyAllWindows();

	return EXIT_SUCCESS;
}

void processVideo(){
	// Create capture object
	VideoCapture capture;
	if(!capture.open(0)){ // Open webcam (default)
		cerr 
		<< "Unable to open cam"
		<< endl;

		exit(EXIT_FAILURE);
	}

	// Read input data while not quit
	while((char)keyboard != 'q' && (char)keyboard != 27){
		capture >> frame;

		if(frame.empty()){
			break;
		}

		// Update the background model
		pMOG2->apply(frame, fgMaskMOG2);

		// Get frame number and write to frame
		stringstream ss;
		rectangle(frame, cv::Point(10, 2), cv::Point(100,20), cv::Scalar(255,255,255), -1);
		//ss << capture.get(CAP_PROP_POS_FRAMES);
		//string frameNumberString = ss.str();
		putText(frame, "test", cv::Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0));
		
		// Show current frame and the fg masks
		imshow("Frame", frame);
		imshow("FG Mask MOG 2", fgMaskMOG2);

		keyboard = waitKey(30);
	}

	// Delete the capture object
	capture.release();
}
