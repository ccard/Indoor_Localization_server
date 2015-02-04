#include "DBProvider.h"

template<typename ImType>
bool DBProvider<ImType>::open(string file){
	in = ifstream(file);
	if (!in.is_open()){
		ASSERT(false, "Failed to open file: " << file);
	}
	else {
		db = readDB();
	}
	return true;
}

template<typename ImType>
ImType DBProvider<ImType>::getImage(size_t index){
	if (index < 0 || index >= db.size()){
		return ImType();
	}
	return db[index];
}

template<typename ImType>
ImType DBProvider<ImType>::operator[] (size_t index){
	if (index < 0 || index >= db.size()){
		return ImType();
	}
	return db[index];
}

template<typename ImType>
vector<ImType> DBProvider<ImType>::getImages(){
	return db;
}

template<typename ImType>
string DBProvider<ImType>::getImageLocation(size_t index){
	if (index < 0 || index >= db.size()){
		return "No where";
	}
	return db[index].getName();
}

template<typename ImType>
bool DBProvider<ImType>::saveImages(){
	return false;
}
template<typename ImType>
vector<ImType> DBProvider<ImType>::readDB(){
	vector<ImType> ret;
	string line = "";
#if DEBUG
	int size = 0;
#endif
	while (!in.eof()){
		getline(in, line);
		stringstream stream(line);
		istream_iterator<string> begin(stream);
		istream_iterator<string> end;
		vector<string> vstring(begin, end);
		string xml = vstring[0];
		string img_file = vstring[1];
		FileStorage fs(xml, FileStorage::READ);
		Mat des;
		vector<KeyPoint> kps;
		fs["descriptor"] >> des;
		read(fs["keypoints"], kps);
		fs.release();
#if DEBUG
		size += kps.size();
#endif
		ret.push_back(ImType(img_file, des, kps));
	}
	in.close();
#if DEBUG
	cout << "number of feature points: " << size << endl;
#endif
	return ret;
}

template<typename ImType>
vector<string> DBProvider<ImType>::buildRelativeFilePaths(){
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