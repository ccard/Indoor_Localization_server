#pragma once
#include "ImageProvider.h"

template<typename ImType>
class DBProvider :
	public ImageProvider<ImType>
{
public:
	DBProvider(string file){
		in = ifstream(file);
		if(!in.is_open()){
			ASSERT(false,"Failed to open file: "<<file);
		} else {
			db = readDB();
		}
	}

	ImType getImage(size_t index){
		if(index < 0 || index >= db.size()){
			return ImType();
		}
		return db[index];
	}

	ImType operator[] (size_t index){
		if(index < 0 || index >= db.size()){
			return ImType();
		}
		return db[index];
	}

	vector<ImType> getImages(){
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
	vector<ImType> readDB(){
		vector<ImType> ret;
		string line ="";
#if DEBUG
		int size = 0;
#endif
		while(!in.eof()){
			getline(in,line);
			stringstream stream(line);
			istream_iterator<string> begin(stream);
			istream_iterator<string> end;
			vector<string> vstring(begin,end);
			string xml = vstring[0];
			string img_file = vstring[1];
			FileStorage fs(xml,FileStorage::READ);
			Mat des;
			vector<KeyPoint> kps;
			fs["descriptor"] >> des;
			read(fs["keypoints"], kps);
			fs.release();
#if DEBUG
			size+=kps.size();
#endif
			ret.push_back(ImType(img_file,des,kps));
		}
		in.close();
#if DEBUG
		cout << "number of feature points: "<<size << endl;
#endif
		return ret;
	}


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