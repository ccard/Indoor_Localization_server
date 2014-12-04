//#include "LSHMatching.h"
//
//
//template<class ImType>
//int LSHMatching<ImType>::find(ImageContainer& query, ImageProvider<ImType> &db){
//	KNNRes matches;
//	if(knnMatch(query,db,matches)) {
//		FilteredRes filteredImgs = filterMatchingImages(matches);
//		if(filteredImgs.first){
//			return verify(filteredImgs.second,db,query);
//		} else {
//			return ERROR;
//		}
//	} else {
//		return ERROR;
//	}
//}
//
//template<class ImType>
//bool LSHMatching<ImType>::knnMatch(ImageContainer &query,ImageProvider<ImType> &db,
//	KNNRes &match,vector<Mat> &masks){
//		vector<vector<DMatch>> m;
//		const Mat des = query.getDescriptor();
//		lshMatcher.knnMatch(des,m,mParams.k);
//		//Converts DMatch to MyDMatch
//		match = convertDMatch(m,db,query);
//		return match.size() > 0;
//}
//
//template<class ImType>
//KNNRes LSHMatching<ImType>::convertDMatch(vector<vector<DMatch>> matches,
//	ImageProvider<ImType> &db, ImageContainer &query){
//		KNNRes newMatches;
//
//		for(vector<vector<DMatch>>::iterator i = matches.begin();
//			i != matches.end(); ++i){
//				for(vector<DMatch>::iterator j = (*i).begin();
//					j != (*i).end(); ++j){
//						MyDMatch t(*j);
//						t.train_kp = db[j->imgIdx].getKeyPoint(j->trainIdx);
//						t.query_kp = query.getKeyPoint(j->queryIdx);
//						if(newMatches.find(t.imgIdx) != newMatches.end()){
//							if(newMatches[t.imgIdx].find(t.queryIdx) != newMatches[t.imgIdx].end()){
//								newMatches[t.imgIdx][t.queryIdx].push_back(t);
//							} else {
//								newMatches[t.imgIdx].insert(pair<int,vector<MyDMatch>>(t.queryIdx,vector<MyDMatch>()));
//								newMatches[t.imgIdx][t.queryIdx].push_back(t);
//							}
//						} else {
//							map<int,vector<MyDMatch>> tmp;
//							tmp.insert(pair<int,vector<MyDMatch>>(t.queryIdx,vector<MyDMatch>()));
//							newMatches.insert(pair<int,map<int,vector<MyDMatch>>>(t.imgIdx,tmp));
//							newMatches[t.imgIdx][t.queryIdx].push_back(t);
//						}
//				}
//		}
//
//		return newMatches;
//}
//
//template<class ImType>
//ObjectScene LSHMatching<ImType>::buildObjSceneCorr(vector<MyDMatch> matches){
//	vector<Point2f> train,scene;
//
//	for(vector<MyDMatch>::iterator i = matches.begin();
//		i != matches.end(); ++i){
//			train.push_back(i->train_kp.pt);
//			scene.push_back(i->query_kp.pt);
//	}
//
//	return make_pair(train,scene);
//}
//
//template<class ImType>
//FilteredRes LSHMatching<ImType>::filterMatchingImages(KNNRes &matches){
//		ImgMatches imgIndex;
//		KNNRes imgMatches;
//		vector<vector<MyDMatch>> threshmatch;
//		if(mParams.pointdiff_maxDist >= 1.0f) return make_pair(false,imgIndex);
//
//		if(!matches.empty()){
//			imgMatches = matches;
//
//			//Filters the feature matches and constructs the final form of the structure storing the matches
//			for(map<int,map<int,vector<MyDMatch>>>::iterator i = imgMatches.begin();
//				i != imgMatches.end(); ++i){
//					for(map<int,vector<MyDMatch>>::iterator j = i->second.begin();
//						j != i->second.end(); ++j){
//							if(j->second.size() >= 2){
//								MyDMatch first = j->second[0];
//								MyDMatch second = j->second[1];
//								double dist = first.distance/second.distance;
//								//if the distances ratio pass the test then add the match
//								if(dist <= mParams.pointdiff_maxDist){
//									if(imgIndex.find(first.imgIdx) != imgIndex.end()){
//										imgIndex[first.imgIdx].push_back(first);
//									} else {
//										imgIndex.insert(pair<int,vector<MyDMatch>>(first.imgIdx,vector<MyDMatch>()));
//										imgIndex[first.imgIdx].push_back(first);
//									}
//								}
//							} else {
//								if(imgIndex.find(j->second[0].imgIdx) != imgIndex.end()){
//									imgIndex[j->second[0].imgIdx].push_back(j->second[0]);
//								} else {
//									imgIndex.insert(pair<int,vector<MyDMatch>>(j->second[0].imgIdx,vector<MyDMatch>()));
//									imgIndex[j->second[0].imgIdx].push_back(j->second[0]);
//								}
//							}
//
//					}
//			}
//
//			int mostBins = 0, secondMost = 0, thirdMost = 0,fourthMost=0;
//			for(map<int,vector<MyDMatch>>::iterator i = imgIndex.begin();
//				i != imgIndex.end(); ++i){
//					if(mostBins < i->second.size()) {
//						fourthMost = thirdMost;
//						thirdMost = secondMost;
//						secondMost = mostBins;
//						mostBins = i->second.size();
//					} else if (secondMost < i->second.size()) {
//						fourthMost = thirdMost;
//						thirdMost = secondMost;
//						secondMost = i->second.size();
//					} else if (thirdMost < i->second.size()){
//						fourthMost = thirdMost;
//						thirdMost = i->second.size();
//					} else if (fourthMost < i->second.size()){
//						fourthMost = i->second.size();
//					}
//			}
//
//			ImgMatches ret;
//			for(map<int,vector<MyDMatch>>::iterator i = imgIndex.begin();
//				i != imgIndex.end(); ++i){
//					if(i->second.size() >= secondMost) ret.insert(*i);
//			}
//
//			return make_pair(true,ret);
//		}
//
//		return make_pair(false,imgIndex);
//}
//
//template<class ImType>
//int LSHMatching<ImType>::verify(ImgMatches &matches,ImageProvider<ImType> &db,ImageContainer &query){
//
//		if(matches.size() == 0) return -1;
//
//		FundRes fundamentals = buildFundimentalMat(matches);//buildHomographies(matches);
//
//		double best_fit = 0,second_best = 0;
//		int img = ERROR;
//
//		map<int,double> image_inliers;
//
//		//Find inliesrs to the fundamentals
//		for(FundRes::iterator i = fundamentals.begin();
//			i != fundamentals.end(); ++i){
//
//				int mean = sumInliers(i->second.second);
//
//				if(best_fit < mean){
//					second_best = best_fit;
//					best_fit = mean;
//				} else if (second_best < mean) {
//					second_best = mean;
//				}
//				image_inliers.insert(make_pair(i->first,mean));
//		}
//
//		//Remove all images with less than the second best number of inliers
//		map<int,ImageContainer*> better_matches;
//		for(map<int,double>::iterator i = image_inliers.begin(); i != image_inliers.end(); ++i){
//			if(i->second >= second_best){
//				better_matches.insert(make_pair(i->first,db.getImage(index_dbindex[i->first])));
//			}
//		}
//
//		//Rematch the images
//		ImgMatches matches2 = doubleCheckMatches(better_matches,query);
//
//		//Find fundamental matricies
//		FundRes fundimentals = buildFundimentalMat(matches2);
//
//		best_fit = 0;
//
//		int mean_corr = 0;
//
//		//Find the image with the best number of inliers
//		for(FundRes::iterator i = fundimentals.begin();
//			i != fundimentals.end(); ++i){
//
//				int mean = sumInliers(i->second.second);
//
//				if(best_fit < mean){
//					best_fit = mean;
//					img = i->first;
//				}
//		}
//
//		return (best_fit >= mParams.inlierThresh ? img : ERROR);
//}
//
//template<class ImType>
//FundRes LSHMatching<ImType>::buildFundimentalMat(ImgMatches matches){
//	FundRes ret;
//
//	//Goes through each db image and finds the fundamental matrix between each image and query image
//	for(ImgMatches::iterator i = matches.begin(); 
//		i != matches.end(); ++i){
//			Fundimental p = findFund(buildObjSceneCorr(i->second));
//			ret.insert(make_pair(i->first,p));
//	}
//	return ret;
//}
//
//template<class ImType>
//Fundimental LSHMatching<ImType>::findFund(ObjectScene train_scene){
//	vector<unsigned char> inliers(train_scene.second.size());
//
//	Mat fund = findFundamentalMat(train_scene.first, //db image points
//		train_scene.second, //query image points
//		CV_FM_RANSAC, //Algorithm to use
//		mParams.fund_f_maxDist, //Max error distance after transformation
//		mParams.confidence, //confidence in the resulting matrix
//		inliers); //stores found inliers
//
//	return pair<Mat,vector<unsigned int>>(fund,vector<unsigned int>(inliers.begin(),inliers.end()));
//}
//
//template<class ImType>
//ImgMatches LSHMatching<ImType>::doubleCheckMatches(map<int,ImageContainer*> &db, ImageContainer &query, double min_dist){
//	BFMatcher tempm(NORM_HAMMING2,mParams.compactResults);
//
//	ImgMatches newMatches;
//
//	for(map<int,ImageContainer*>::iterator i = db.begin(); i != db.end(); ++i){
//
//		vector<DMatch> received,good;
//		tempm.match(query.getDescriptor(),i->second->getDescriptor(),received);
//
//		newMatches.insert(make_pair(i->first,convertDMatch(received,*(i->second),query,i->first)));
//
//	}
//	return newMatches;
//}
//
//template<class ImType>
//vector<MyDMatch> LSHMatching<ImType>::convertDMatch(vector<DMatch> matches,
//	ImageContainer &train, ImageContainer &query, int train_idx){
//		vector<MyDMatch> newMatches;
//		for(vector<DMatch>::iterator j = matches.begin();
//			j != matches.end(); ++j){
//				MyDMatch t(*j);
//				t.imgIdx = train_idx;
//				t.train_kp = train.getKeyPoint(j->trainIdx);
//				t.query_kp = query.getKeyPoint(j->queryIdx);
//				newMatches.push_back(t);
//		}
//
//		return newMatches;
//}
//
//template<class ImType>
//int LSHMatching<ImType>::sumInliers(vector<unsigned int> inliers){
//	int b = 0,index=0;
//	for(vector<unsigned int>::iterator i = inliers.begin();
//		i != inliers.end(); ++i){
//			b += *i;
//	}
//	return b;
//}