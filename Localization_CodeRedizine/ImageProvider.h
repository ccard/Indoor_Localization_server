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
#define NUM_THREADS 2
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

	/**
	 * If it was able to open the file and the file is currently open or if the 
	 * the database has images
	 *
	 * @return: true if the ifstream is open or has images
	 */
	bool isOpen(){ return in.is_open() || db.size() > 0;}

	/**
	 * Returns the number of images in the provider
	 *
	 * @return: the number of images in the image provider
	 */
	size_t size(){ return db.size(); }

	/**
	 * Opens the file and reads in the images that the file lists
	 * in the future it may read the images in from a database like sql
	 *
	 * @param: the file containing the list of images to readin
	 * @return: true if was able to open the file and read the images, false otherwise
	 */
	virtual bool open(string file) = 0;

	/**
	 * Returns the iamge at the index or a empty image if the index is out of bounds
	 *
	 * @param: the index of the image to retrieve
	 * 
	 * @return: the image at the index or empty image if index is out of bounds
	 */
	virtual T getImage(size_t index) = 0;

	/**
	 * This functions the same as getImage except that it allows the ImageProvider
	 * to act like an array
	 */
	virtual T operator[] (size_t index) = 0;

	/**
	 * Returns all the images in the ImageProvider
	 *
	 * @return: vector of all the images in the ImageProvider
	 */
	virtual vector<T> getImages() = 0;

	/**
	 * Retrieves the images location that is stored in the imagecontainer
	 * or an error message if there is no location info
	 *
	 * @param: the index of the image
	 *
	 * @return: the images location or an error message if the index is out of bounds
	 */
	virtual string getImageLocation(size_t index) = 0;

	/**
	 * Saves the discriptors and key points of the image
	 *
	 * @return: true if it was able to save false otherwise
	 */
	virtual bool saveImages() = 0;

	void addImage(T image){
		db.push_back(image);
	}

protected:
	vector<T> db;
	ifstream in;
}; 

