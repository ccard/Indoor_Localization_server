#pragma once
/**
 * Author: Chris Card
 * This Interface encapsulates the different matching paradimes
 */
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\nonfree\features2d.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include "opencv2\flann\flann.hpp"
#include "opencv2\objdetect\objdetect.hpp"
#include "ImageProvider.h"
#include "ImageContainer.h"
#include <type_traits>
#include <typeinfo>

//Defines the static struct for the matching parameters
#ifndef _MATCHPARAMS
	#define _MATCHPARAMS
	//This is the structure that contains the matching params
	static struct MatchParams{
		int k,inlierThresh;
		double fund_f_maxDist,confidence,pointdiff_maxDist;
		bool compactResults,_init;
	} mParams;
#endif

using namespace cv;
using namespace std;

template<typename ImType>
class Matcher
{
	//Ensures that the template type of class is derived from ImageContainer at compile time
	static_assert(is_base_of<ImageContainer,ImType>::value, "Must be derived class of ImageContainer");
public:
	const static int ERROR = -1;
	

	/**
	 * Adds the images to the matching databse
	 *
	 * @param: the image provider with the images
	 */
	virtual void operator<< (ImageProvider<ImType>& images) = 0;

	/**
	 * compares the query image to the database
	 *
	 * @param: the query image
	 */
	virtual int find(ImageContainer& query, ImageProvider<ImType> &db) = 0;

	/**
	 * Trains the database
	 */
	virtual void train() = 0;
};

