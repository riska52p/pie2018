﻿#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/photo.hpp>
#include <iostream>
#include <cstdlib>
#include <Eigen/Dense>
#include <Eigen/Sparse>

using namespace cv;
using namespace std;
using namespace Eigen;

bool mousedown, ismix = false;
Mat mask, target, roi, src;
vector<vector<Point>> contours;
vector<Point> pts;
Point minp, maxp;

//State
enum state { SEAMLESS_CLONING, MIXED_CLONING, TEXTURE_FLATTENING, ILLUMINATION, COLOR, SEAMLESS_TILING } e_state;

//Canny Edge Detection for Texture Flattening
static Mat EdgeDetector(int, void*)
{
	int lowThreshold = 25; //make it can be changed if have time
						   //const int max_lowThreshold = 100;
	const int ratio = 3;
	const int kernel_size = 3;
	Mat srcGray, edgeMask, dst;
	dst.create(src.size(), src.type());
	cvtColor(src, srcGray, COLOR_BGR2GRAY);
	blur(srcGray, edgeMask, Size(3, 3));
	Canny(edgeMask, edgeMask, lowThreshold, lowThreshold * ratio, kernel_size);

	dst = Scalar::all(0);
	src.copyTo(dst, edgeMask);
	imshow("Edge Mask", edgeMask);
	//imwrite("../Image/Canny_Edge.jpg", edgeMask);
	return edgeMask;
}

//Gradient Norm Claculation for Illumination
Mat GradientNorm(Mat img)
{
	//double result = 0.2;
	//average gradient norm of f* over O
	Mat imgGray, gradX, gradY, gradNorm;
	GaussianBlur(img, img, Size(3, 3), 0, 0, BORDER_DEFAULT);
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	// Gradient X
	Sobel(imgGray, gradX, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	// Gradient Y
	Sobel(imgGray, gradY, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
	gradNorm = abs(gradX) + abs(gradY);

	return gradNorm;
}

Mat poisson(Mat s, Mat t, Mat ma)
{
	Mat out(s.size(), CV_8UC3);
	// Sparse matrix
	SparseMatrix<double, RowMajor> A(out.cols * out.rows, out.cols * out.rows);
	SimplicialCholesky<SparseMatrix<double>> solver;
	MatrixXd b = MatrixXd::Zero(out.cols * out.rows, 3);
	MatrixXd x(out.cols * out.rows, 3);

	for (int i = 0; i < out.rows; i++)
	{
		for (int j = 0; j < out.cols; j++)
		{
			Vector3d p, gn, alpha, ab;
			double beta;
			Mat gradNorm;
			// Unknown pixels
			if (ma.at<uchar>(i, j) == 255)
			{
				p << t.at<Vec3b>(i, j)[0], t.at<Vec3b>(i, j)[1], t.at<Vec3b>(i, j)[2];
				int n = 4;
				//Calculation of illumination gradient
				if (e_state == ILLUMINATION)
				{
					gradNorm = GradientNorm(t);
					gn = Vector3d(gradNorm.at<Vec3b>(i, j)[0], gradNorm.at<Vec3b>(i, j)[1], gradNorm.at<Vec3b>(i, j)[2]);
					alpha = 0.2 * gn;
					beta = 0.2;
					ab = Vector3d(pow(alpha[0], beta), pow(alpha[1], beta), pow(alpha[2], beta));
				}
				// Up
				if (i == 0)
					n--;
				else
				{
					A.coeffRef(i*out.cols + j, (i - 1)*out.cols + j) = -1;
					Vector3d q(t.at<Vec3b>(i - 1, j)[0], t.at<Vec3b>(i - 1, j)[1], t.at<Vec3b>(i - 1, j)[2]);
					Vector3d p1, q1, pq, g, p1q1;
					p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
					q1 = Vector3d(s.at<Vec3b>(i - 1, j)[0], s.at<Vec3b>(i - 1, j)[1], s.at<Vec3b>(i - 1, j)[2]);
					double f, f1;
					switch (e_state)
					{
					case SEAMLESS_CLONING:
						b.row(i*out.cols + j) += p - q;
						break;
					case MIXED_CLONING:
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							b.row(i*out.cols + j) += p1 - q1;
						else b.row(i*out.cols + j) += p - q;
						break;
					case TEXTURE_FLATTENING:
						b.row(i*out.cols + j) += Vector3d(0.0, 0.0, 0.0);
						break;
					case ILLUMINATION:
						p1q1 = p1 - q1;
						pq = p1q1.cwiseAbs();
						pq = Vector3d(pow(pq[0] + 0.0001, -beta), pow(pq[1] + 0.0001, -beta), pow(pq[2] + 0.0001, -beta));
						g = Vector3d(ab[0] * pq[0] * p1q1[0], ab[1] * pq[1] * p1q1[1], ab[2] * pq[2] * p1q1[2]);
						b.row(i*out.cols + j) += g;
						break;
					case COLOR:
						p = Vector3d(t.at<Vec3b>(i, j)[0] * 0.5, t.at<Vec3b>(i, j)[1] * 0.5, t.at<Vec3b>(i, j)[2] * 2.5);
						q = Vector3d(t.at<Vec3b>(i - 1, j)[0] * 0.5, t.at<Vec3b>(i - 1, j)[1] * 0.5, t.at<Vec3b>(i - 1, j)[2] * 2.5);
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							pq = p1 - q1;
						else pq = (p - q);
						b.row(i*out.cols + j) += pq;
						break;
					}
				}
				// Down
				if (i == out.rows - 1)
					n--;
				else
				{
					A.coeffRef(i*out.cols + j, (i + 1)*out.cols + j) = -1;
					Vector3d q(t.at<Vec3b>(i + 1, j)[0], t.at<Vec3b>(i + 1, j)[1], t.at<Vec3b>(i + 1, j)[2]);
					Vector3d p1, q1, pq, g, p1q1;;
					p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
					q1 = Vector3d(s.at<Vec3b>(i + 1, j)[0], s.at<Vec3b>(i + 1, j)[1], s.at<Vec3b>(i + 1, j)[2]);
					double f, f1;
					switch (e_state)
					{
					case SEAMLESS_CLONING:
						b.row(i*out.cols + j) += p - q;
						break;
					case MIXED_CLONING:
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							b.row(i*out.cols + j) += p1 - q1;
						else b.row(i*out.cols + j) += p - q;
						break;
					case TEXTURE_FLATTENING:
						b.row(i*out.cols + j) += Vector3d(0.0, 0.0, 0.0);
						break;
					case ILLUMINATION:
						p1q1 = p1 - q1;
						pq = p1q1.cwiseAbs();
						pq = Vector3d(pow(pq[0] + 0.0001, -beta), pow(pq[1] + 0.0001, -beta), pow(pq[2] + 0.0001, -beta));
						g = Vector3d(ab[0] * pq[0] * p1q1[0], ab[1] * pq[1] * p1q1[1], ab[2] * pq[2] * p1q1[2]);
						b.row(i*out.cols + j) += g;
						break;
					case COLOR:
						p = Vector3d(t.at<Vec3b>(i, j)[0] * 0.5, t.at<Vec3b>(i, j)[1] * 0.5, t.at<Vec3b>(i, j)[2] * 2.5);
						q = Vector3d(t.at<Vec3b>(i + 1, j)[0] * 0.5, t.at<Vec3b>(i + 1, j)[1] * 0.5, t.at<Vec3b>(i + 1, j)[2] * 2.5);
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							pq = p1 - q1;
						else pq = p - q;
						b.row(i*out.cols + j) += pq;
						break;
					}
				}
				// Left
				if (j == 0)
					n--;
				else
				{
					A.coeffRef(i*out.cols + j, i*out.cols + j - 1) = -1;
					Vector3d q(t.at<Vec3b>(i, j - 1)[0], t.at<Vec3b>(i, j - 1)[1], t.at<Vec3b>(i, j - 1)[2]);
					Vector3d p1, q1, pq, g, p1q1;
					p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
					q1 = Vector3d(s.at<Vec3b>(i, j - 1)[0], s.at<Vec3b>(i, j - 1)[1], s.at<Vec3b>(i, j - 1)[2]);
					double f, f1;
					switch (e_state)
					{
					case SEAMLESS_CLONING:
						b.row(i*out.cols + j) += p - q;
						break;
					case MIXED_CLONING:
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							b.row(i*out.cols + j) += p1 - q1;
						else b.row(i*out.cols + j) += p - q;
						break;
					case TEXTURE_FLATTENING:
						b.row(i*out.cols + j) += Vector3d(0.0, 0.0, 0.0);
						break;
					case ILLUMINATION:
						p1q1 = p1 - q1;
						pq = p1q1.cwiseAbs();
						pq = Vector3d(pow(pq[0] + 0.0001, -beta), pow(pq[1] + 0.0001, -beta), pow(pq[2] + 0.0001, -beta));
						g = Vector3d(ab[0] * pq[0] * p1q1[0], ab[1] * pq[1] * p1q1[1], ab[2] * pq[2] * p1q1[2]);
						b.row(i*out.cols + j) += g;
						break;
					case COLOR:
						p = Vector3d(t.at<Vec3b>(i, j)[0] * 0.5, t.at<Vec3b>(i, j)[1] * 0.5, t.at<Vec3b>(i, j)[2] * 2.5);
						q = Vector3d(t.at<Vec3b>(i, j - 1)[0] * 0.5, t.at<Vec3b>(i, j - 1)[1] * 0.5, t.at<Vec3b>(i, j - 1)[2] * 2.5);
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							pq = p1 - q1;
						else pq = p - q;
						b.row(i*out.cols + j) += pq;
						break;
					}
				}
				// Right
				if (j == out.cols - 1)
					n--;
				else
				{
					A.coeffRef(i*out.cols + j, i*out.cols + j + 1) = -1;
					Vector3d q(t.at<Vec3b>(i, j + 1)[0], t.at<Vec3b>(i, j + 1)[1], t.at<Vec3b>(i, j + 1)[2]);
					Vector3d p1, q1, pq, g, p1q1;
					p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
					q1 = Vector3d(s.at<Vec3b>(i, j + 1)[0], s.at<Vec3b>(i, j + 1)[1], s.at<Vec3b>(i, j + 1)[2]);
					double f, f1;
					switch (e_state)
					{
					case SEAMLESS_CLONING:
						b.row(i*out.cols + j) += p - q;
						break;
					case MIXED_CLONING:
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							b.row(i*out.cols + j) += p1 - q1;
						else b.row(i*out.cols + j) += p - q;
						break;
					case TEXTURE_FLATTENING:
						b.row(i*out.cols + j) += Vector3d(0.0, 0.0, 0.0);
						break;
					case ILLUMINATION:
						p1q1 = p1 - q1;
						pq = p1q1.cwiseAbs();
						pq = Vector3d(pow(pq[0] + 0.0001, -beta), pow(pq[1] + 0.0001, -beta), pow(pq[2] + 0.0001, -beta));
						g = Vector3d(ab[0] * pq[0] * p1q1[0], ab[1] * pq[1] * p1q1[1], ab[2] * pq[2] * p1q1[2]);
						b.row(i*out.cols + j) += g;
						break;
					case COLOR:
						p = Vector3d(t.at<Vec3b>(i, j)[0] * 0.5, t.at<Vec3b>(i, j)[1] * 0.5, t.at<Vec3b>(i, j)[2] * 2.5);
						q = Vector3d(t.at<Vec3b>(i, j + 1)[0] * 0.5, t.at<Vec3b>(i, j + 1)[1] * 0.5, t.at<Vec3b>(i, j + 1)[2] * 2.5);
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							pq = p1 - q1;
						else pq = p - q;
						b.row(i*out.cols + j) += pq;
						break;
					}
				}
				// Center
				A.coeffRef(i*out.cols + j, i*out.cols + j) = n;
				//If Illumination
				if (e_state == ILLUMINATION)
					b.row(i*out.cols + j) /= 4;
			}
			// Known pixels
			else
			{
				int n;
				A.coeffRef(i*out.cols + j, i*out.cols + j) = 1;
				p << s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2];
				//Texture Flattening Only
				if (e_state == TEXTURE_FLATTENING)
				{
					n = 4;
					//up
					if (i == 0)
						n--;
					else
					{
						Vector3d q(t.at<Vec3b>(i - 1, j)[0], t.at<Vec3b>(i - 1, j)[1], t.at<Vec3b>(i - 1, j)[2]);
						b.row(i*out.cols + j) += q + p - q;
					}
					//down
					if (i == out.rows - 1)
						n--;
					else
					{
						Vector3d q(t.at<Vec3b>(i + 1, j)[0], t.at<Vec3b>(i + 1, j)[1], t.at<Vec3b>(i + 1, j)[2]);
						b.row(i*out.cols + j) += q + p - q;
					}
					//left
					if (j == 0)
						n--;
					else
					{
						Vector3d q(t.at<Vec3b>(i, j - 1)[0], t.at<Vec3b>(i, j - 1)[1], t.at<Vec3b>(i, j - 1)[2]);
						b.row(i*out.cols + j) += q + p - q;
					}
					//right
					if (j == out.cols - 1)
						n--;
					else
					{
						Vector3d q(t.at<Vec3b>(i, j + 1)[0], t.at<Vec3b>(i, j + 1)[1], t.at<Vec3b>(i, j + 1)[2]);
						b.row(i*out.cols + j) += q + p - q;
					}
					A.coeffRef(i*out.cols + j, i*out.cols + j) = n;
					//b.row(i*out.cols + j) /= 4;
				}
				else
					b.row(i*out.cols + j) = p;
			}
			if (e_state == SEAMLESS_TILING)
			{
				p = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
				//Up
				if (i < 10)
				{
					Vector3d q(s.at<Vec3b>(out.rows - i, j)[0], s.at<Vec3b>(out.rows - i, j)[1], s.at<Vec3b>(out.rows - i, j)[2]);
					b.row(i*out.cols + j) += (p + q) / 2;
				}
				//Down
				else if (i > out.rows - 10)
				{
					Vector3d q(s.at<Vec3b>(out.rows - i, j)[0], s.at<Vec3b>(out.rows - i, j)[1], s.at<Vec3b>(out.rows - i, j)[2]);
					b.row(i*out.cols + j) += (p + q) / 2;
				}
				else
					b.row(i*out.cols + j) += p;
				//Left
				if (j < 10)
				{
					Vector3d q(s.at<Vec3b>(i, out.cols - j)[0], s.at<Vec3b>(i, out.cols - j)[1], s.at<Vec3b>(i, out.cols - j)[2]);
					b.row(i*out.cols + j) += (p + q) / 2;
				}
				//Right
				else if (j > out.cols - 10)
				{
					Vector3d q(s.at<Vec3b>(i, out.cols - j)[0], s.at<Vec3b>(i, out.cols - j)[1], s.at<Vec3b>(i, out.cols - j)[2]);
					b.row(i*out.cols + j) += (p + q) / 2;
				}
				else
					//Center
					b.row(i*out.cols + j) += p;
				A.coeffRef(i*out.cols + j, i*out.cols + j) = 3;
			}
		}
	}
	// Solve poisson equation
	solver.compute(A.transpose() * A);
	x = solver.solve(A.transpose() * b);
	// Result
	for (int i = 0; i < out.rows; i++)
		for (int j = 0; j < out.cols; j++)
			for (int k = 0; k < 3; k++)
				// Deal with overflow
				out.at<Vec3b>(i, j)[k] = MAX(MIN(x(i * out.cols + j, k), 255), 0);


	return out;
}

void TextureFlattening()
{
	Mat dest = src.clone();
	Mat mask1 = EdgeDetector(0, 0);
	for (int i = 0; i < mask1.rows; i++)
		for (int j = 0; j < mask1.cols; j++)
		{
			if (mask1.at<uchar>(i, j) == 0)
				mask1.at<uchar>(i, j) = 255;
			else if (mask1.at<uchar>(i, j) == 255)
			{
				mask1.at<uchar>(i, j) = 0;
				for (int k = 0; k < 2; k++)
				{
					if (j > k && j < mask1.cols - k)
					{
						mask1.at<uchar>(i, j + k) = 0;
						mask1.at<uchar>(i, j - k) = 0;
					}
					if (i > k && i < mask1.rows - k)
					{
						mask1.at<uchar>(i + k, j) = 0;
						mask1.at<uchar>(i - k, j) = 0;
					}
				}
			}
		}
	Mat result;
	result = poisson(src, dest, mask1);
	//textureFlattening(dest, mask1, result, 40, 50, 3);
	imshow("Texture Flatten", result);
}

void SeamlessTiling()
{
	Mat img = src.clone();
	Mat dest = src.clone();
	//Mat result = src.clone();
	Mat result = poisson(img, dest, dest);

	Mat newImg;
	for (int i = 0; i < 2; i++)
	{
		Mat copyImg = result.clone();
		vconcat(result, copyImg, newImg);
		for (int j = 0; j < 2; j++)
		{
			Mat copyImg = newImg.clone();
			hconcat(newImg, copyImg, newImg);
		}
	}
	imshow("Seamless Tiling", newImg);
}

void onMouse(int event, int x, int y, int flags, void* userdata)
{
	Mat img = *((Mat *)userdata);

	if (event == EVENT_LBUTTONDOWN)
	{
		//reset
		mousedown = true;
		contours.clear();
		pts.clear();
		minp = Point(1000, 1000);
		maxp = Point(0, 0);
		roi = target.clone();
	}

	if (event == EVENT_RBUTTONDOWN)
	{
		if (e_state == SEAMLESS_CLONING || e_state == MIXED_CLONING)
		{
			// Seamless cloning
			Mat image = img.clone();
			Mat tar = target.clone();
			// Crop images
			Mat crops = image(Rect(x, y, MIN(mask.cols, img.cols - x), MIN(mask.rows, img.rows - y)));
			tar = tar(Rect(MAX(minp.x - 5, 0), MAX(minp.y - 5, 0), MIN(mask.cols, img.cols - x), MIN(mask.rows, img.rows - y)));
			Mat cropm = mask(Rect(0, 0, MIN(mask.cols, img.cols - x), MIN(mask.rows, img.rows - y)));
			// Solve poisson equation
			Mat result = poisson(crops, tar, cropm);
			result.copyTo(crops);
			imshow("Cloning", image);
		}
		if (e_state == ILLUMINATION)
		{
			Mat image = target.clone();
			Mat tar = target.clone();
			tar = tar(Rect(MAX(minp.x - 5, 0), MAX(minp.y - 5, 0), MIN(mask.cols, img.cols - x), MIN(mask.rows, img.rows - y)));
			//crop src the same size as mask, no boundary condition
			Mat crops = image(Rect(MAX(minp.x - 5, 0), MAX(minp.y - 5, 0), mask.cols, mask.rows));
			//no need to crop the mask again
			//Mat cropm = mask(Rect(0, 0, MIN(mask.cols, img.cols - x), MIN(mask.rows, img.rows - y)));
			// Solve poisson equation using original mask
			Mat result = poisson(crops, tar, mask);
			result.copyTo(crops);
			imshow("Illumination Change", image);
		}
		if (e_state == COLOR)
		{
			Mat image = target.clone();
			Mat tar = target.clone();
			tar = tar(Rect(MAX(minp.x - 5, 0), MAX(minp.y - 5, 0), MIN(mask.cols, img.cols - x), MIN(mask.rows, img.rows - y)));
			//crop src the same size as mask, no boundary condition
			Mat crops = image(Rect(MAX(minp.x - 5, 0), MAX(minp.y - 5, 0), mask.cols, mask.rows));
			//no need to crop the mask again
			//Mat cropm = mask(Rect(0, 0, MIN(mask.cols, img.cols - x), MIN(mask.rows, img.rows - y)));
			// Solve poisson equation using original mask
			Mat result = poisson(crops, tar, mask);
			result.copyTo(crops);
			imshow("Color Change", image);
		}
	}

	if (event == EVENT_LBUTTONUP)
	{
		mousedown = false;
		if (pts.size() > 2)
		{
			// Create mask
			mask = Mat::zeros(img.size(), CV_8UC1);
			contours.push_back(pts);
			drawContours(mask, contours, 0, 255, -1);
			// Pad bounding box by 5 pixels 
			mask = mask(Rect(MAX(minp.x - 5, 0), MAX(minp.y - 5, 0), MIN(maxp.x - minp.x + 10, mask.cols - minp.x), MIN(maxp.y - minp.y + 10, mask.rows - minp.y)));
		}
	}

	if (mousedown)
	{
		// Draw contour
		if (pts.size() > 2)
		{
			// Show contour
			line(img, Point(x, y), pts[pts.size() - 1], 0);
			// Bounding box
			minp.x = MAX(MIN(minp.x, x), 0);
			minp.y = MAX(MIN(minp.y, y), 0);
			maxp.x = MIN(MAX(maxp.x, x), img.cols);
			maxp.y = MIN(MAX(maxp.y, y), img.rows);
			imshow("roi", img);
		}

		pts.emplace_back(Point(x, y));
	}
}
//Guide to user
void PrintMenu()
{
	cout << "Main Menu" << endl;
	cout << "n - Seamless Cloning" << endl;
	cout << "m - Mixed Seamless Cloning" << endl;
	cout << "e - Texture Flattening" << endl;
	cout << "i - Local Illumination Change" << endl;
	cout << "c - Local Color Change" << endl;
	cout << "t - Seamless Tiling" << endl;
	cout << "Esc - Exit Application" << endl;
	cout << "What would you like to do?" << endl;
}

int main(int argc, char** argv)
{
	// Read the src file
	src = imread("../Image/beach.jpg");
	// Read the target file
	target = imread("../Image/bird.jpg");
	// Check for invalid input
	if (src.empty() || target.empty())
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}
	roi = target.clone();
	// Show target to select roi
	imshow("roi", roi);
	// Show background
	imshow("src", src);
	PrintMenu();
	while (1)
	{
		switch (waitKey(0))
		{
			// Press 'ESC' to end the program
		case 27:
			exit(0);
			break;
			// Mixed gradient
		case 'm':
			e_state = MIXED_CLONING;
			//Destroy All Windows
			cvDestroyAllWindows();
			// Read the src file
			src = imread("../Image/2.jpg");
			// Read the target file
			target = imread("../Image/bird.jpg");
			roi = target.clone();
			// Show target to select roi
			imshow("roi", roi);
			// Show background
			imshow("src", src);
			setMouseCallback("roi", onMouse, &roi);
			setMouseCallback("src", onMouse, &src);
			break;
			// No mixed gradient
		case 'n':
			e_state = SEAMLESS_CLONING;
			//Destroy All Windows
			cvDestroyAllWindows();
			// Read the src file
			src = imread("../Image/beach.jpg");
			// Read the target file
			target = imread("../Image/bird.jpg");
			roi = target.clone();
			// Show target to select roi
			imshow("roi", roi);
			// Show background
			imshow("src", src);
			setMouseCallback("roi", onMouse, &roi);
			setMouseCallback("src", onMouse, &src);
			break;
			// Edge Detector
		case 'e':
			e_state = TEXTURE_FLATTENING;
			//Destroy All Windows
			cvDestroyAllWindows();
			// Read the src file
			src = imread("../Image/girl.jpg");
			// Show background
			imshow("src", src);
			TextureFlattening();
			break;
		case 'i':
			e_state = ILLUMINATION;
			//Destroy All Windows
			cvDestroyAllWindows();
			// Read the target file
			target = imread("../Image/banana.jpg");
			roi = target.clone();
			// Show target
			imshow("roi", roi);
			setMouseCallback("roi", onMouse, &roi);
			break;
		case 'c':
			e_state = COLOR;
			//Destroy All Windows
			cvDestroyAllWindows();
			// Read the target file
			target = imread("../Image/flower.jpg");
			roi = target.clone();
			// Show target
			imshow("roi", roi);
			setMouseCallback("roi", onMouse, &roi);
			break;
		case 't':
			e_state = SEAMLESS_TILING;
			//Destroy All Windows
			cvDestroyAllWindows();
			// Read the src file
			src = imread("../Image/tile.jpg");
			// Show background
			imshow("src", src);
			SeamlessTiling();
			break;
		}
	}
	return 0;
}