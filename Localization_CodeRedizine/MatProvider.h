#pragma once
/**
* Author: Chris Card
* This class provides images that are not in an xml format
*/
#include "ImageProvider.h"
#include <sys/stat.h>


template<typename ImgType>
class MatProvider :
	public ImageProvider < ImgType >
{
	static_assert(is_same<ImgType, MyMat>::value, "The type must be of MyMat to use this class");
public:
	MatProvider(){}
	MatProvider(string file){
		open(file);
	}

	bool open(string file);

	ImgType getImage(size_t index);

	ImgType operator[] (size_t index);

	vector<ImgType> getImages();

	string getImageLocation(size_t index);

	bool saveImages(string outputlist, string outputdir);

private:

	vector<ImgType> readDB();

	/**
	* Builds a list of the files that it needs to load
	*
	* @return: a list of all files to open and load
	*/
	vector<string> buildRelativeFilePaths();

	pair<string,bool> saveImage(ImgType img,string dir);
};

