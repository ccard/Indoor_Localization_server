#include "DBProvider.h"

//template<class ImType>
//DBProvider<ImType>::DBProvider(string file){
//	in = ifstream(file);
//	if(!in.is_open()){
//		ASSERT(false,"Failed to open file: "<<file);
//	} else {
//		db = readDB();
//	}
//}
//
//template<class ImType>
//ImType DBProvider<ImType>::getImage(int index){
//	if(index < 0 || index >= db.size()){
//		return DBImage();
//	}
//	return db[index];
//}
//
//template<class ImType>
//vector<ImType> DBProvider<ImType>::getImages(){
//	return db;
//}
//
//template<class ImType>
//vector<ImType> DBProvider<ImType>::readDB(){
//	vector<MyMat> ret;
//	string line ="";
//	int size = 0;
//	while(!in.eof()){
//		getline(in,line);
//		stringstream stream(line);
//		istream_iterator<string> begin(stream);
//		istream_iterator<string> end;
//		vector<string> vstring(begin,end);
//		string xml = vstring[0];
//		string img_file = vstring[1];
//		FileStorage fs(xml,FileStorage::READ);
//		Mat des;
//		vector<KeyPoint> kps;
//		fs["descriptor"] >> des;
//		read(fs["keypoints"], kps);
//		fs.release();
//		size+=kps.size();
//#if is_same<ImType,DBImage>::value
//
//		ret.push_back(DBImage(img_file,des,kps));
//
//#elif is_same<ImType,MyMat>::value
//
//		ret.push_back(MyMat(img_file,des,kps));
//
//#else
//
//		ASSERT(false,"Incompatible "<<"types");
//
//#endif
//	}
//	in.close();
//	cout << "number of feature points: "<<size << endl;
//	return ret;
//}
//
//template<class ImType>
//vector<string> DBProvider<ImType>::buildRelativeFilePaths(){
//	string relativepath;
//	vector<string> ret;
//	while(in >> relativepath){
//		ifstream np(relativepath+"images.txt");
//		string imgname;
//		while(np >> imgname){
//			ret.push_back(relativepath+imgname);
//		}
//		np.close();
//	}
//	in.close();
//	return ret;
//}
//
//template<class ImType>
//string DBProvider<ImType>::getImageLocation(int index){
//	if(index < 0 || index >= db.size()){
//		return "No where";
//	}
//	return db[index].getName();
//}
//
//template<class ImType>
//bool DBProvider<ImType>::saveImages(){
//	return false;
//}