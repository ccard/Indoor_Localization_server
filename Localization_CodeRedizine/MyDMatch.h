#pragma once
/**
* Author: Chris Card
* Date: 1/27/14
*
* this class is a rapper for the opencv struct dmatch to allow
* me to use it as key for a map as well as storing the physical
* key point data for each image so i don't have to pass in the data
* base and query image to every function
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
class MyDMatch
{
public:
	MyDMatch(DMatch m);

	MyDMatch(void);

	int imgIdx;
	int queryIdx;
	int trainIdx;
	float distance;
	KeyPoint train_kp,query_kp;

	bool operator<(const MyDMatch m)const;

	bool operator==(const MyDMatch &m) const;

	bool operator()(const MyDMatch &lhs, const MyDMatch &rhs);
};