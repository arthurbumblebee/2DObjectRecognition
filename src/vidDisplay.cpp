/*
	Arthur and Shafat
	S19
	Simple example of video capture and manipulation
	Based on OpenCV tutorials

	Compile command (macos)
	clang++ -o vid -I /opt/local/include vidDisplay.cpp -L /opt/local/lib -lopencv_core -lopencv_highgui -lopencv_video -lopencv_videoio

	use the makefiles provided
	make vid
*/

#include <cstdio>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {
	VideoCapture *capdev;
	char label[256];
	int quit = 0;
	int frameid = 0;
	char buffer[256];
	vector<int> pars;

	pars.push_back(5);

	if( argc < 2 ) {
	    printf("Usage: %s <label>\n", argv[0]);
	    exit(-1);
	}

	// open the video device
	capdev = new VideoCapture(0);
	if( !capdev->isOpened() ) {
		printf("Unable to open video device\n");
		return(-1);
	}

	// store the label
	strcpy(label, argv[1]);

	Size refS( (int) capdev->get(CAP_PROP_FRAME_WIDTH ),
		       (int) capdev->get(CAP_PROP_FRAME_HEIGHT));

	printf("Expected size: %d %d\n", refS.width, refS.height);

	namedWindow("Video", 1); // identifies a window?

	// matrices to hold multiple image outputs, etc
	Mat frame, filtered_frame, threshed_frame, gray_frame, kernel, labeled_frame, labels, centroids, img_color, stats;


	for(;!quit;) {
		
		// *capdev >> frame; // get a new frame from the camera, treat as a stream
		frame = imread("../data/training/paddle.000.png");

		if( frame.empty() ) {
		  printf("frame is empty\n");
		  break;
		}

		// fix edges and reduce noise by using median blur
		medianBlur(frame, filtered_frame, 7);
		// imshow("Video median", frame);

		// Reduce noise by blurring with a Gaussian filter ( kernel size = 5 )
		GaussianBlur( filtered_frame, filtered_frame, Size(5, 5), 0, 0, BORDER_DEFAULT );
		// imshow("Video filtered ", filtered_frame);

		// convert the frame to grayscale for thresholding
		cvtColor(filtered_frame, gray_frame, COLOR_BGR2GRAY);
			
		// thresh using otsu's binarization to separate image from background
		threshold(gray_frame, threshed_frame, 0, 255, THRESH_BINARY+THRESH_OTSU);
		
		// morphological processing, closing and then opening to remove spurious regions and fill in holes in objects
		// use a kernel size of 7
		int morph_size = 7;
		kernel = getStructuringElement( MORPH_OPEN, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
		morphologyEx(threshed_frame, threshed_frame, MORPH_CLOSE, kernel, Point(-1,-1), 1);
		morphologyEx(threshed_frame, threshed_frame, MORPH_OPEN, kernel, Point(-1,-1), 1);

		// run a connected components analysis on image
		// int no_of_regions = connectedComponentsWithStats(threshed_frame, labeled_frame, 4, CV_16U);
		int no_of_regions = connectedComponentsWithStats(threshed_frame, labeled_frame, stats, centroids, 4, CV_16U);
		normalize(labeled_frame, labeled_frame, 0, 255, NORM_MINMAX, CV_8U);

		printf("number of regions : %d\n", no_of_regions);

		// colors for each region
		vector<Vec3b> colors(no_of_regions);
		colors[0] = Vec3b(0,0,0); // bg
		for (int label = 1; label < no_of_regions; ++label){
			colors[label] = Vec3b( rand()&255, rand()&255, rand()&255 );
		}

		// draw rectangles around connected regions
		vector<Rect> rectComponent;
		for (int i = 0;i < no_of_regions ;i++)
		{
			Rect r(Rect(Point(stats.at<int>(i,CC_STAT_LEFT ),
							stats.at<int>(i,CC_STAT_TOP)),
							Size(stats.at<int>(i,CC_STAT_WIDTH ),
							stats.at<int>(i,CC_STAT_HEIGHT))));
			rectComponent.push_back(r);
			rectangle(frame,r,colors[i],1);

		}

		// show the processed output
		// imshow("threshed", labeled_frame);

		// display the video frame in the window
		imshow("Video", frame);

		// respond to keypresses
		int key = waitKey(10);
		switch(key) {
			// q quits the program
			case 'q':
				quit = 1;
				break;

			// capture a photo if the user hits c
			case 'c': 
				sprintf(buffer, "%s.%03d.png", label, frameid++);
				imwrite(buffer, frame, pars);
				printf("Image written: %s\n", buffer);
				break;

			default:
				break;
		}

	} // end for

	// terminate the video capture
	printf("Terminating\n");
	delete capdev;

	return(0);
}
