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
bool MatProvider<ImgType>::saveImages(string outputlistfile, string outputdir){
	
#if _OPENMP
	cout << "Writing image files..." << endl;
	map<string,string> image_list,tmp_list;
#pragma omp parallel private(tmp_list) shared(image_list,outputdir) num_threads(NUM_THREADS)
	{
#pragma omp for
		for (int i = 0; i < db.size(); ++i){
			ImgType tmp_img(db[i]);
			pair<string, bool> tmp = saveImage(tmp_img, outputdir);
			if (!tmp.second){
				return false;
			}
			tmp_list.insert(make_pair(tmp.first, tmp_img.getName()));
		}
#pragma omp critical
		{
			for (map<string, string>::iterator t = tmp_list.begin(); t != tmp_list.end(); ++t){
				image_list.insert(t);
			}
		}
	}
#else
	int num_complete = 0,db_size = db.size();
	map<string,string> image_list;
	cout << "Writing image files..." << endl;
	for(vector<ImgType>::iterator i = db.begin(); i != db.end(); ++i){
		++num_complete;
		pair<string,bool> tmp = saveImage(*i,outputdir);
		if(!tmp.second){
			return false;
		}
		image_list.insert(make_pair(tmp.first,i->getName()));
		cout << "\r" << ((num_complete/db_size)*100) << "%            " << flush;
	}
	cout << endl;
#endif

	cout << "Writing to " << outputlistfile << "..." << endl;
	ofstream out(outputlistfile);
	if(!out.is_open()){
		cerr << "Failed to open file " << outputlistfile << endl;
		return false;
	}
	for(map<string,string>::iterator i = image_list.begin(); i != image_list.end(); ++i){
		string tmps = i->first+" "+i->second;
		out << tmps << endl;
	}
	out.close();

	return true;
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
			string tmp_name = images[i];
			imread(tmp_name).copyTo(temp);
			resize(temp, temp, Size(612, 816));
			temp.initDescriptor();
			temp.makeMask();
			temp.calcDescriptor();
			temp.setName(tmp_name);
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
	vector<ImgType> ret;
	for (vector<string>::iterator i = images.begin(); i != images.end(); ++i){
		ImgType temp;
		imread(*i).copyTo(temp);
		resize(temp, temp, Size(612, 816));
		temp.initDescriptor();
		temp.makeMask();
		temp.calcDescriptor();
		temp.setName(*i);
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

template<typename ImgType>
pair<string,bool> MatProvider<ImgType>::saveImage(ImgType &img, string dir){
	struct stat info;
	/*if(stat(dir.c_str(), &info) != 0){
		cerr << "Can not access: " << dir << endl;
		return make_pair("",false);
	} else if(info.st_mode & S_IFDIR){

	} else {
		cerr << "This is not a directory: " << dir << endl;
		return make_pair("",false);
	}*/
	string name = img.getName();
	name = name.substr(name.find_last_of('\\')+1,name.size());
	name = dir+name+".xml";
	Mat des;
	img.getDescriptor().copyTo(des);
	FileStorage fs(name,FileStorage::WRITE);
	fs << "keypoints" << img.getKeyPoints();
	fs << "descriptor" << des;
	fs.release();
	return make_pair(name,true);
}