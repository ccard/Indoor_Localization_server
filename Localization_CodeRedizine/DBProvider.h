#pragma once
#include "ImageProvider.h"

template<typename ImType>
class DBProvider :
	public ImageProvider<ImType>
{
public:

	DBProvider(){}

	DBProvider(string file){
		open(file);
	}

	bool open(string file);

	ImType getImage(size_t index);

	ImType operator[] (size_t index);

	vector<ImType> getImages();

	string getImageLocation(size_t index);

	bool saveImages();

private:
	vector<ImType> readDB();

	vector<string> buildRelativeFilePaths();
};