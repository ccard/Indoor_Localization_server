#pragma once
/**
* Author: Chris Card
*
* This class loads in the xml files that contian the precomputed descriptors and
* key points.
*/
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

	bool saveImages(string outputlist, string outputdir);

private:
	vector<ImType> readDB();

	vector<string> buildRelativeFilePaths();
};