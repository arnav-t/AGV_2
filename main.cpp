#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

#define IMAGE "./testing/image_0/um_000003.png"

using namespace std;
using namespace cv;

Mat img = imread(IMAGE, 0);
Mat imgO = imread(IMAGE, 1);
const int sections = 10;
const float slopeThreshold = 0.3;
const int horWidth = 1;
const int hThres = 7;
const int hMinLineLength = 4;

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

float extendOutside(Point2f &a, Point2f &b)
{
	float m = (b.y - a.y)/(b.x - a.x);
	float c = a.y - m*a.x;
	a = Point(0, c);
	b = Point(img.cols, m*(img.cols) + c);
	return m;
}

void drawExtendedLine(int i, int j, int k, vector<Vec4i> &lines)
{
	Rect secRect(0, k*(img.rows/sections), img.cols, img.rows/sections);
	Point2f i1f = Point(lines[i][2],lines[i][3] + k*(img.rows/sections));
	Point2f i2f = Point(lines[i][0],lines[i][1] + k*(img.rows/sections));
	if(fabs(extendOutside(i1f, i2f)) < slopeThreshold)
		return;
	Point i1(i1f.x, i1f.y);
	Point i2(i2f.x, i2f.y);
	clipLine(secRect, i1, i2);
	line(imgO, i1, i2, Scalar(0,0,255), 1, CV_AA);
	Point2f j1f = Point(lines[j][2],lines[j][3] + k*(img.rows/sections));
	Point2f j2f = Point(lines[j][0],lines[j][1] + k*(img.rows/sections));
	if(fabs(extendOutside(j1f, j2f)) < slopeThreshold)
		return;
	Point j1(j1f.x, j1f.y);
	Point j2(j2f.x, j2f.y);
	clipLine(secRect, j1, j2);
	line(imgO, j1, j2, Scalar(0,0,255), 1, CV_AA);
}

int main(int argc, char *argv[])
{
	char loadPath[64] = IMAGE; 
	if(argc == 2)
	{
		img = imread(argv[1], 0);
		imgO = imread(argv[1], 1);	
		strcpy(loadPath, argv[1]);
	}
	Mat imgSec(img.rows/sections, img.cols, CV_8UC1, Scalar(0));
	Mat imgVotes(img.rows, img.cols, CV_8UC3, Scalar(0,0,0));
	vector<Vec3i> secLines[img.rows];
	vector<vector<Vec4i>> vecOfLines;
	for(int k = 0; k < sections; ++k)
	{
		int Y = 0;
		for(int y = k*(img.rows/sections); y < (k+1)*(img.rows/sections); ++y)
		{
			for(int x = 0; x < img.cols; ++x)
				imgSec.at<uchar>(Y,x) = img.at<uchar>(y,x);
			Y += 1;
		}
		vector<Mat> channels;
		split(imgSec, channels);
		Scalar secMean = mean(channels[0]);
		Canny(imgSec, imgSec, secMean[0], 3*(secMean[0]), 3);
		cout << "\n\nk = " << k << endl;
		vector<Vec4i> lines;
		HoughLinesP(imgSec, lines, 1, CV_PI/180, hThres + k/6, hMinLineLength);
		vecOfLines.push_back(lines);
		cout << "Total Lines = " << lines.size() << endl;
		for(int j = 0; j < lines.size(); ++j)
		{
			for(int i = j + 1; i < lines.size(); ++i)
			{
				//drawExtendedLine(i, j, k, lines);
				Point inter = lineIntersection(Point(lines[j][0],lines[j][1]),Point(lines[j][2],lines[j][3]),Point(lines[i][0],lines[i][1]),Point(lines[i][2],lines[i][3]));
				inter.y += k*(img.rows/sections);
				if(inBounds(inter.x,inter.y))
				{
					int length = min(sqrt(pow(lines[j][0]-lines[j][2],2) + pow(lines[j][1]-lines[j][3],2)), sqrt(pow(lines[i][0]-lines[i][2],2) + pow(lines[i][1]-lines[i][3],2)));
					if(imgVotes.at<Vec3b>(inter.y,inter.x)[0] + 4*length <= 255)
						imgVotes.at<Vec3b>(inter.y,inter.x)[0] += 4*length;
					else
						imgVotes.at<Vec3b>(inter.y,inter.x)[0] = 255;
					if(imgVotes.at<Vec3b>(inter.y,0)[1] + length/8 <= 255)
						for(int X = 0; X < img.cols; ++X)
							imgVotes.at<Vec3b>(inter.y,X)[1] += length/8;
					else
						for(int X = 0; X < img.cols; ++X)
							imgVotes.at<Vec3b>(inter.y,X)[1] = 255;
					secLines[inter.y].push_back(Vec3i(i,j,k));
					//imgVot.at<uchar>(y,x) += (sqrt(pow(lines[j][0]-lines[j][2],2) + pow(lines[j][1]-lines[j][3],2))
					//					    + sqrt(pow(lines[i][0]-lines[i][2],2) + pow(lines[i][1]-lines[i][3],2)))/10;
				}
			}
		}
		//imshow("Sec", imgSec);
		//waitKey(0);
	}
	int peakY = 0;
	for(int y = 0; y < img.rows; ++y)
		if(imgVotes.at<Vec3b>(y,0)[1] > imgVotes.at<Vec3b>(peakY,0)[1])
			peakY = y;
	for(int y = 0; y < img.rows; ++y)
	{
		if(abs(y - peakY) <= horWidth)
			for(int i=0; i < secLines[y].size(); ++i)
				drawExtendedLine(secLines[y][i][0], secLines[y][i][1], secLines[y][i][2], vecOfLines[secLines[y][i][2]]);
	}
	imshow("Votes", imgVotes);
	imshow("Original", imgO);
	char savePath[32] = "./result/";
	strcat(savePath, loadPath + 24);
	cout << "\n\nResult saved to " << savePath << endl;
	imwrite(savePath, imgO);
	waitKey(0);
	return 0;
}