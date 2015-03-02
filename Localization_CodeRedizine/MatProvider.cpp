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
	int num_complete = 0;
	map<string,string> image_list;
	cout << "Writing image files..." << endl;
	for(vector<ImgType>::iterator i = db.begin(); i != db.end(); ++i){
		++num_complete;
		pair<string,bool> tmp = saveImage(*i,outputdir);
		if(!tmp.second){
			return false;
		}
		image_list.insert(make_pair(tmp.first,i->getName()));
		cout << "\r" << ((i/db_size)*100) << "%            " << flush;
	}
	cout << endl;

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

template<typename ImgType>
pair<string,bool> MatProvider<ImgType>::saveImage(ImgType img, string dir){
	struct stat info;
	if(stat(dir, &info) != 0){
		cerr << "Can not access: " << dir << endl;
		return make_pair("",false);
	} else if(info.st_mode & S_IFDIR){

	} else {
		cerr << "This is not a directory: " << dir << endl;
		return make_pair("",false);
	}
	string name = img.getName();
	name = name.substr(name.find_last_of('\\')+1,name.size());
	string name = dir+name+".xml";
	Mat des;
	img.getDescriptor().copyTo(des);
	FileStorage fs(name,FileStorage::WRITE);
	fs << "keypoints" << img.getKeyPoints();
	fs << "descriptor" << des;
	fs.release();
	return make_pair(name,true);
}