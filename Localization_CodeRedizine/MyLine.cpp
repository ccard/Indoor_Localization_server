#include "MyLine.h"
#include <stdlib.h>
using namespace std;


MyLine::MyLine(Point p1, Point p2,Scalar color, double thickness,int type, int index){
	this->p1 = p1;
	this->p2 = p2;
	this->color = color;
	this->thickness = thickness;
	this->type = type;
	this->index = index;
}

void MyLine::drawLine(Mat &image){
	line(image,p1,p2,color,thickness,type);
}

MyLine MyLine::extend(double width,double height, Point vanishing){
	Point p3(0,0);
	Point p4(0,height-1);
	if(p1.x == p2.x){
		p3.x = p1.x;
		p4.x = p2.x;
	} else if(p1.y == p2.y){
		p3.y = p1.y;
		p4.y = p2.y;
		p4.x = width-1;
	} else{
		double slope = getSlope(false);
		double b = getB(slope);

		p3.x = (-b)/slope;
		p4.x = (height-b)/slope;
	}

	bool left = p1.x <= vanishing.x;
	bool for3 = (left ? p3.x < vanishing.x : p3.x > vanishing.x);
	bool for4 = (left ? p4.x < vanishing.x : p4.x > vanishing.x);
	Point p = (for3 && !for4 ? p3 : p4);
	return MyLine(vanishing,p,Scalar(255,0,0),thickness,type,index);
}

MyLine MyLine::extend(double width,double height){
	Point p3(0,0);
	Point p4(0,height-1);
	if(p1.x == p2.x){
		p3.x = p1.x;
		p4.x = p2.x;
	} else if(p1.y == p2.y){
		p3.y = p1.y;
		p4.y = p2.y;
		p4.x = width-1;
	}else{
		double slope = getSlope(false);
		double b = getB(slope);

		p3.x = (-b)/slope;
		p4.x = (height-b)/slope;
	}
	return MyLine(p3,p4,color,thickness,type,index);
}

bool MyLine::lineIntersection(Point2f p, int thresh){


	double slope = getSlope(false);
	double b = getB(slope);

	double y = p.x*slope+b;

	bool match = y >= (p.y-thresh);
	match &= y <= (p.y+thresh);
	return match;
}

bool MyLine::lineIntersection(MyLine other,Point2f& i){
	Mat l1 = lineEq();
	Mat l2 = other.lineEq();
	Mat p = l1.cross(l2);

	pair<Point2f,Point2f> points1 = getPoints();
	pair<Point2f,Point2f> points2 = other.getPoints();
	Point2f p1_1 = points1.first;
	Point2f p1_2 = points1.second;
	Point2f p2_1 = points2.first;
	Point2f p2_2 = points2.second;
	float p_div = (p.at<float>(2,0) == 0 ? 1.f : p.at<float>(2,0));
	Point2f p_3(round(p.at<float>(0,0)/p_div),round(p.at<float>(1,0)/p_div));

	bool fits = (min(p1_1.x, p1_2.x) <= p_3.x) && (max(p1_1.x, p1_2.x) >= p_3.x);
	fits &= (min(p1_1.y, p1_2.y) <= p_3.y) && (max(p1_1.y, p1_2.y) >= p_3.y);
	fits &= (min(p2_1.x, p2_2.x) <= p_3.x) && (max(p2_1.x, p2_2.x) >= p_3.x);
	fits &= (min(p2_1.y, p2_2.y) <= p_3.y) && (max(p2_1.y , p2_2.y) >= p_3.y);
	i = (fits ? p_3 : Point2f(-1.f,-1.f));
	return fits;

}

//bool MyLine::lineOverlap(MyLine other){
//	double slope1,slope2,b1,b2;
//	slope1 = getSlope(false);
//	slope2 = other.getSlope(false);
//	b1 = getB(slope1);
//	b2 = other.getB(slope2);
//
//	bool s = ((slope1 >= 0) && (slope2 >= 0)) || ((slope1 < 0) && (slope2 < 0));
//	bool slopediff = (s ? abs(abs(slope1) - abs(slope2)) < 3 : false);
//	bool interdiff = (s ? abs(abs(b1)-abs(b2)) < 3 : false);
//
//	return s && slopediff && interdiff;
//}

Point2f MyLine::lineIntersection(MyLine other){
	Mat l1 = lineEq();
	Mat l2 = other.lineEq();
	Mat p = l1.cross(l2);

	float p_div = (p.at<float>(2,0) == 0 ? 1.f : p.at<float>(2,0));
	Point2f p_3(p.at<float>(0,0)/p_div,p.at<float>(1,0)/p_div);
	return p_3;
}

pair<Mat,Mat> MyLine::getNormPoints(){
	Mat x1(3,1,CV_32F);
	Mat x2(3,1,CV_32F);
	x1.at<float>(0,0) = p1.x;
	x1.at<float>(1,0) = p1.y;
	x1.at<float>(2,0) = 1.0f;
	x2.at<float>(0,0) = p2.x;
	x2.at<float>(1,0) = p2.y;
	x2.at<float>(2,0) = 1.0f;
	normalize(x1,x1);
	normalize(x2,x2);
	return make_pair(x1,x2);
}

bool MyLine::lineOverlap(MyLine other,int thresh){
	bool match = other.lineIntersection(p1,thresh);
	match &= other.lineIntersection(p2,thresh);
	return match;
}

Mat MyLine::lineEq(){
	Mat x1(3,1,CV_32F);
	Mat x2(3,1,CV_32F);
	x1.at<float>(0,0) = p1.x;
	x1.at<float>(1,0) = p1.y;
	x1.at<float>(2,0) = 1.0f;
	x2.at<float>(0,0) = p2.x;
	x2.at<float>(1,0) = p2.y;
	x2.at<float>(2,0) = 1.0f;
	normalize(x1,x1);
	normalize(x2,x2);

	return x1.cross(x2);
}

bool MyLine::isParallel(MyLine other){
	Mat l1 = lineEq();
	Mat l2 = other.lineEq();
	Mat p = l1.cross(l2);

	bool par = 0 == p.at<float>(0,0);
	par &= 0 == p.at<float>(1,0);
	par &= 0 == p.at<float>(2,0);
	return par;
}

double MyLine::getSlope(bool angle){
	double theta = (angle ? atan2(double(p2.y-p1.y),p2.x-p1.x)*180/CV_PI : (p2.y-p1.y)/((double)(p2.x-p1.x)));
	return theta;
}

bool MyLine::operator<(const MyLine &rhs) const{
	return (index < rhs.index);
}

bool MyLine::operator()(const MyLine &lhs, const MyLine &rhs) const{
	return (lhs.index == rhs.index);
}
