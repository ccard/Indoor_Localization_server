#pragma once
/**
 * Author: Chris Card
 *
 * This interface process images and provides them
 * to the other objects
 */

#include "ImageContainer.h"
#include "DBImage.h"
#include "MyMat.h"
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\nonfree\features2d.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include <vector>
#include <type_traits>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iterator>
#include <set>

using namespace cv;
using namespace std;

template <typename T>
 class ImageProvider
  {
	  static_assert(is_base_of<ImageContainer,T>::value, "Must be derived class of ImageContainer");
  public:

	  ~ImageProvider(){
		  db.clear();
	  }

	  bool open(){ return in.is_open() || db.size() > 0;}
  
	virtual T getImage(size_t index) = 0;

	virtual T operator[] (size_t index) = 0;

	virtual vector<T> getImages() = 0;
  	
	virtual string getImageLocation(size_t index) = 0;

	virtual bool saveImages() = 0;
  protected:
	  vector<T> db;
	  ifstream in;
  }; 

