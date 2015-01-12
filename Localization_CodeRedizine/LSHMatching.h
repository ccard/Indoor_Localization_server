#pragma once
/**
 * Author: Chris Card
 * This class implements the use of LSH to perform the matching
 */
#include "Matcher.h"
#include "MyDMatch.h"

#ifndef _LSHTYPES
#define _LSHTYPES
//Defines type defs and structures to return from the methods

typedef map<int,vector<MyDMatch>> ImgMatches; //General image matching structure
typedef map<int,ImgMatches> KNNRes; //The structure returned from KNN method call
typedef pair<vector<Point2f>,vector<Point2f>> ObjectScene; //Defines the object secene relation first part of pair is the object(db), second part is the scene(query)
typedef pair<bool,ImgMatches> FilteredRes; //This is what is returned from the filtering stage
typedef pair<Mat,vector<unsigned int>> Fundimental; //Defines pair from fundimentl matrix to the inliers array
typedef map<int,Fundimental> FundRes; //What is returned form finding fundimental matricies from multiple images
#endif

template <typename ImType>
class LSHMatching :
	public Matcher<ImType>
{
public:
	LSHMatching(){ 
		lastIndex = 0;
		lshMatcher = FlannBasedMatcher(new flann::LshIndexParams(1,31,2));
		if(!mParams._init){
			mParams.k = 20;
			mParams.inlierThresh = 16;
			mParams.confidence = 0.99;
			mParams.pointdiff_maxDist = 0.8;
			mParams.fund_f_maxDist = 0;
			mParams.compactResults = true;
			mParams._init = true;//Ensures that these params are defined only once
		}
	}

	void operator<< (ImageProvider<ImType>& images){
		vector<ImType> imgs = images.getImages();
		vector<Mat> descriptors;

		for(vector<ImType>::iterator i = imgs.begin(); i != imgs.end(); ++i){
			descriptors.push_back(i->getDescriptor());
			index_dbindex.insert(make_pair(lastIndex,i->getIndex()));
			++lastIndex;
		}

		lshMatcher.add(descriptors);
	}

	int find(ImageContainer& query, ImageProvider<ImType> &db){
		KNNRes matches;
		if(knnMatch(query,db,matches)) {
			FilteredRes filteredImgs = filterMatchingImages(matches);
			if(filteredImgs.first){
				return verify(filteredImgs.second,db,query);
			} else {
				return ERROR;
			}
		} else {
			return ERROR;
		}
	}

	void train(){
		lshMatcher.train();	
	}

private:
	FlannBasedMatcher lshMatcher;
	size_t lastIndex;
	map<size_t,size_t> index_dbindex;

	/**
	* This method performs a k nearest neighbor mathing agianst the 
	* flann based matcher
	*
	* @param: query image
	* @param: the data base of images
	* @param: the place to store the matches when all is said and done
	* @param: the minimimum number of neighbors needed to be considered a match
	* @param: the masks empty by default
	* @param: if the algorithm should compact the results
	*
	* @return: true if it found matches
	*/
	bool knnMatch(ImageContainer &query,ImageProvider<ImType> &db,
		KNNRes &match,vector<Mat> &masks = vector<Mat>()){
			vector<vector<DMatch>> m;
			const Mat des = query.getDescriptor();
			lshMatcher.knnMatch(des,m,mParams.k);
			//Converts DMatch to MyDMatch
			match = convertDMatch(m,db,query);
			return match.size() > 0;
	}


	/**
	* This method converts DMatch objects to MyDMat objects with the key points
	* embeded
	*
	* @param: the results of the matching
	* @param: training db
	* @param: the query image
	*
	* @return: the matches vector in the form of MyDMatch
	*/
	KNNRes convertDMatch(vector<vector<DMatch>> matches,
		ImageProvider<ImType> &db, ImageContainer &query){
			KNNRes newMatches;

			for(vector<vector<DMatch>>::iterator i = matches.begin();
				i != matches.end(); ++i){
					for(vector<DMatch>::iterator j = (*i).begin();
						j != (*i).end(); ++j){
							MyDMatch t(*j);
							t.train_kp = db[j->imgIdx].getKeyPoint(j->trainIdx);
							t.query_kp = query.getKeyPoint(j->queryIdx);
							if(newMatches.find(t.imgIdx) != newMatches.end()){
								if(newMatches[t.imgIdx].find(t.queryIdx) != newMatches[t.imgIdx].end()){
									newMatches[t.imgIdx][t.queryIdx].push_back(t);
								} else {
									newMatches[t.imgIdx].insert(pair<int,vector<MyDMatch>>(t.queryIdx,vector<MyDMatch>()));
									newMatches[t.imgIdx][t.queryIdx].push_back(t);
								}
							} else {
								map<int,vector<MyDMatch>> tmp;
								tmp.insert(pair<int,vector<MyDMatch>>(t.queryIdx,vector<MyDMatch>()));
								newMatches.insert(pair<int,map<int,vector<MyDMatch>>>(t.imgIdx,tmp));
								newMatches[t.imgIdx][t.queryIdx].push_back(t);
							}
					}
			}

			return newMatches;
	}

	KNNRes convertDMatch(vector<vector<DMatch>> matches,
		map<int,ImType> &db, map<int,int> tmpdb_to_db, ImageContainer &query){
			KNNRes newMatches;

			for(vector<vector<DMatch>>::iterator i = matches.begin();
				i != matches.end(); ++i){
					for(vector<DMatch>::iterator j = (*i).begin();
						j != (*i).end(); ++j){
							DMatch m = *j;
							m.imgIdx = tmpdb_to_db[j->imgIdx];
							MyDMatch t(m);
							t.train_kp = db[t.imgIdx].getKeyPoint(j->trainIdx);
							t.query_kp = query.getKeyPoint(j->queryIdx);
							if(newMatches.find(t.imgIdx) != newMatches.end()){
								if(newMatches[t.imgIdx].find(t.queryIdx) != newMatches[t.imgIdx].end()){
									newMatches[t.imgIdx][t.queryIdx].push_back(t);
								} else {
									newMatches[t.imgIdx].insert(pair<int,vector<MyDMatch>>(t.queryIdx,vector<MyDMatch>()));
									newMatches[t.imgIdx][t.queryIdx].push_back(t);
								}
							} else {
								map<int,vector<MyDMatch>> tmp;
								tmp.insert(pair<int,vector<MyDMatch>>(t.queryIdx,vector<MyDMatch>()));
								newMatches.insert(pair<int,map<int,vector<MyDMatch>>>(t.imgIdx,tmp));
								newMatches[t.imgIdx][t.queryIdx].push_back(t);
							}
					}
			}

			return newMatches;
	}

	vector<MyDMatch> convertDMatch(vector<DMatch> matches,
		ImageContainer &train, ImageContainer &query,int train_idx){
			vector<MyDMatch> newMatches;
			for(vector<DMatch>::iterator j = matches.begin();
				j != matches.end(); ++j){
					MyDMatch t(*j);
					t.imgIdx = train_idx;
					t.train_kp = train.getKeyPoint(j->trainIdx);
					t.query_kp = query.getKeyPoint(j->queryIdx);
					newMatches.push_back(t);
			}

			return newMatches;
	}

	/**
	* This method builds the object and scene points and returns them so that
	* a homography can be estimated using the ransac method
	* 
	* @param: the vector of corresponding matching points from the training images
	*
	* @return: a pair of vectors first being the training keypoints and the second being
	* the query key points
	*/
	ObjectScene buildObjSceneCorr(vector<MyDMatch> matches){
		vector<Point2f> train,scene;

		for(vector<MyDMatch>::iterator i = matches.begin();
			i != matches.end(); ++i){
				train.push_back(i->train_kp.pt);
				scene.push_back(i->query_kp.pt);
		}

		return make_pair(train,scene);
	}

	/**
	* This method compares the image to the database given a max distance threshold
	* and the minimum number of bins that have to be filled for it to be valid
	*
	* @param: the lsh matching class
	* @param: the number of neighbors required to match
	* @param: the max distance threshold between kypoints
	* @param: the minimum number of bins that have to be full after distance thresholding
	*
	* @return: the if it has all data it needs and the data set image index as traind 
	*  on by lsh
	*/
	FilteredRes filterMatchingImages(KNNRes &matches){
		ImgMatches imgIndex;
		KNNRes imgMatches;
		vector<vector<MyDMatch>> threshmatch;
		if(mParams.pointdiff_maxDist >= 1.0f) return make_pair(false,imgIndex);

		if(!matches.empty()){
			imgMatches = matches;

			//Filters the feature matches and constructs the final form of the structure storing the matches
			for(map<int,map<int,vector<MyDMatch>>>::iterator i = imgMatches.begin();
				i != imgMatches.end(); ++i){
					for(map<int,vector<MyDMatch>>::iterator j = i->second.begin();
						j != i->second.end(); ++j){
							if(j->second.size() >= 2){
								MyDMatch first = j->second[0];
								MyDMatch second = j->second[1];
								double dist = first.distance/second.distance;
								//if the distances ratio pass the test then add the match
								if(dist <= mParams.pointdiff_maxDist){
									if(imgIndex.find(first.imgIdx) != imgIndex.end()){
										imgIndex[first.imgIdx].push_back(first);
									} else {
										imgIndex.insert(pair<int,vector<MyDMatch>>(first.imgIdx,vector<MyDMatch>()));
										imgIndex[first.imgIdx].push_back(first);
									}
								}
							} else {
								if(imgIndex.find(j->second[0].imgIdx) != imgIndex.end()){
									imgIndex[j->second[0].imgIdx].push_back(j->second[0]);
								} else {
									imgIndex.insert(pair<int,vector<MyDMatch>>(j->second[0].imgIdx,vector<MyDMatch>()));
									imgIndex[j->second[0].imgIdx].push_back(j->second[0]);
								}
							}

					}
			}

			size_t mostBins = 0, secondMost = 0, thirdMost = 0,fourthMost=0;
			for(map<int,vector<MyDMatch>>::iterator i = imgIndex.begin();
				i != imgIndex.end(); ++i){
					if(mostBins < i->second.size()) {
						fourthMost = thirdMost;
						thirdMost = secondMost;
						secondMost = mostBins;
						mostBins = i->second.size();
					} else if (secondMost < i->second.size()) {
						fourthMost = thirdMost;
						thirdMost = secondMost;
						secondMost = i->second.size();
					} else if (thirdMost < i->second.size()){
						fourthMost = thirdMost;
						thirdMost = i->second.size();
					} else if (fourthMost < i->second.size()){
						fourthMost = i->second.size();
					}
			}

			ImgMatches ret;
			for(map<int,vector<MyDMatch>>::iterator i = imgIndex.begin();
				i != imgIndex.end(); ++i){
					if(i->second.size() >= secondMost) ret.insert(*i);
			}

			return make_pair(true,ret);
		}

		return make_pair(false,imgIndex);
	}

	/**
	* This method verifies the filtered keypoints are indeed a good match i.e. they form
	* a Homography to each other.
	*
	* This method uses RANSAC to find homographies for all detected images
	*
	* @param: the map of training image indexs to the DMathc object defining the keypoints
	* @param: the database of images so that the test images can be drawn default empty
	* @param: the query image
	* @param: the threshold for the ditance between transformed point and expected point
	* to be considered a good match
	* @param: the percentage of points that the homography fits to be considered a good match
	*
	* @return: pair of boolean and int boolean is true and int contains the image index in the data base
	* if no good match found then boolean is false and int is negative
	*/
	int verify(ImgMatches &matches, ImageProvider<ImType> &db,ImageContainer &query){

		if(matches.size() == 0) return -1;

		FundRes fundamentals = buildFundimentalMat(matches);//buildHomographies(matches);

		double best_fit = 0,second_best = 0;
		int img = ERROR;

		map<int,double> image_inliers;

#if INSPECT
		
//Find inliesrs to the fundamentals
		for(FundRes::iterator i = fundamentals.begin();
			i != fundamentals.end(); ++i){
				vector <MyDMatch> tmp;
				int mean = sumInliers(matches[i->first],i->second.second,tmp);

				if(best_fit < mean){
					showMatches(db[i->first],query,tmp,i->second.first);
					second_best = best_fit;
					best_fit = mean;
					img = i->first;
				} else if (second_best < mean) {
					second_best = mean;
				}
				image_inliers.insert(make_pair(i->first,mean));
		}
#else
		//Find inliesrs to the fundamentals
		for(FundRes::iterator i = fundamentals.begin();
			i != fundamentals.end(); ++i){

				int mean = sumInliers(i->second.second);

				if(best_fit < mean){
					second_best = best_fit;
					best_fit = mean;
					img = i->first;
				} else if (second_best < mean) {
					second_best = mean;
				}
				image_inliers.insert(make_pair(i->first,mean));
		}
#endif

		if (best_fit >= mParams.inlierThresh){
			return img;
		}

		img = ERROR;
		//Remove all images with less than the second best number of inliers
		map<int,ImType> better_matches;
		for(map<int,double>::iterator i = image_inliers.begin(); i != image_inliers.end(); ++i){
			if(i->second >= second_best){
				better_matches.insert(make_pair(i->first,db[i->first]));
			}
		}

		//Rematch the images
		ImgMatches matches2 = doubleCheckMatches(better_matches,query);

		cout << (matches2.empty() ? "empty" : "not empty") << endl;

		//Find fundamental matricies
		FundRes fundimentals = buildFundimentalMat(matches2);

		best_fit = 0;

#if INSPECT
		vector <MyDMatch> inliers,tmp;
		Mat f;
		//Find the image with the best number of inliers
		for(FundRes::iterator i = fundimentals.begin();
			i != fundimentals.end(); ++i){

				int mean = sumInliers(matches2[i->first],i->second.second,tmp);

				if(best_fit < mean){
					inliers.clear();
					inliers.insert(inliers.begin(),tmp.begin(),tmp.end());
					f.release();
					i->second.first.copyTo(f);
					best_fit = mean;
					img = i->first;
				}
		}

		if(best_fit >= mParams.inlierThresh){
			showMatches(db[img],query,inliers,f,true);
		}
#else
		//Find the image with the best number of inliers
		for(FundRes::iterator i = fundimentals.begin();
			i != fundimentals.end(); ++i){

				int mean = sumInliers(i->second.second);

				if(best_fit < mean){
					best_fit = mean;
					img = i->first;
				}
		}
#endif

		return (best_fit >= mParams.inlierThresh ? img : ERROR);
	}

	/**
	* This Function builds the fundimental matricies that relate one image to another
	* and is used to provide the fundimental matrix as well as the number of inliers to
	* that model
	*
	* @param: the matche
	*
	* @return: a map of ints to pairs, first of the pairs is the fundimental matrix the second is
	* the inliers 1 for inlier 0 for not
	*/
	FundRes buildFundimentalMat(ImgMatches matches){
		FundRes ret;

		//Goes through each db image and finds the fundamental matrix between each image and query image
		for(ImgMatches::iterator i = matches.begin(); 
			i != matches.end(); ++i){
				Fundimental p = findFund(buildObjSceneCorr(i->second));
				ret.insert(make_pair(i->first,p));
		}
		return ret;
	}

	/**
	* Finds the fundimental matrix and returns it along with the inliers
	*
	* @param: train and scene points, first train second scene
	*
	* @return: par from thr fund matrix to the inliers
	*/
	Fundimental findFund(ObjectScene train_scene){
		vector<unsigned char> inliers(train_scene.second.size());

		Mat fund = findFundamentalMat(train_scene.first, //db image points
			train_scene.second, //query image points
			CV_FM_RANSAC, //Algorithm to use
			mParams.fund_f_maxDist, //Max error distance after transformation
			mParams.confidence, //confidence in the resulting matrix
			inliers); //stores found inliers

		return pair<Mat,vector<unsigned int>>(fund,vector<unsigned int>(inliers.begin(),inliers.end()));
	}

	/**
	* This function rematches the points to ensure a better fit then what lsh might return
	*
	* @param: the list of training images to match against
	* @param: the query image
	* @param: the error with in the min distance to for a match to be kept
	* 
	* @return: vector of mydmathc objects
	*/
	ImgMatches doubleCheckMatches(map<int,ImType> &db, ImageContainer &query, double min_dist = 0.2){
		BFMatcher tempm(NORM_HAMMING2,mParams.compactResults);

		ImgMatches newMatches;

		/*FlannBasedMatcher tmplshMatcher = FlannBasedMatcher(new flann::LshIndexParams(1,31,2));

		map<int,int> tmpdb_to_db;

		vector<Mat> ndb;

		int index = 0;

		for(map<int,ImType>::iterator i = db.begin(); i != db.end(); ++i){
			tmpdb_to_db.insert(make_pair(index,i->first));
			ndb.push_back(i->second.getDescriptor());
			++index;
		}

		tmplshMatcher.add(ndb);
		tmplshMatcher.train();
		vector<vector<DMatch>> matches;
		tmplshMatcher.knnMatch(query.getDescriptor(),matches,mParams.k);

		KNNRes tmpRes = convertDMatch(matches,db,tmpdb_to_db,query);

		FilteredRes tmpfRes = filterMatchingImages(tmpRes);
		newMatches = tmpfRes.second;*/

		for(map<int,ImType>::iterator i = db.begin(); i != db.end(); ++i){

			vector<DMatch> received;
			tempm.match(query.getDescriptor(),i->second.getDescriptor(),received);

			newMatches.insert(make_pair(i->first,convertDMatch(received,i->second,query,i->first)));

		}
		return newMatches;
	}

	/**
	 * Sums the inliers 
	 */
	int sumInliers(vector<unsigned int> &inliers){
		int b = 0,index=0;
		for(vector<unsigned int>::iterator i = inliers.begin();
			i != inliers.end(); ++i){
				b += *i;
		}
		return b;
	}

#if INSPECT
	int sumInliers(vector<MyDMatch> &matches, vector<unsigned int> &inliers, vector<MyDMatch> &fittingMatches){
		int b = 0,index=0;
		for(vector<unsigned int>::iterator i = inliers.begin();
			i != inliers.end(); ++i){
				b += *i;
				if(*i){
					fittingMatches.push_back(matches[index]);
				}
				index++;
		}
		return b;
	}
	void showMatches(ImType &db, ImageContainer &query, vector<MyDMatch> &inliers, Mat F,bool step = false){
		ObjectScene objscene = buildObjSceneCorr(inliers);
		if(!step){
			if(db.loadImage()){
				Mat r;
				db.getMat(r);
				KeyPoint p;
				for(size_t i = 0; i < inliers.size(); ++i) {
					p = inliers[i].train_kp;

					circle(r,p.pt,2,Scalar(0,0,255),-1);
					char buff[33];
					itoa(i,buff,10);
					putText(r,buff,Point(p.pt.x+4,p.pt.y+4),FONT_HERSHEY_COMPLEX,.5,
						Scalar(255,13,255));
				}
				vector<Vec3f> eplines;
				computeCorrespondEpilines(objscene.second,2,F,eplines);
				for(vector<Vec3f>::iterator t = eplines.begin(); t != eplines.end(); ++t){
					float y1 = (-1*((*t)[2]/(*t)[1]));
					float y2 = (-1*((*t)[0]/(*t)[1])*db.imageSize().width)-((*t)[2]/(*t)[1]);
					Point p1(0,y1),p2(db.imageSize().width,y2);
					line(r,p1,p2,Scalar::all(-1));
				}
				imshow("DBInliers", r);
			} else {
				cout << "No DB image to show" << endl;
			}
			if(query.hasImage()){
				Mat r;
				query.getMat(r);
				KeyPoint p;
				for(size_t i = 0; i < inliers.size(); ++i) {
					p = inliers[i].query_kp;

					circle(r,p.pt,2,Scalar(0,0,255),-1);
					char buff[33];
					itoa(i,buff,10);
					putText(r,buff,Point(p.pt.x+4,p.pt.y+4),FONT_HERSHEY_COMPLEX,.5,
						Scalar(255,13,255));
				}
				vector<Vec3f> eplines;
				computeCorrespondEpilines(objscene.first,1,F,eplines);
				for(vector<Vec3f>::iterator t = eplines.begin(); t != eplines.end(); ++t){
					float y1 = (-1*((*t)[2]/(*t)[1]));
					float y2 = (-1*((*t)[0]/(*t)[1])*query.imageSize().width)-((*t)[2]/(*t)[1]);
					Point p1(0,y1),p2(query.imageSize().width,y2);
					line(r,p1,p2,Scalar::all(-1));
				}
				imshow("QueryInliers",r);
			} else {
				cout << "No Query Image to show" << endl;
			}
		} else if(query.hasImage() && db.loadImage()) {
			Mat q,d;
			
			vector<Vec3f> eplinesdb,eplinesq;
			computeCorrespondEpilines(objscene.first,1,F,eplinesq);
			computeCorrespondEpilines(objscene.second,2,F,eplinesdb);
			for(size_t t = 0; t < eplinesdb.size(); ++t){
				db.getMat(d);
				query.getMat(q);
				
				KeyPoint q_p = inliers[t].query_kp;
				circle(q,q_p.pt,2,Scalar(0,0,255),-1);
				char q_buff[33];
				itoa(t,q_buff,10);
				putText(q,q_buff,Point(q_p.pt.x+4,q_p.pt.y+4),FONT_HERSHEY_COMPLEX,.5,
					Scalar(255,13,255));

				KeyPoint d_p = inliers[t].train_kp;
				circle(d,d_p.pt,2,Scalar(0,0,255),-1);
				char d_buff[33];
				itoa(t,d_buff,10);
				putText(d,d_buff,Point(d_p.pt.x+4,d_p.pt.y+4),FONT_HERSHEY_COMPLEX,.5,
					Scalar(255,13,255));

				float q_y1 = (-1*(eplinesq[t][2]/eplinesq[t][1]));
				float q_y2 = (-1*(eplinesq[t][0]/eplinesq[t][1])*query.imageSize().width)-(eplinesq[t][2]/eplinesq[t][1]);
				Point q_p1(0,q_y1),q_p2(query.imageSize().width,q_y2);
				line(q,q_p1,q_p2,Scalar(0,100,255));

				float db_y1 = (-1*(eplinesdb[t][2]/eplinesdb[t][1]));
				float db_y2 = (-1*(eplinesdb[t][0]/eplinesdb[t][1])*db.imageSize().width)-(eplinesdb[t][2]/eplinesdb[t][1]);
				Point db_p1(0,db_y1),db_p2(db.imageSize().width,db_y2);
				line(d,db_p1,db_p2,Scalar(0,100,255));

				imshow("QueryInliers",q);
				imshow("DBInliers", d);
				waitKey();
			}
		}
		
		waitKey();
	}
#endif
};

