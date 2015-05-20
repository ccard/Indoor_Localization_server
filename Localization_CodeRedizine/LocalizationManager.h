#pragma once

/**
* Author: Chris Card
* This class calls all of the necessary files and performs the matching
* it is inteded to be the primary interface to for matching the only class you need to implement
* are other matchers, ImageProviders, and Image types that are needed
* 
* Usage:
*  Options:
*  1) Use provide sub types of Matcher, ImageProvider, and ImageContainer as this classes template types
*  2) Implement your own Matchers, ImageProviders, and/or ImageContainers by extending them and passing them in as
*     the template type parameters
*
*  So long as your classes extend Matcher, ImageProvider, and ImageContainer this class can utilize them
*/

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/flann/flann.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <type_traits>
#include "ImageProvider.h"
#include "ImageContainer.h"
#include "MyMat.h"
#include "Matcher.h"
#include <time.h>
#include <stdlib.h>
#if DEBUG
#include <time.h>
#endif

using namespace std;
using namespace cv;

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
class LocalizationManager
{
	static_assert(is_base_of<ImageContainer,ImgType>::value, "Must be derived class of ImageContainer");
	static_assert(is_base_of<ImageProvider<ImgType>,ImgProviderType>::value, "Must be derived class of ImageProvider");
	static_assert(is_base_of<Matcher<ImgType>,ImgMatcherType>::value,"Must be derived class of Matcher");

public:
	const static int LSHMATCHER = 1;

	LocalizationManager(string file){
		if(!init(file)){
			ASSERT(false,"Failed to initialize");
		}
	}

	int operator ==(ImageContainer &lhs){
		return match.find(lhs,db);
	}

	ImgType getDBImage(size_t index){
		return db[index];
	}

#if DEBUG
	bool performTestingStats(ImgProviderType &pr, string file);

	void performSubsample(string file);

	/**
	* Performs database testing based on video input
	*
	* @param: the video file to open
	* @param: the location to save the results to
	* @param: the sampling frequency
	* @param: the time in mils to seek to in the video
	*/
	void performVideoTesting(string vFile, string outFile, int minNumORB, bool manualComparison = false,
		int sampleFrequency = 5, double seek = 0);

void evaluateMatches(string imageFile);

void evaluateMatches(ImgProviderType &images);
#endif
private:
	ImgProviderType db;
	ImgMatcherType match;

	bool init(string file){
		if(!loadDB(file)){
			return false;
		}

		match << db;
		match.train();
		return true;
	}

	bool loadDB(string file){
		db.open(file);
		return db.isOpen();
	}

#if DEBUG
	void createTestingSet(set<int> indicies, vector<ImgType> &testSet, ImgProviderType &newdb);

	vector<set<int>> generateTestingSet(size_t groupSize, int num_tests);

	Mat drawKeyPoints(MyMat &im, bool draw_arrows = false, Scalar color = Scalar(0, 0, 255), 
		bool draw_circle = false);
#endif
};