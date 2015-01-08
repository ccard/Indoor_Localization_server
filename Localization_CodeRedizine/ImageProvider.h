#pragma once
/**
 * Author: Chris Card
 *
 * This interface is for any class that will read in images from a database and then provides those
 * images to any class that may need them
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

#ifdef _OPENMP
#ifndef _OPENMP_THREAD_CNT
#define _OPENMP_THREAD_CNT
#define NUM_THREADS 4
#endif
#endif

template <typename T>
 class ImageProvider
  {
	  static_assert(is_base_of<ImageContainer,T>::value, "Must be derived class of ImageContainer");
  public:

	  ~ImageProvider(){
		 db.clear();
	  }


	  bool isOpen(){ return in.is_open() || db.size() > 0;}

	  size_t size(){ return db.size(); }
  
	virtual bool open(string file) = 0;

	virtual T getImage(size_t index) = 0;

	virtual T operator[] (size_t index) = 0;

	virtual vector<T> getImages() = 0;
  	
	virtual string getImageLocation(size_t index) = 0;

	virtual bool saveImages() = 0;

#if DEBUG
	void addImage(T image){
		db.push_back(image);
	}
#endif
  protected:
	  vector<T> db;
	  ifstream in;
  }; 

