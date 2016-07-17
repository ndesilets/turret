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
#include <string>
#include <stdlib.h>

using namespace cv;
using namespace std;

// Fn prototypes
void processVideo();

int main(int argc, char *argv[]){
	// Create GUI windows
	namedWindow("Frame");
	namedWindow("FG Mask MOG 2");

	// Start processing video
	processVideo();

	// Cleanup
	destroyAllWindows();

	return EXIT_SUCCESS;
}

void processVideo(){
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
