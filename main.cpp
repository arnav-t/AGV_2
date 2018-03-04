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

int main()
{
	Canny( img, img, 100, 250, 3 );
	Mat imgVot(img.rows, img.cols, CV_8UC1, Scalar(0));
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
		HoughLinesP(imgSec, lines, 1, CV_PI/180, 80, 30);
		cout << "Total Lines = " << lines.size() << endl;
		for(int i = 0; i < lines.size(); ++i)
		{
			//cout << lines[i][1] << " -> ";
			lines[i][1] += k*(img.rows/sections);
			lines[i][3] += k*(img.rows/sections);
			//cout << lines[i][1] << endl;
		}
		for(int j = 0; j < lines.size(); ++j)
		{
			float a1;
			try
			{
				a1 = (lines[j][3] - lines[j][1])/(lines[j][2] - lines[j][0]);
			}
			catch(...)
			{
				continue;
			}
			float b1 = -1;
			float c1 = lines[j][0] - lines[j][1];
			for(int i = j + 1; i < lines.size(); ++i)
			{
				float a2;
				try
				{
					a2 = (lines[i][3] - lines[i][1])/(lines[i][2] - lines[i][0]);
				}
				catch(...)
				{
					continue;
				}
				float b2 = -1;
				float c2 = lines[i][0] - lines[i][1];
				float det = a1*b2 - a2*b1;
				if((int)(100*fabs(det)) != 0)
				{
					int x = (c1*b2 - c2*b1)/det;
					int y = (a1*c2 - a2*c1)/det;
					cout << x << "\t" << y << endl;
					if(inBounds(x,y))
					{
						line(imgO, Point(lines[i][2],lines[i][3]), Point(lines[i][0],lines[i][1]), Scalar(0,0,255), 1, CV_AA);
						line(imgO, Point(lines[j][2],lines[j][3]), Point(lines[j][0],lines[j][1]), Scalar(0,0,255), 1, CV_AA);
						cout << " ^\t^\t";
						int lengthSum = (sqrt(pow(lines[j][0]-lines[j][2],2) + pow(lines[j][1]-lines[j][3],2)) + sqrt(pow(lines[i][0]-lines[i][2],2) + pow(lines[i][1]-lines[i][3],2)));
						cout << lengthSum << endl;
						imgVot.at<uchar>(y,x) = 255;
						//imgVot.at<uchar>(y,x) += (sqrt(pow(lines[j][0]-lines[j][2],2) + pow(lines[j][1]-lines[j][3],2))
						//					    + sqrt(pow(lines[i][0]-lines[i][2],2) + pow(lines[i][1]-lines[i][3],2)))/10;
					}
				}
			}
		}
		//imshow("Sec", imgSec);
		//waitKey(0);
	}
	imshow("Canny", img);
	imshow("Votes", imgVot);
	imshow("Original", imgO);
	waitKey(0);
	return 0;
}