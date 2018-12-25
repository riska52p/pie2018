// PIE2018.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>
#include <Eigen/Dense>
#include <Eigen/Sparse>

using namespace cv;
using namespace std;
using namespace Eigen;

Rect rect, rec1, roi1;
Mat image, src, target, mask, roi;
bool mousedown;
vector<vector<Point> > contours;
vector<Point> pts;
Point minp(1000, 1000);

Mat poi(Mat s, Mat t, Mat ma)
{
	Mat tmp(s.size(), CV_8UC3);

	SparseMatrix<double, RowMajor> A(tmp.cols * tmp.rows, tmp.cols * tmp.rows);
	SimplicialCholesky<SparseMatrix<double>> solver;
	MatrixXd b = MatrixXd::Zero(tmp.cols * tmp.rows, 3);
	MatrixXd x(tmp.cols * tmp.rows, 3);

	for (int i = 0; i < tmp.rows; i++)
	{
		for (int j = 0; j < tmp.cols; j++)
		{
			Vector3d p;
			//unknown
			if (ma.at<uchar>(i, j) == 255)
			{
				p << t.at<Vec3b>(i, j)[0], t.at<Vec3b>(i, j)[1], t.at<Vec3b>(i, j)[2];
				int n = 4;
				//up
				if (i == 0)
					n--;
				else
				{
					A.coeffRef(i*tmp.cols + j, (i - 1)*tmp.cols + j) = -1;
					Vector3d q;
					/*if (ma.at<uchar>(i - 1, j) == 0)
					{
						q << s.at<Vec3b>(i - 1, j)[0], s.at<Vec3b>(i - 1, j)[1], s.at<Vec3b>(i - 1, j)[2];
						b.row(i*tmp.cols + j) += q;
					}*/
					q << t.at<Vec3b>(i - 1, j)[0], t.at<Vec3b>(i - 1, j)[1], t.at<Vec3b>(i - 1, j)[2];
					b.row(i*tmp.cols + j) += p - q;
				}
				//down
				if (i == tmp.rows-1)
					n--;
				else
				{
					A.coeffRef(i*tmp.cols + j, (i + 1)*tmp.cols + j) = -1;
					Vector3d q;
					/*if (ma.at<uchar>(i + 1, j) == 0)
					{
						q << s.at<Vec3b>(i + 1, j)[0], s.at<Vec3b>(i + 1, j)[1], s.at<Vec3b>(i + 1, j)[2];
						b.row(i*tmp.cols + j) += q;
					}*/
					q << t.at<Vec3b>(i + 1, j)[0], t.at<Vec3b>(i + 1, j)[1], t.at<Vec3b>(i + 1, j)[2];
					b.row(i*tmp.cols + j) += p - q;
				}
				//left
				if (j == 0)
					n--;
				else
				{
					A.coeffRef(i*tmp.cols + j, i*tmp.cols + j - 1) = -1;
					Vector3d q;
					/*if (ma.at<uchar>(i, j - 1) == 0)
					{
						q << s.at<Vec3b>(i, j - 1)[0], s.at<Vec3b>(i, j - 1)[1], s.at<Vec3b>(i, j - 1)[2];
						b.row(i*tmp.cols + j) += q;
					}*/
					q << t.at<Vec3b>(i, j - 1)[0], t.at<Vec3b>(i, j - 1)[1], t.at<Vec3b>(i, j - 1)[2];
					b.row(i*tmp.cols + j) += p - q;
				}
				//right
				if (j == tmp.cols - 1)
					n--;
				else
				{
					A.coeffRef(i*tmp.cols + j, i*tmp.cols + j + 1) = -1;
					Vector3d q;
					/*if (ma.at<uchar>(i, j + 1) == 0)
					{
						q << s.at<Vec3b>(i, j + 1)[0], s.at<Vec3b>(i, j + 1)[1], s.at<Vec3b>(i, j + 1)[2];
						b.row(i*tmp.cols + j) += q;
					}*/
					q << t.at<Vec3b>(i, j + 1)[0], t.at<Vec3b>(i, j + 1)[1], t.at<Vec3b>(i, j + 1)[2];
					b.row(i*tmp.cols + j) += p - q;
				}
				//center
				A.coeffRef(i*tmp.cols + j, i*tmp.cols + j) = n;
			}
			else
			{
				A.coeffRef(i*tmp.cols + j, i*tmp.cols + j) = 1;
				p << s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2];
				b.row(i*tmp.cols + j) = p;
			}

		}
	}

	solver.compute(A.transpose() * A);
	x = solver.solve(A.transpose() * b);

	for (int i = 0; i < tmp.rows; i++)
	{
		for (int j = 0; j < tmp.cols; j++)
		{
			tmp.at<Vec3b>(i, j)[0] = x(i * tmp.cols + j, 0);
			tmp.at<Vec3b>(i, j)[1] = x(i * tmp.cols + j, 1);
			tmp.at<Vec3b>(i, j)[2] = x(i * tmp.cols + j, 2);
		}
	}
	return tmp;
}

void onMouse(int event, int x, int y, int flags, void* userdata)
{
	Mat img = *((Mat *)userdata);

	if (event == EVENT_LBUTTONDOWN)
	{
		mousedown = true;
		pts.clear();
	}

	if (event == EVENT_LBUTTONUP)
	{
		mousedown = false;
		if (pts.size() > 2)
		{
			mask = Mat::zeros(target.size(), CV_8UC1);
			contours.push_back(pts);
			drawContours(mask, contours, 0, Scalar(255), -1);
		}
		else
		{
			Mat mask2 = src.clone();
			Mat tar = target.clone();
			Mat mas = mask2(Rect(x-minp.x, y-minp.y, target.cols, target.rows));
			Mat re = poi(mas, tar, mask);
			re.copyTo(mas);
			imshow("final", mask2);
		}
	}

	if (mousedown)
	{
		if (pts.size() > 2)
		{
			line(img, Point(x, y), pts[pts.size() - 1], Scalar(0));
			//offset
			minp.x = MIN(minp.x, x);
			minp.y = MIN(minp.y, y);
			imshow("Mask", img);
		}
			
		pts.push_back(Point(x, y));
	}
}

int main(int argc, char** argv)
{
	String imageName("../Image/bg.jpg"); // by default
	String image1Name("../Image/bird.jpg"); // by default

	src = imread(imageName); // Read the file
	target = imread(image1Name); // Read the file
	roi = target.clone();
	Mat a = src.clone();
	//roi1 = Rect(0, 0, target.cols, target.rows);
	//image = imread(imageName, IMREAD_COLOR); // Read the file
	if (src.empty() || target.empty())		 // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}
	namedWindow("Mask");		  // Create a window for display.
	imshow("Mask", roi);		  // Show our image inside it.
	setMouseCallback("Mask", onMouse, &roi);

	namedWindow("src");		  // Create a window for display.
	imshow("src", src);		  // Show our image inside it.
	setMouseCallback("src", onMouse, &a);
	
	waitKey(0);				// Wait for a keystroke in the window
	return 0;
}