#include "MatProvider.h"

template<typename ImgType>
bool MatProvider<ImgType>::open(string file){
	in = ifstream(file);
	if (!in.is_open()){
		ASSERT(false, "Failed to open file: " << file);
	}
	else {
		db = readDB();
	}
	return true;
}

template<typename ImgType>
ImgType MatProvider<ImgType>::getImage(size_t index){
	if (index < 0 || index >= db.size()){
		return ImgType();
	}
	return db[index];
}

template<typename ImgType>
ImgType MatProvider<ImgType>::operator[] (size_t index){
	if (index < 0 || index >= db.size()){
		return ImgType();
	}
	return db[index];
}

template<typename ImgType>
vector<ImgType> MatProvider<ImgType>::getImages(){
	return db;
}

template<typename ImgType>
string MatProvider<ImgType>::getImageLocation(size_t index){
	if (index < 0 || index >= db.size()){
		return "No where";
	}
	return db[index].getName();
}

template<typename ImgType>
bool MatProvider<ImgType>::saveImages(string outputlist, string outputdir){
	return false;
}

template<typename ImgType>
vector<ImgType> MatProvider<ImgType>::readDB(){
	vector<string> images = buildRelativeFilePaths();
	ASSERT(images.size() > 0, "Failed to load the image file");
#ifdef _OPENMP
	vector<ImgType> ret = vector<ImgType>(images.size());
	map<size_t, ImgType> priv;

#pragma omp parallel private(priv) shared(ret) num_threads(NUM_THREADS)
	{
#pragma omp for
		for (int i = 0; i < images.size(); ++i){
			ImgType temp;
			imread(images[i]).copyTo(temp);
			resize(temp, temp, Size(612, 816));
			temp.initDescriptor();
			temp.makeMask();
			temp.calcDescriptor();
			priv.insert(make_pair(i, temp));
		}
#pragma omp critical
			{
				for (map<int, MyMat>::iterator j = priv.begin(); j != priv.end(); ++j){
					ret[j->first] = j->second;
				}
			}
	}
#else
	vector<ImgType> ret = vector<ImgType>(images.size());
	for (vector<string>::iterator i = images.begin(); i != images.end(); ++i){
		ImgType temp;
		imread(*i).copyTo(temp);
		resize(temp, temp, Size(612, 816));
		temp.initDescriptor();
		temp.makeMask();
		temp.calcDescriptor();
		ret.push_back(temp);
	}
#endif
	return ret;
}

template<typename ImgType>
vector<string> MatProvider<ImgType>::buildRelativeFilePaths(){
	string relativepath;
	vector<string> ret;
	while (in >> relativepath){
		ifstream np(relativepath + "images.txt");
		string imgname;
		while (np >> imgname){
			ret.push_back(relativepath + imgname);
		}
		np.close();
	}
	in.close();
	return ret;
}