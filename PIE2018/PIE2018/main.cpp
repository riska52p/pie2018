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

bool mousedown, ismix = false;
Mat mask, target, roi, src;
vector<vector<Point>> contours;
vector<Point> pts;
Point minp, maxp;

enum state { SEAMLESS_CLONING, MIXED_CLONING, TEXTURE_FLATTENING, ILLUMINATION, COLOR, SEAMLESS_TILING } e_state;

static Mat EdgeDetector(int, void*)
{
	int lowThreshold = 40; //make it can be changed if have time
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
	return edgeMask;
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
			Vector3d p;
			// Unknown pixels
			if (ma.at<uchar>(i, j) == 255)
			{
				p << t.at<Vec3b>(i, j)[0], t.at<Vec3b>(i, j)[1], t.at<Vec3b>(i, j)[2];
				int n = 4;
				// Up
				if (i == 0)
					n--;
				else
				{
					A.coeffRef(i*out.cols + j, (i - 1)*out.cols + j) = -1;
					Vector3d q(t.at<Vec3b>(i - 1, j)[0], t.at<Vec3b>(i - 1, j)[1], t.at<Vec3b>(i - 1, j)[2]);
					Vector3d p1, q1;
					double f, f1;
					switch (e_state)
					{
					case SEAMLESS_CLONING:
						b.row(i*out.cols + j) += p - q;
						break;
					case MIXED_CLONING:
						p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
						q1 = Vector3d(s.at<Vec3b>(i - 1, j)[0], s.at<Vec3b>(i - 1, j)[1], s.at<Vec3b>(i - 1, j)[2]);
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							b.row(i*out.cols + j) += p1 - q1;
						else b.row(i*out.cols + j) += p - q;
						break;
					case TEXTURE_FLATTENING:
						p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
						q1 = Vector3d(s.at<Vec3b>(i - 1, j)[0], s.at<Vec3b>(i - 1, j)[1], s.at<Vec3b>(i - 1, j)[2]);
						b.row(i*out.cols + j) += Vector3d(0, 0, 0);
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
					Vector3d p1, q1;
					double f, f1;
					switch (e_state)
					{
					case SEAMLESS_CLONING:
						b.row(i*out.cols + j) += p - q;
						break;
					case MIXED_CLONING:
						p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
						q1 = Vector3d(s.at<Vec3b>(i + 1, j)[0], s.at<Vec3b>(i + 1, j)[1], s.at<Vec3b>(i + 1, j)[2]);
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							b.row(i*out.cols + j) += p1 - q1;
						else b.row(i*out.cols + j) += p - q;
						break;
					case TEXTURE_FLATTENING:
						p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
						q1 = Vector3d(s.at<Vec3b>(i + 1, j)[0], s.at<Vec3b>(i + 1, j)[1], s.at<Vec3b>(i + 1, j)[2]);
						b.row(i*out.cols + j) += Vector3d(0, 0, 0);
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
					Vector3d p1, q1;
					double f, f1;
					switch (e_state)
					{
					case SEAMLESS_CLONING:
						b.row(i*out.cols + j) += p - q;
						break;
					case MIXED_CLONING:
						p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
						q1 = Vector3d(s.at<Vec3b>(i, j - 1)[0], s.at<Vec3b>(i, j - 1)[1], s.at<Vec3b>(i, j - 1)[2]);
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							b.row(i*out.cols + j) += p1 - q1;
						else b.row(i*out.cols + j) += p - q;
						break;
					case TEXTURE_FLATTENING:
						p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
						q1 = Vector3d(s.at<Vec3b>(i, j - 1)[0], s.at<Vec3b>(i, j - 1)[1], s.at<Vec3b>(i, j - 1)[2]);
						b.row(i*out.cols + j) += Vector3d(0, 0, 0);
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
					Vector3d p1, q1;
					double f, f1;
					switch (e_state)
					{
					case SEAMLESS_CLONING:
						b.row(i*out.cols + j) += p - q;
						break;
					case MIXED_CLONING:
						p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
						q1 = Vector3d(s.at<Vec3b>(i, j + 1)[0], s.at<Vec3b>(i, j + 1)[1], s.at<Vec3b>(i, j + 1)[2]);
						f = (p - q).dot(p - q);
						f1 = (p1 - q1).dot(p1 - q1);
						if (f < f1)
							b.row(i*out.cols + j) += p1 - q1;
						else b.row(i*out.cols + j) += p - q;
						break;
					case TEXTURE_FLATTENING:
						p1 = Vector3d(s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2]);
						q1 = Vector3d(s.at<Vec3b>(i, j + 1)[0], s.at<Vec3b>(i, j + 1)[1], s.at<Vec3b>(i, j + 1)[2]);
						b.row(i*out.cols + j) += Vector3d(0,0,0);
						break;
					}
				}
				// Center
				A.coeffRef(i*out.cols + j, i*out.cols + j) = n;
				/*if (e_state == TEXTURE_FLATTENING)
					b.row(i*out.cols + j) /= 4048;*/
			}
			// Known pixels
			else
			{
				A.coeffRef(i*out.cols + j, i*out.cols + j) = 1;
				p << s.at<Vec3b>(i, j)[0], s.at<Vec3b>(i, j)[1], s.at<Vec3b>(i, j)[2];
				switch (e_state)
				{
				case SEAMLESS_CLONING:
					b.row(i*out.cols + j) = p;
					break;
				case MIXED_CLONING:
					b.row(i*out.cols + j) = p;
					break;
				case TEXTURE_FLATTENING:
					int n = 4;
					//up
					if (i == 0)
						n--;
					else
					{
						Vector3d q(t.at<Vec3b>(i - 1, j)[0], t.at<Vec3b>(i - 1, j)[1], t.at<Vec3b>(i - 1, j)[2]);
						b.row(i*out.cols + j) += q+ p-q;
					}
					//down
					if (i == out.rows - 1)
						n--;
					else
					{
						Vector3d q(t.at<Vec3b>(i + 1, j)[0], t.at<Vec3b>(i + 1, j)[1], t.at<Vec3b>(i + 1, j)[2]);
						b.row(i*out.cols + j) += q + p-q;
					}
					//left
					if (j == 0)
						n--;
					else
					{
						Vector3d q(t.at<Vec3b>(i, j - 1)[0], t.at<Vec3b>(i, j - 1)[1], t.at<Vec3b>(i, j - 1)[2]);
						b.row(i*out.cols + j) += q + p-q;
					}
					//right
					if (j == out.cols - 1)
						n--;
					else
					{
						Vector3d q(t.at<Vec3b>(i, j + 1)[0], t.at<Vec3b>(i, j + 1)[1], t.at<Vec3b>(i, j + 1)[2]);
						b.row(i*out.cols + j) += q + p-q;
					}
					b.row(i*out.cols + j) /= 4;
					break;
				}
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
			{
				//x(i * out.cols + j, k) /= 128;
				// Deal with overflow
				out.at<Vec3b>(i, j)[k] = MAX(MIN(x(i * out.cols + j, k), 255), 0);
			}
				

	return out;
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
			imshow("final", image);
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
int main(int argc, char** argv)
{
	// Read the src file
	src = imread("../Image/boy.jpg");
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
			setMouseCallback("roi", onMouse, &roi);
			setMouseCallback("src", onMouse, &src);
			break;
			// No mixed gradient
		case 'n':
			e_state = SEAMLESS_CLONING;
			setMouseCallback("roi", onMouse, &roi);
			setMouseCallback("src", onMouse, &src);
			break;
			// Edge Detector
		case 'e':
			e_state = TEXTURE_FLATTENING;
			Mat dest = src.clone();
			Mat mask1 = EdgeDetector(0, 0);
			for (int i = 0; i < mask1.rows; i++)
				for (int j = 0; j < mask1.cols; j++)
				{
					if (mask1.at<uchar>(i, j) == 255)
						mask1.at<uchar>(i, j) = 0;
					else if (mask1.at<uchar>(i,j) == 0)
						mask1.at<uchar>(i, j) = 255;
				}
			Mat result = poisson(src, dest, mask1);
			imshow("Texture Flatten", result);
			break;
		case 'i':
			e_state = ILLUMINATION;

			break;
		}
	}
	return 0;
}