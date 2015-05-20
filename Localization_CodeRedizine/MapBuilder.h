#pragma once

/**
 * Author: Chris Card
 * This class builds the topological map from the labled images provided
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
#include "MapMatching.h"
#include <time.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

#ifndef _MAPSTRUCTS
#define _MAPSTRUCTS
	/**
	* Defines the structure of the map to be returned
	*/
	typedef pair<int,Rat> ImgMatch;
#endif

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
class MapBuilder
{
	static_assert(is_base_of<ImageContainer,ImgType>::value, "Must be derived class of ImageContainer");
	static_assert(is_base_of<ImageProvider<ImgType>,ImgProviderType>::value, "Must be derived class of ImageProvider");
	static_assert(is_base_of<Matcher<ImgType>,ImgMatcherType>::value,"Must be derived class of Matcher");
public:
	MapBuilder(string file){
		if(!init(file)){
			ASSERT(false,"Failed to initialize");
		}
	}

	void buildMap(string output_prefix);

	NearImages matchImage(ImgType q, ImgProviderType &ndb);

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

	void newDB(ImgType &q, ImgProviderType &ndb);

	void buildTransform(map<size_t,map<size_t,Rat>> &transformMatrix, map<size_t,vector<size_t>> &adjacency, ImgType &q, NearImages &ni);

	void writeToFile(string output_prefix,map<size_t,map<size_t,Rat>> &transformMatrix, map<size_t,vector<size_t>> &adjacency);
};

