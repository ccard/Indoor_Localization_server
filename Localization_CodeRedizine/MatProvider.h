#pragma once
/**
* Author: Chris Card
* This class provides images that are not in an xml format
*/
#include "ImageProvider.h"


template<typename ImgType>
class MatProvider :
	public ImageProvider<ImgType>
{
	static_assert(is_same<ImgType,MyMat>::value,"The type must be of MyMat to use this class");
public:
	MatProvider(string file){
		in = ifstream(file);
		if(!in.is_open()){
			ASSERT(false,"Failed to open file: "<<file);
		} else {
			db = readDB();
		}
	}

	ImgType getImage(size_t index){
		if(index < 0 || index >= db.size()){
			return ImgType();
		}
		return db[index];
	}

	ImgType operator[] (size_t index){
		if(index < 0 || index >= db.size()){
			return ImgType();
		}
		return db[index];
	}

	vector<ImgType> getImages(){
		return db;
	}

	string getImageLocation(size_t index){
		if(index < 0 || index >= db.size()){
			return "No where";
		}
		return db[index].getName();
	}

	bool saveImages(){
		return false;
	}

private:

	vector<ImgType> readDB(){
		vector<string> images = buildRelativeFilePaths();
		ASSERT(images.size() > 0, "Failed to load the image file");
#ifdef _OPENMP
		vector<ImgType> ret = vector<ImgType>(images.size());
		map<size_t,ImgType> priv;

#pragma omp parallel private(priv) shared(ret) num_threads(4)
		{
#pragma omp for
			for(int i = 0; i < images.size(); ++i){
				ImgType temp;
				imread(images[i]).copyTo(temp);
				resize(temp,temp,Size(612,816));
				temp.initDescriptor();
				temp.makeMask();
				temp.calcDescriptor();
				priv.insert(make_pair(i,temp));
			}
#pragma omp critical
			{
				for(map<int,MyMat>::iterator j = priv.begin(); j != priv.end(); ++j){
					ret[j->first] = j->second;
				}
			}
		}
#else
		vector<ImgType> ret = vector<ImgType>(images.size());
		for(vector<string>::iterator i = images.begin(); i != images.end(); ++i){
			ImgType temp;
			imread(*i).copyTo(temp);
			resize(temp,temp,Size(612,816));
			temp.initDescriptor();
			temp.makeMask();
			temp.calcDescriptor();
			ret.push_back(temp);
		}
#endif
		return ret;
	}

	/**
	* Builds a list of the files that it needs to load
	*
	* @return: a list of all files to open and load
	*/
	vector<string> buildRelativeFilePaths(){
		string relativepath;
		vector<string> ret;
		while(in >> relativepath){
			ifstream np(relativepath+"images.txt");
			string imgname;
			while(np >> imgname){
				ret.push_back(relativepath+imgname);
			}
			np.close();
		}
		in.close();
		return ret;
	}
};

