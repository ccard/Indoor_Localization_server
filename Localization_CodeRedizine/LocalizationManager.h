#pragma once

/**
* Author: Chris Card
* This class calls all of the necessary files and performs the matching
*/

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\nonfree\features2d.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include "opencv2\flann\flann.hpp"
#include "opencv2\objdetect\objdetect.hpp"
#include <type_traits>
#include "ImageProvider.h"
#include "ImageContainer.h"
#include "DBImage.h"
#include "MyMat.h"
#include "DBProvider.h"
#include "LSHMatching.h"
#include "Matcher.h"

using namespace std;
using namespace cv;

#if DEBUG
template<typename ImgType, typename ImgProviderType, typename ImgProviderImgType>
#else
template<typename ImgType>
#endif
class LocalizationManager
{
#if DEBUG
	static_assert(is_base_of<ImageContainer,ImgType>::value, "Must be derived class of ImageContainer");
	static_assert(is_base_of<ImageContainer,ImgProviderImgType>::value, "Must be derived class of ImageContainer");
	static_assert(is_base_of<ImageProvider<ImgProviderImgType>,ImgProviderType>::value,"Must be derived class of ImageProvider");
#else
	static_assert(is_base_of<ImageContainer,ImgType>::value, "Must be derived class of ImageContainer");;
#endif
public:
	const static int LSHMATCHER = 1;

	LocalizationManager(string file,int matchType=LSHMATCHER){
		ASSERT(init(file,matchType),"Failed to initialize");
	}
	~LocalizationManager(){
		delete []db;
		delete []match;
	}

	int operator ==(const ImageContainer &lhs){
		return match->find(lhs,(*db));
	}

	const ImageProvider<ImgType>& getDB(){
		return db;
	}

#if DEBUG
	bool performTestingStats(ImgProviderType &pr,string file){
		return false;
	}

	void performSubsample(string file, int matchType = LSHMATCHER){
		cout << "Performing subsample testing. The results will be outputed to '" << file << "' in csv format." << endl;
		cout << "The format will be 'group,query_file_name,guessed_file_name'." << endl;
		vector<set<int>> testSets = generateTestingSet(30,5);
		vector<pair<int,pair<string,string>>> results,privres;

#ifdef _OPENMP
		cout << "Starting tests in parallel...." << endl;
#pragma omp parallel private(privres) shared(results) num_threads(2)
		{
#pragma omp for
			for(int i = 0; i < testSets.size(); ++i){
				cout << "Running group: " << i << endl;
				ImageProvider<ImgType> *ndb;
				vector<ImgType> testSet;
#pragma omp critical
				{
					createTestingSet(testSets[i],testSet,ndb);
				}
				Matcher<ImgType> *nmatch;
				switch(matchType){
				case LSHMATCHER:
					nmatch = new LSHMatching<ImgType>();
					nmatch->operator<< (*ndb);
					nmatch->train();
					break;
				default:
					ASSERT(false,"There is no matching type: "<<matchType);
				}

				for(vector<ImgType>::iterator j = testSet.begin(); j != testSet.end(); ++j){
					int img = nmatch->find(*j,ndb);
					if (img <= Matcher::ERROR){
						privres.push_back(make_pair(i,make_pair(j->getName(),"no match")));
					} else {
						privres.push_back(make_pair(i,make_pair(j->getName(),ndb->getImage(img).getName())));
					}
				}
				delete []ndb;
				testSet.clear();
			}
#pragma omp critical
			{
				results.insert(results.end(),privres.begin(),privres.end());
			}
		}
#else
		cout << "Starting sequential tests...." << endl;
		for(int i = 0; i < testSets.size(); ++i){
			cout << "Running group: " << i << endl;
			ImageProvider<ImgType> *ndb;
			vector<ImgType> testSet;
			createTestingSet(testSets[i],testSet,ndb);
			Matcher<ImgType> *nmatch;
			switch(matchType){
			case LSHMATCHER:
				nmatch = new LSHMatching<ImgType>();
				nmatch->operator<< (*ndb);
				nmatch->train();
				break;
			default:
				ASSERT(false,"There is no matching type: "<<matchType);
			}

			for(vector<ImgType>::iterator j = testSet.begin(); j != testSet.end(); ++j){
				int img = nmatch->find(*j,ndb);
				if (img <= Matcher::ERROR){
					results.push_back(make_pair(i,make_pair(j->getName(),"no match")));
				} else {
					results.push_back(make_pair(i,make_pair(j->getName(),ndb->getImage(img).getName())));
				}
			}
			delete []ndb;
			testSet.clear();
		}
#endif
		cout << "Writing results to the file ..... " << endl;
		ofstream out(file);
		for(int i = 0; i < results.size(); ++i){
			char buff[10];
			itoa(results[i].first,buff,10);
			string tmps = string(buff) + "," + results[i].second.first + "," + results[i].second.second;
			out << tmps << endl;
		}
		out.close();
		cout << "Finished!" << endl;
	}

	/**
	* Performs database testing based on video input
	*
	* @param: the video file to open
	* @param: the location to save the results to
	* @param: the sampling frequency
	*/
	void performVideoTesting(string vFile, string outFile, bool manualComparison = false, int sampleFrequency = 5){
		VideoCapture vid(vFile.c_str());
		if (vid.isOpened()){
			if (manualComparison){
				int correct = 0;
				int falsePositives = 0;
				int num_sampled = 0;

				namedWindow("Frame");
				namedWindow("Match");
				Mat frame,empty(20,20,CV_16S);
				vid >> frame;
				do{
					++num_sampled;
					MyMat img;
					frame.copyTo(img);
					//resize(img,img,Size(816,612));
					img.initDescriptor();
					img.makeMask();
					img.calcDescriptor();

					int img_num = match->find(img,(*db));

					imshow("Frame",drawKeyPoints(img));

					if(img_num <= Matcher<ImgType>::ERROR){
						imshow("Match",empty);
					} else {
						ImgType tmpimg = db->getImage(img_num);

						if(tmpimg.loadImage()){
							imshow("Match",drawKeyPoints(tmpimg));
						} else {
							cout << "failed to load image" << endl;
						}
					}

					cout << "Is this correct [y]es or [n]o?";
					char yn = waitKey();
					cout << endl;
					if(yn == 'y'){
						correct++;
					} else {
						falsePositives++;
					}

					for(int i = 1; i < sampleFrequency-1; ++i) {
						vid.grab();
					}
					vid >> frame;
				} while (!frame.empty());

				ofstream out(outFile);
				out << "Correct,False Positives,Number of Samples" << endl;
				out << correct << "," << falsePositives << "," << num_sampled << endl;
				out.close();

				cout << "Num correct: " << correct << endl;
				cout << "False positives: " << falsePositives << endl;
				cout << "Accuracy: " << ((correct/num_sampled)*100) << "%" << endl;

			} else {
				cout << "The results will be written to '" << outFile << "' in csv fromat." << endl;
				cout << "The format of the file will be 'timestamp,fram number,matched database image'" << endl;
				map<double,pair<int,string>> results;

				while (vid.grab()) {
					MyMat m;
					vid.retrieve(m);
					resize(m,m,Size(612,816));
					m.initDescriptor();
					m.makeMask();
					m.calcDescriptor();
					double time = vid.get(CV_CAP_PROP_POS_MSEC);
					int frame = vid.get(CV_CAP_PROP_POS_FRAMES);

					int img = match->find(m,(*db));
					pair<int,string> tmp(frame,(img <= Matcher<ImgType>::ERROR ? "No Match" : db->getImage(img).getName()));
					results.insert(make_pair(time,tmp));
					for(int i = 1; i < sampleFrequency-1; ++i) {
						vid.grab();
					}
				}

				ofstream out(outFile);
				for(map<double,pair<int,string>>::iterator i = results.begin(); i != results.end(); ++i){
					out << i->first << "," << i->second.first << "," << i->second.second << endl;
				}
				out.close();
				cout << "Finished .... " << endl;
			}
		} else {
			ASSERT(false,"Failed to open video file "<<vFile);
		}
		vid.release();
	}

#if INSPECT
	void evaluateMatches(string imageFile){

	}

	void evaluateMatches(ImgProviderType &images){

	}
#endif
#endif
private:
	ImageProvider<ImgType> *db;
	Matcher<ImgType> *match;

	bool init(string file,int matchType){
		if(!loadDB(file)){
			return false;
		}
		switch(matchType){
		case LSHMATCHER:
			match = new LSHMatching<ImgType>();
			match->operator<< (*db);
			match->train();
			break;
		default:
			ASSERT(false,"There is no matching type: "<<matchType);
		}
		return true;
	}

	bool loadDB(string file){
		db = new DBProvider<ImgType>(file);
		return db->open();
	}

#if DEBUG
	void createTestingSet(set<int> indicies, vector<ImgType> &testSet, ImageProvider<ImgType> &newdb){
		for(int i = 0; i < db->size(); ++i){
			if(indicies.find(i) != indicies.end()){
				testSet.push_back(db->getImage(i));
			} else {
				newdb.addImage(db->getImage(i));
			}
		}
	}
	vector<set<int>> generateTestingSet(int groupSize,int num_tests){
		vector<set<int>> ret;

		for(int i = 0; i < num_tests; ++i){
			set<int> group;
			srand(time(NULL)*rand());
			while(group.size() < groupSize){
				int index = rand()%db->size();
				if(group.find(index) == group.end()) group.insert(index);
			}
			ret.push_back(group);
		}
		return ret;
	}

	Mat drawKeyPoints(MyMat &im, bool draw_arrows = false,Scalar color = Scalar(0,0,255),bool draw_circle = false){
		KeyPoint p;
		Mat r;
		im.copyTo(r);
		for(int i = 0; i < im.getKeyPoints().size(); ++i) {
			p = im.getKeyPoint(i);

			if(draw_circle) circle(r,p.pt,p.size,color,1);

			if (draw_arrows){
				double x = p.pt.x + p.size*cos(p.angle);
				double y = p.pt.y + p.size*sin(p.angle);

				int mag = 5;
				double pi = 3.141592653;
				double x1 = x-mag*cos(p.angle+pi/4);
				double y1 = y-mag*sin(p.angle+pi/4);

				double x2 = x-mag*cos(p.angle-pi/4);
				double y2 = y-mag*sin(p.angle-pi/4);

				line(r,p.pt,Point(x,y),color);
				line(r,Point(x,y),Point(x1,y1),color);
				line(r,Point(x,y),Point(x2,y2),color);
			}
			circle(r,p.pt,2,Scalar(0,0,255),-1);
			char buff[33];
			itoa(i,buff,10);
			putText(r,buff,Point(p.pt.x+4,p.pt.y+4),FONT_HERSHEY_COMPLEX,.5,
				Scalar(255,13,255));
		}
		return r;
	}
#endif
};