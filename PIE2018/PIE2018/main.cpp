// PIE2018.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	String imageName("../Image/beach.jpg"); // by default
	/*if (argc > 1)
	{
		imageName = argv[1];
	}*/
	Mat image;
	image = imread(imageName, IMREAD_COLOR); // Read the file
	if (image.empty())						 // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}
	namedWindow("Display window", 0);		  // Create a window for display.
	resizeWindow("Display window", 800, 600); // Resize the window.
	imshow("Display window", image);		  // Show our image inside it.
	waitKey(0);								  // Wait for a keystroke in the window
	return 0;
}