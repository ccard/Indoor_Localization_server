#pragma once

/**
* Author: Chris Card
* This class calls all of the necessary files and performs the matching
* it is inteded to be the primary interface to for matching the only class you need to implement
* are other matchers, ImageProviders, and Image types that are needed
* 
* Usage:
*  Options:
*  1) Use provide sub types of Matcher, ImageProvider, and ImageContainer as this classes template types
*  2) Implement your own Matchers, ImageProviders, and/or ImageContainers by extending them and passing them in as
*     the template type parameters
*
*  So long as your classes extend Matcher, ImageProvider, and ImageContainer this class can utilize them
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
#include "MyMat.h"
#include "Matcher.h"
#include <time.h>
#include <stdlib.h>

using namespace std;
using namespace cv;

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
class LocalizationManager
{
	static_assert(is_base_of<ImageContainer,ImgType>::value, "Must be derived class of ImageContainer");
	static_assert(is_base_of<ImageProvider<ImgType>,ImgProviderType>::value, "Must be derived class of ImageProvider");
	static_assert(is_base_of<Matcher<ImgType>,ImgMatcherType>::value,"Must be derived class of Matcher");

public:
	const static int LSHMATCHER = 1;

	LocalizationManager(string file){
		if(!init(file)){
			ASSERT(false,"Failed to initialize");
		}
	}

	int operator ==(const ImageContainer &lhs){
		return match.find(lhs,db);
	}

	const ImgProviderType getDB(){
		return db;
	}

#if DEBUG
	bool performTestingStats(ImgProviderType &pr,string file){

		map<string, string> results;

		for (int i = 0; i < pr.size(); ++i){
			int img = match.find(pr[i], db);

			if (ImgMatcherType::ERROR >= img){
				results.insert(make_pair(pr[i].getName(), "no match"));
			}
			else {
				results.insert(make_pair(pr[i].getName(), db[img].getName()));
			}
		}

		ofstream out(file.c_str());

		for (map<string, string>::iterator i = results.begin(); i != results.end(); ++i){
			out << i->first << "," << i->second << endl;
		}

		out.close();
		return true;
	}

	void performSubsample(string file){
		cout << "Performing subsample testing. The results will be outputed to '" << file << "' in csv format." << endl;
		cout << "The format will be 'group,query_file_name,guessed_file_name'." << endl;
		vector<set<int>> testSets = generateTestingSet(30,5);
		vector<pair<int,pair<string,string>>> results,privres;

#ifdef _OPENMP
		cout << "Starting tests in parallel...." << endl;
#pragma omp parallel private(privres) shared(results) num_threads(NUM_THREADS)
		{
#pragma omp for
			for(int i = 0; i < testSets.size(); ++i){
				cout << "Running group: " << i << endl;
				ImgProviderType ndb;
				vector<ImgType> testSet;
#pragma omp critical
				{
					createTestingSet(testSets[i],testSet,ndb);
				}
				ImgMatcherType nmatch;
				nmatch << ndb;
				nmatch.train();

				for(vector<ImgType>::iterator j = testSet.begin(); j != testSet.end(); ++j){
					int img = nmatch.find(*j,ndb);
					if (img <= ImgMatcherType::ERROR){
						privres.push_back(make_pair(i,make_pair(j->getName(),"no match")));
					} else {
						privres.push_back(make_pair(i,make_pair(j->getName(),ndb[img].getName())));
					}
				}

				testSet.clear();
			}
#pragma omp critical
			{
				results.insert(results.end(),privres.begin(),privres.end());
			}
		}
#else
		cout << "Starting sequential tests...." << endl;
		for(size_t i = 0; i < testSets.size(); ++i){
			cout << "Running group: " << i << endl;
			ImgProviderType ndb;
			vector<ImgType> testSet;
			createTestingSet(testSets[i],testSet,ndb);
			ImgMatcherType nmatch;
			nmatch << ndb;
			nmatch.train();

			for(vector<ImgType>::iterator j = testSet.begin(); j != testSet.end(); ++j){
				int img = nmatch.find(*j,ndb);
				if (img <= ImgMatcherType::ERROR){
					results.push_back(make_pair(i,make_pair(j->getName(),"no match")));
				} else {
					results.push_back(make_pair(i,make_pair(j->getName(),ndb[img].getName())));
				}
			}
			
			testSet.clear();
		}
#endif
		cout << "Writing results to the file ..... " << endl;
		ofstream out(file);
		for(size_t i = 0; i < results.size(); ++i){
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
	* @param: the time in mils to seek to in the video
	*/
	void performVideoTesting(string vFile, string outFile, int minNumORB, bool manualComparison = false, int sampleFrequency = 5, double seek = 0){
		VideoCapture vid(vFile.c_str());
		if (vid.isOpened()){
			if (manualComparison){
				double correct = 0, falsePositives = 0;
				size_t num_sampled = 0;

				namedWindow("Frame");
				namedWindow("Match");
				Mat frame,empty(20,20,CV_16S);
#if INSPECT
				vid.set(CV_CAP_PROP_POS_MSEC,seek);
#endif
				do{
					vid >> frame;
					MyMat img;
					frame.copyTo(img);
					//resize(img,img,Size(816,612));
					img.initDescriptor();
					img.makeMask();
					img.calcDescriptor();

					if(img.getKeyPoints().size() < minNumORB) continue;

					++num_sampled;
					int img_num = match.find(img,db);

					imshow("Frame",drawKeyPoints(img));

					if(img_num <= ImgMatcherType::ERROR){
						imshow("Match",empty);
						cout << "no match: " << img_num << endl;
					} else {
						ImgType tmpimg = db[img_num];

						if(tmpimg.loadImage()){
							imshow("Match",drawKeyPoints(tmpimg));
						} else {
							cout << "failed to load image" << endl;
						}
					}

					cout << "Is this correct [y]es or [n]o?";
					char yn = waitKey();
					cout << yn << endl;
					if(yn == 'y'){
						correct++;
					} else {
						falsePositives++;
					}

					cout << "Running accuracy: " << ((correct/num_sampled)*100) << "%" << endl;

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
				double num_sampled = 0;
				time_t running = 0;

				while (vid.grab()) {
					time_t start = time(NULL);
					++num_sampled;
					MyMat m;
					vid.retrieve(m);
					//resize(m,m,Size(612,816));
					m.initDescriptor();
					m.makeMask();
					m.calcDescriptor();
					double timestamp = vid.get(CV_CAP_PROP_POS_MSEC);
					int frame = vid.get(CV_CAP_PROP_POS_FRAMES);

					int img = match.find(m,db);
					pair<int,string> tmp(frame,(img <= ImgMatcherType::ERROR ? "No Match" : db.getImage(img).getName()));
					results.insert(make_pair(timestamp,tmp));
					running += (time(NULL)-start);
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
				cout << "Average time per frame: " << (running/num_sampled) << endl;
			}
		} else {
			ASSERT(false,"Failed to open video file "<<vFile);
		}
		vid.release();
	}

	void evaluateMatches(string imageFile){
		MyMat q;
		Mat r = imread(imageFile);
		r.copyTo(q);
		resize(q,q,Size(612,816));
		q.makeMask();
		q.initDescriptor();
		q.calcDescriptor();

		int image = match.find(q,db);
		namedWindow("Query",CV_WINDOW_KEEPRATIO);
		namedWindow("Retrieved",CV_WINDOW_KEEPRATIO);
		if(image > ImgMatcherType::ERROR){
			imshow("Query",q);
			if(db[image].loadImage()){
			Mat rt;
			db[image].getMat(rt);
			imshow("Retrieved",rt);
			}
			waitKey();
		}
	}

	void evaluateMatches(ImgProviderType &images){

	}
#endif
private:
	ImgProviderType db;
	ImgMatcherType match;

	bool init(string file){
		if(!loadDB(file)){
			return false;
		}

		match << db;
		match.train();
		return true;
	}

	bool loadDB(string file){
		db.open(file);
		return db.isOpen();
	}

#if DEBUG
	void createTestingSet(set<int> indicies, vector<ImgType> &testSet, ImgProviderType &newdb){
		for(size_t i = 0; i < db.size(); ++i){
			if(indicies.find(i) != indicies.end()){
				testSet.push_back(db[i]);
			} else {
				newdb.addImage(db[i]);
			}
		}
	}
	vector<set<int>> generateTestingSet(size_t groupSize,int num_tests){
		vector<set<int>> ret;

		for(int i = 0; i < num_tests; ++i){
			set<int> group;
			srand(time(NULL)*rand());
			while(group.size() < groupSize){
				int index = rand()%db.size();
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
		for(size_t i = 0; i < im.getKeyPoints().size(); ++i) {
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