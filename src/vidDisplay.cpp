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
	Mat frame, filtered_frame, threshed_frame, gray_frame, kernel, labeled_frame;
	Mat labels, centroids, img_color, stats, regionImg;
	Point center;

	// colors for each region
	int regions = 10;
	vector<Vec3b> colors(regions);
	for (int label = 0; label < regions; ++label){
		colors[label] = Vec3b( rand()&255, rand()&255, rand()&255 );
	}

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
		// imshow("Video thresholded ", threshed_frame);

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

		vector<Rect> rectComponent;
		for (int label = 0; label < no_of_regions ; label++)
		{
			// draw rectangles around connected regions
			Rect r(Rect(Point(stats.at<int>(label, CC_STAT_LEFT ),
							stats.at<int>(label ,CC_STAT_TOP)),
							Size(stats.at<int>(label ,CC_STAT_WIDTH ),
							stats.at<int>(label ,CC_STAT_HEIGHT))));
			rectComponent.push_back(r);
			rectangle(frame, r, colors[label], 1);

			// computes a set of features (moments) for a specified region given a region map and a region ID
			// Calculate central Moments for each region processed threshold image
			regionImg = (labeled_frame == label);
			Moments my_moments = moments(regionImg, true);
			// find and draw the center
			if (my_moments.m00 != 0) {\
        center.x = my_moments.m10 / my_moments.m00;
        center.y = my_moments.m01 / my_moments.m00;
        circle(frame, center, 3, Scalar(0, 255, 0), -1);
    	}
			
			// Calculate Hu Moments
			double huMoments[7];
			HuMoments(my_moments, huMoments);

			// Log scale hu moments so that the scaling makes sense
			for(int i = 0; i < 7; i++)
			{
				huMoments[i] = -1 * copysign(1.0, huMoments[i]) * log10(abs(huMoments[i]));  
				printf("hu[%d] = %f\n", i, huMoments[i]);
			}
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

			case 't':
				quit = 1;
				break;
			// capture a photo if the user hits p
			case 'p': 
			{
				sprintf(buffer, "../data/%s%03d.png", label, frameid++);
				imwrite(buffer, frame , pars);
				printf("Image written: %s\n", buffer);

				break;
			}


			default:
				break;
		}

	} // end for

	// terminate the video capture
	printf("Terminating\n");
	delete capdev;

	return(0);
}
