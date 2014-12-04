#pragma once

/**
* Author: Chris Card
* This class calls all of the necessary files and performs the matching
*/

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\nonfree\features2d.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include "opencv2\flann\flann.hpp"
#include "opencv2\objdetect\objdetect.hpp"
#include <type_traits>
#include "ImageProvider.h"
#include "ImageContainer.h"
#include "DBImage.h"
#include "MyMat.h"
#include "DBProvider.h"
#include "LSHMatching.h"
#include "Matcher.h"

using namespace std;
using namespace cv;

#if DEBUG
template<typename ImgType, typename ImgProviderType, typename ImgProviderImgType>
#else
template<typename ImgType>
#endif
class LocalizationManager
{
#if DEBUG
	static_assert(is_base_of<ImageContainer,ImgType>::value, "Must be derived class of ImageContainer");
	static_assert(is_base_of<ImageContainer,ImgProviderImgType>::value, "Must be derived class of ImageContainer");
	static_assert(is_base_of<ImageProvider<ImgProviderImgType>,ImgProviderType>::value,"Must be derived class of ImageProvider");
#else
	static_assert(is_base_of<ImageContainer,ImgType>::value, "Must be derived class of ImageContainer");;
#endif
public:
	const static int LSHMATCHER = 1;

	LocalizationManager(string file,int matchType=LSHMATCHER){
		ASSERT(init(file,matchType),"Failed to initialize");
	}
	~LocalizationManager(){
		delete []db;
		delete []match;
	}

	int operator ==(const ImageContainer &lhs){
		return match->find(lhs,(*db));
	}

#if DEBUG
	bool performTestingStats(ImgProviderType &pr,string file){
		return false;
	}

	void performSubsample(string file){

	}

	void performVideoTesting(string vFile){

	}

#if INSPECT
	void evaluateMatches(string imageFile){

	}

	void evaluateMatches(ImgProviderType &images){

	}
#endif
#endif
private:
	ImageProvider<ImgType> *db;
	Matcher<ImgType> *match;

	bool init(string file,int matchType){
		if(!loadDB(file)){
			return false;
		}
		switch(matchType){
		case LSHMATCHER:
			match = new LSHMatching<ImgType>();
			match->operator<< (*db);
			match->train();
			break;
		default:
			ASSERT(false,"There is no matching type: "<<matchType);
		}
		return true;
	}

	bool loadDB(string file){
		db = new DBProvider<ImgType>(file);
		return db->open();
	}
};

