#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <vector>
#include <cmath>

#define IMAGE "test1.jpg"

using namespace std;
using namespace cv;

Mat img = imread(IMAGE, 0);
Mat imgO = imread(IMAGE, 1);
const int sections = 5;

bool inBounds(int x, int y)
{
	if(y >= img.rows || y < 0)
		return false;
	else if(x >= img.cols || x < 0)
		return false;
	else 
		return true;
}

Point lineIntersection(Point A, Point B, Point C, Point D)
{
    double a1 = B.y - A.y;
    double b1 = A.x - B.x;
    double c1 = a1*(A.x) + b1*(A.y);
 
    double a2 = D.y - C.y;
    double b2 = C.x - D.x;
    double c2 = a2*(C.x)+ b2*(C.y);
 
    double determinant = a1*b2 - a2*b1;
 
    if (determinant == 0)
        return Point(-1, -1);
    else
    {
        double x = (b2*c1 - b1*c2)/determinant;
        double y = (a1*c2 - a2*c1)/determinant;
        return Point(x, y);
    }
}

int main()
{
	Canny( img, img, 100, 250, 3 );
	Mat imgVot(img.rows, img.cols, CV_8UC1, Scalar(0));
	Mat imgHor(img.rows, img.cols, CV_8UC1, Scalar(0));
	Mat imgSec(img.rows/sections, img.cols, CV_8UC1, Scalar(0));
	for(int k = 0; k < sections; ++k)
	{
		int Y = 0;
		for(int y = k*(img.rows/sections); y < (k+1)*(img.rows/sections); ++y)
		{
			for(int x = 0; x < img.cols; ++x)
				imgSec.at<uchar>(Y,x) = img.at<uchar>(y,x);
			Y += 1;
		}
		cout << "\n\nk = " << k << endl;
		vector<Vec4i> lines;
		HoughLinesP(imgSec, lines, 1, CV_PI/180, 50, 10);
		cout << "Total Lines = " << lines.size() << endl;
		for(int j = 0; j < lines.size(); ++j)
		{
			for(int i = j + 1; i < lines.size(); ++i)
			{
				Point inter = lineIntersection(Point(lines[j][0],lines[j][1]),Point(lines[j][2],lines[j][3]),Point(lines[i][0],lines[i][1]),Point(lines[i][2],lines[i][3]));
				inter.y += k*(img.rows/sections);
				if(inBounds(inter.x,inter.y))
				{
					line(imgO, Point(lines[i][2],lines[i][3] + k*(img.rows/sections)), Point(lines[i][0],lines[i][1] + k*(img.rows/sections)), Scalar(0,0,255), 1, CV_AA);
					line(imgO, Point(lines[j][2],lines[j][3] + k*(img.rows/sections)), Point(lines[j][0],lines[j][1] + k*(img.rows/sections)), Scalar(0,0,255), 1, CV_AA);
					int length = min(sqrt(pow(lines[j][0]-lines[j][2],2) + pow(lines[j][1]-lines[j][3],2)), sqrt(pow(lines[i][0]-lines[i][2],2) + pow(lines[i][1]-lines[i][3],2)));
					if(imgVot.at<uchar>(inter.y,inter.x) + length <= 255)
						imgVot.at<uchar>(inter.y,inter.x) += length;
					else
						imgVot.at<uchar>(inter.y,inter.x) = 255;
					if(imgHor.at<uchar>(inter.y,0) + length/4 <= 255)
						for(int X = 0; X < img.cols; ++X)
							imgHor.at<uchar>(inter.y,X) += length/4;
					else
						for(int X = 0; X < img.cols; ++X)
							imgHor.at<uchar>(inter.y,X) = 255;
					//imgVot.at<uchar>(y,x) += (sqrt(pow(lines[j][0]-lines[j][2],2) + pow(lines[j][1]-lines[j][3],2))
					//					    + sqrt(pow(lines[i][0]-lines[i][2],2) + pow(lines[i][1]-lines[i][3],2)))/10;
				}
			}
		}
		//imshow("Sec", imgSec);
		//waitKey(0);
	}
	imshow("Canny", img);
	imshow("Votes", imgVot);
	imshow("Horizon", imgHor);
	imshow("Original", imgO);
	waitKey(0);
	return 0;
}