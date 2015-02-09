#include "MapBuilder.h"

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void MapBuilder<ImgType,ImgProviderType,ImgMatcherType>::buildMap(string output_prefix){
	map<size_t,map<size_t,Rat>> transformMatrix;
	map<size_t,vector<size_t>> adjacency;
	double db_size = db.size();
	for(size_t i = 0; i < db.size(); ++i){
		ImgProviderType ndb;
		newDB(db[i],ndb);
		NearImages r = matchImage(db[i],ndb);
		buildTransform(transformMatrix,adjacency,db[i],r);
		cout << "\r" << ((i/db_size)*100) << "%            " << flush;
	}
	cout << endl;
	writeToFile(output_prefix,transformMatrix,adjacency);
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
NearImages MapBuilder<ImgType, ImgProviderType, ImgMatcherType>::matchImage(ImgType q,ImgProviderType &ndb){
	NearImages r;
	int i = match.find(q, ndb, r);
#if INSPECT
	cout << "R: " << r.first << endl << "t: " << r.second << endl;
	namedWindow("QueryImg", CV_WINDOW_KEEPRATIO);
	namedWindow("DBImg", CV_WINDOW_KEEPRATIO);
	imshow("QueryImg", q);
	if (db[i].loadImage()){
		imshow("DBImg", db[i]);
	}
#endif
	return r;
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void MapBuilder<ImgType, ImgProviderType, ImgMatcherType>::newDB(ImgType &q, ImgProviderType &ndb){
	for(size_t i = 0; i < db.size(); ++i){
		if(db[i].getIndex() == q.getIndex()) continue;
		ndb.addImage(db[i]);
	}
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void MapBuilder<ImgType, ImgProviderType, ImgMatcherType>::buildTransform(map<size_t,map<size_t,Rat>> &transformMatrix, 
	map<size_t,vector<size_t>> &adjacency, ImgType &q, NearImages &ni){
		for(NearImages::iterator i = ni.begin(); i != ni.end(); ++i){
			size_t ni_index = i->first;
			if(transformMatrix.find(q.getIndex()) == transformMatrix.end()){
				transformMatrix.insert(make_pair(q.getIndex(),map<size_t,Rat>()));
			}
			transformMatrix[q.getIndex()].insert(make_pair(ni_index,i->second));

			if(adjacency.find(q.getIndex()) == adjacency.end()){
				adjacency.insert(make_pair(q.getIndex(),vector<size_t>()));
			}
			adjacency[q.getIndex()].push_back(ni_index);
		}
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void MapBuilder<ImgType, ImgProviderType, ImgMatcherType>::writeToFile(string output_prefix,map<size_t,map<size_t,Rat>> &transformMatrix,
	map<size_t,vector<size_t>> &adjacency){

		string outadj = output_prefix+"_adjacency.txt";
		string outtrans = output_prefix+"_ROT.txt";

		cout << "Writing to " << outadj << " ...." << endl;

		ofstream outa(outadj);
		if(!outa.is_open()){
			cerr << "Failed to open " << outadj << " file aborting!" << endl;
			return;
		}
		outa << "n1,adjn_1,...,adjn_n" << endl;
		for(map<size_t,vector<size_t>>::iterator i = adjacency.begin(); i != adjacency.end(); ++i){
			outa << i->first;
			for(vector<size_t>::iterator j = i->second.begin(); j != i->second.end(); ++j){
				if(transformMatrix[i->first][*j].first.empty()) continue;
				outa << "," << *j;
			}
			outa << endl;
		}

		outa.close();

		cout << "Writing to " << outtrans << " ...." << endl;
		ofstream outt(outtrans);
		if(!outt.is_open()){
			cerr << "Failed to open file " << outtrans << endl;
			return;
		}
		outt << "n,adjn,r11,r12,r13,t1,r21,r22,r23,t2,r31,r32,r33,t3" << endl;
		for(map<size_t,map<size_t,Rat>>::iterator i = transformMatrix.begin(); i != transformMatrix.end(); ++i){
			for(map<size_t,Rat>::iterator j = i->second.begin(); j != i->second.end(); ++j){
				if (j == i->second.end()){
					cout << " here" << endl;
					break;
				}
				if(j->second.first.empty()) continue;
				outt << i->first << "," << j->first << ",";
				outt << j->second.first.at<double>(0,0) << "," << j->second.first.at<double>(0,1) << "," << j->second.first.at<double>(0,2) << "," << j->second.second.at<double>(0,0) << ",";
				outt << j->second.first.at<double>(1,0) << "," << j->second.first.at<double>(1,1) << "," << j->second.first.at<double>(1,2) << "," << j->second.second.at<double>(1,0) << ",";
				outt << j->second.first.at<double>(2,0) << "," << j->second.first.at<double>(2,1) << "," << j->second.first.at<double>(2,2) << "," << j->second.second.at<double>(2,0) << endl;
			}
		}
		outt.close();
}