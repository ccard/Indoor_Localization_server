#pragma once

/*
* This class contains information for lines
*/

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\nonfree\features2d.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include "opencv2\flann\flann.hpp"
#include "opencv2\objdetect\objdetect.hpp"
using namespace cv;
using namespace std;
class MyLine
{
public:
	static double round(double num) { return (num < 0.0 ? ceil(num - .5) : floor(num + .5)); }
	MyLine(Point p1, Point p2,Scalar color, double thickness = 2,int type = 8, int index = -1);

	void drawLine(Mat &image);

	bool lineIntersection(MyLine other, Point2f& i);

	Point2f lineIntersection(MyLine other);

	bool lineIntersection(Point2f other,int thresh = 2);

	bool lineOverlap(MyLine other, int thresh = 2);

	Mat lineEq();

	double length(){ return norm(p1-p2); }

	pair<Mat,Mat> getNormPoints();

	pair<Point2f,Point2f> getPoints(){ return make_pair(p1,p2); }

	bool isParallel(MyLine other);

	double getSlope(bool angle = true);

	MyLine extend(double width,double height,Point vanishing);

	MyLine extend(double widht, double height);

	//bool lineOverlap(MyLine other);

	double getB(double slope){ return p1.y-slope*p1.x;}

	const Point getP1(){return p1;}

	const Point getP2(){return p2;}

	bool operator<(const MyLine &rhs) const;

	bool operator()(const MyLine &lhs, const MyLine &rhs) const;

	int getIndex(){ return index; }

private:
	Point p1,p2;
	Scalar color;
	double thickness;
	int type,index;
};


