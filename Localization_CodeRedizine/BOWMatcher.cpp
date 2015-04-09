#include "BOWMatcher.h"

#include "LSHMatching.h"

template <typename ImType>
int BOWMatcher<ImType>::find(ImageContainer& query, ImageProvider<ImType> &db){
	BOWKNNRes matches;
	if (knnMatch(query, db, matches)) {
		BOWFilteredRes filteredImgs = filterMatchingImages(matches);
		if (filteredImgs.first){
			return verify(filteredImgs.second, db, query);
		}
		else {
			return ERROR;
		}
	}
	else {
		return ERROR;
	}
}

template <typename ImType>
double BOWMatcher<ImType>::sim(Mat &v_q, Mat &v_d){
	Mat v_ql = v_q.t()*v_q;
	Mat v_dl = v_d.t()*v_d;
	Mat v_l = v_q.t()*v_d;
	double v = v_l.at<double>(0,0);
	double norm_q = v_ql.at<double>(0,0);
	double norm_d = v_dl.at<double>(0,0);
	norm_q = sqrt(norm_q);
	norm_d = sqrt(norm_d);
	double dist  = v/(norm_q*norm_d);
	return dist;
}

template <typename ImType>
bool BOWMatcher<ImType>::knnMatch(ImageContainer &query, ImageProvider<ImType> &db,
	BOWKNNRes &match, vector<Mat> &masks = vector<Mat>()){
	//vector<vector<DMatch>> m;
	Mat des1 = query.getDescriptor();
	Mat desF(des1.rows,des1.cols,CV_32F);
	des1.convertTo(desF,CV_32F);
	const Mat des = desF.clone();
	
#if DEBUG
	clock_t start = clock();
#endif
	//bowMatcher.knnMatch(des, m, mParams.k);
	vector<DMatch> m;
	bowMatcher.match(des, m);
	struct srt{
		bool operator()(const pair<size_t,double> &r, const pair<size_t,double> &l){
			return r.second > l.second;
		}
	} sorter;
	vector<pair<size_t, double>> sims;
	
	map<size_t, int> qFreq;
	for(size_t i =0; i < clusters; ++i){
		qFreq.insert(make_pair(i,0));
	}
	
	double n_d = 0;
	for (vector<DMatch>::iterator i = m.begin(); i != m.end(); ++i){
		qFreq[i->trainIdx] += 1;
		++n_d;
	}

	double N = clusters;
	Mat v_q (N,1,CV_64F); 
	for (map<size_t, int>::iterator i = qFreq.begin(); i != qFreq.end(); ++i){
		double n_id = i->second;
		double N_i = wordFreq[i->first];
		double t = (n_id/n_d)*log(N/N_i);
		v_q.at<double>(i->first,0) = t;
	}

#if INSPECT
	cout << "tf-idf of q: " << v_q << endl;
#endif

	double secondmost = 0, firstmost = 0;
	for(map<size_t, Mat>::iterator i = tf_idf.begin(); i != tf_idf.end(); ++i){
		double s = sim(v_q,i->second);
#if INSPECT
		cout << "Image: " << i->first << ", similarity: " << s << endl;
		if (i->first % 30 == 0){
			system("PAUSE");
		}
#endif
		if(s > firstmost){
			secondmost = firstmost;
			firstmost = s;
		} else if (s > secondmost){
			secondmost = s;
		}
		sims.push_back(make_pair(i->first,s));
	}
	sort(sims.begin(),sims.end(),sorter);

	int n = 10, c = 0;;
	map<int, ImType> images;
	for(vector<pair<size_t, double>>::iterator i = sims.begin(); i != sims.end(); ++i){
		if (c < n){
			images.insert(make_pair(i->first,db[i->first]));
			++c;
		}
	}

	BFMatcher tempm(NORM_HAMMING2, mParams.compactResults);

	map<int, vector<vector<DMatch>>> tmpDMatch;
#if INSPECT
	cvNamedWindow("matches", CV_WINDOW_KEEPRATIO);
	Mat tmp_db, temp_q, temp_R;
#endif

	for (map<int, ImType>::iterator i = images.begin(); i != images.end(); ++i){
		vector<vector<DMatch>> received;
		tempm.knnMatch(query.getDescriptor(), i->second.getDescriptor(), received, mParams.k);
#if INSPECT
		if (db[i->first].loadImage()){
			db[i->first].getMat(tmp_db);
			query.getMat(temp_q);
			drawMatches(temp_q, query.getKeyPoints(), tmp_db, db[i->first].getKeyPoints(), received, temp_R);
			waitKey();
			tmp_db.release();
			temp_q.release();
			temp_R.release();
		}
#endif
		tmpDMatch.insert(make_pair(i->first, received));
	}

	match = convertDMatch(tmpDMatch, images, query);

#if DEBUG
	cout << "BOW Matching time for k=" << mParams.k << ", is " << ((float)(clock() - start)) / CLOCKS_PER_SEC << " sec" << endl;
#endif
	//Converts DMatch to MyDMatch
	//match = convertDMatch(m, db, query);
	return match.size() > 0;
}


template <typename ImType>
BOWKNNRes BOWMatcher<ImType>::convertDMatch(vector<vector<DMatch>> matches,
	ImageProvider<ImType> &db, ImageContainer &query){
	BOWKNNRes newMatches;

	for (vector<vector<DMatch>>::iterator i = matches.begin();
		i != matches.end(); ++i){
		for (vector<DMatch>::iterator j = (*i).begin();
			j != (*i).end(); ++j){
			MyDMatch t(*j);
			t.train_kp = db[j->imgIdx].getKeyPoint(j->trainIdx);
			t.query_kp = query.getKeyPoint(j->queryIdx);
			if (newMatches.find(t.imgIdx) != newMatches.end()){
				if (newMatches[t.imgIdx].find(t.queryIdx) != newMatches[t.imgIdx].end()){
					newMatches[t.imgIdx][t.queryIdx].push_back(t);
				}
				else {
					newMatches[t.imgIdx].insert(pair<int, vector<MyDMatch>>(t.queryIdx, vector<MyDMatch>()));
					newMatches[t.imgIdx][t.queryIdx].push_back(t);
				}
			}
			else {
				map<int, vector<MyDMatch>> tmp;
				tmp.insert(pair<int, vector<MyDMatch>>(t.queryIdx, vector<MyDMatch>()));
				newMatches.insert(pair<int, map<int, vector<MyDMatch>>>(t.imgIdx, tmp));
				newMatches[t.imgIdx][t.queryIdx].push_back(t);
			}
		}
	}

	return newMatches;
}

template <typename ImType>
BOWKNNRes BOWMatcher<ImType>::convertDMatch(vector<vector<DMatch>> matches,
	map<int, ImType> &db, map<int, int> tmpdb_to_db, ImageContainer &query){
	BOWKNNRes newMatches;

	for (vector<vector<DMatch>>::iterator i = matches.begin();
		i != matches.end(); ++i){
		for (vector<DMatch>::iterator j = (*i).begin();
			j != (*i).end(); ++j){
			DMatch m = *j;
			m.imgIdx = tmpdb_to_db[j->imgIdx];
			MyDMatch t(m);
			t.train_kp = db[t.imgIdx].getKeyPoint(j->trainIdx);
			t.query_kp = query.getKeyPoint(j->queryIdx);
			if (newMatches.find(t.imgIdx) != newMatches.end()){
				if (newMatches[t.imgIdx].find(t.queryIdx) != newMatches[t.imgIdx].end()){
					newMatches[t.imgIdx][t.queryIdx].push_back(t);
				}
				else {
					newMatches[t.imgIdx].insert(pair<int, vector<MyDMatch>>(t.queryIdx, vector<MyDMatch>()));
					newMatches[t.imgIdx][t.queryIdx].push_back(t);
				}
			}
			else {
				map<int, vector<MyDMatch>> tmp;
				tmp.insert(pair<int, vector<MyDMatch>>(t.queryIdx, vector<MyDMatch>()));
				newMatches.insert(pair<int, map<int, vector<MyDMatch>>>(t.imgIdx, tmp));
				newMatches[t.imgIdx][t.queryIdx].push_back(t);
			}
		}
	}

	return newMatches;
}

template <typename ImType>
BOWKNNRes BOWMatcher<ImType>::convertDMatch(map<int, vector<vector<DMatch>>> &matches,
	map<int, ImType> &db, ImageContainer &query){
	BOWKNNRes newMatches;

	for (map<int, vector<vector<DMatch>>>::iterator beg = matches.begin(); beg != matches.end(); ++beg){
		for (vector<vector<DMatch>>::iterator i = beg->second.begin();
			i != beg->second.end(); ++i){
			for (vector<DMatch>::iterator j = (*i).begin();
				j != (*i).end(); ++j){
				DMatch m = *j;
				m.imgIdx = beg->first;
				MyDMatch t(m);
				t.train_kp = db[t.imgIdx].getKeyPoint(j->trainIdx);
				t.query_kp = query.getKeyPoint(j->queryIdx);
				if (newMatches.find(t.imgIdx) != newMatches.end()){
					if (newMatches[t.imgIdx].find(t.queryIdx) != newMatches[t.imgIdx].end()){
						newMatches[t.imgIdx][t.queryIdx].push_back(t);
					}
					else {
						newMatches[t.imgIdx].insert(pair<int, vector<MyDMatch>>(t.queryIdx, vector<MyDMatch>()));
						newMatches[t.imgIdx][t.queryIdx].push_back(t);
					}
				}
				else {
					map<int, vector<MyDMatch>> tmp;
					tmp.insert(pair<int, vector<MyDMatch>>(t.queryIdx, vector<MyDMatch>()));
					newMatches.insert(pair<int, map<int, vector<MyDMatch>>>(t.imgIdx, tmp));
					newMatches[t.imgIdx][t.queryIdx].push_back(t);
				}
			}
		}
	}

	return newMatches;
}

template <typename ImType>
vector<MyDMatch> BOWMatcher<ImType>::convertDMatch(vector<DMatch> matches,
	ImageContainer &train, ImageContainer &query, int train_idx){
	vector<MyDMatch> newMatches;
	for (vector<DMatch>::iterator j = matches.begin();
		j != matches.end(); ++j){
		MyDMatch t(*j);
		t.imgIdx = train_idx;
		t.train_kp = train.getKeyPoint(j->trainIdx);
		t.query_kp = query.getKeyPoint(j->queryIdx);
		newMatches.push_back(t);
	}

	return newMatches;
}

template <typename ImType>
BOWObjectScene BOWMatcher<ImType>::buildObjSceneCorr(vector<MyDMatch> matches){
	vector<Point2f> train, scene;

	for (vector<MyDMatch>::iterator i = matches.begin();
		i != matches.end(); ++i){
		train.push_back(i->train_kp.pt);
		scene.push_back(i->query_kp.pt);
	}

	return make_pair(train, scene);
}

template <typename ImType>
BOWFilteredRes BOWMatcher<ImType>::filterMatchingImages(BOWKNNRes &matches){
	BOWImgMatches imgIndex;
	BOWKNNRes imgMatches;
	vector<vector<MyDMatch>> threshmatch;
	if (mParams.pointdiff_maxDist >= 1.0f) return make_pair(false, imgIndex);

	if (!matches.empty()){
		imgMatches = matches;

		//Filters the feature matches and constructs the final form of the structure storing the matches
		for (map<int, map<int, vector<MyDMatch>>>::iterator i = imgMatches.begin();
			i != imgMatches.end(); ++i){
			for (map<int, vector<MyDMatch>>::iterator j = i->second.begin();
				j != i->second.end(); ++j){
				MyDMatch match = j->second[0];
				if (j->second.size() >= 2){
					MyDMatch first = j->second[0];
					MyDMatch second = j->second[1];
					double dist = first.distance / second.distance;
					//if the distances ratio pass the test then add the match
					if (dist <= mParams.pointdiff_maxDist){
						match = first;
					}
					else {
						continue;
					}
				}
				if (imgIndex.find(match.imgIdx) != imgIndex.end()){
					imgIndex[match.imgIdx].push_back(match);
				}
				else {
					imgIndex.insert(pair<int, vector<MyDMatch>>(match.imgIdx, vector<MyDMatch>()));
					imgIndex[match.imgIdx].push_back(match);
				}
			}
		}

		BOWImgMatches geoFiltered = geometricFiltering(imgIndex, mParams.geo_k);

		size_t mostBins = 0, secondMost = 0, thirdMost = 0, fourthMost = 0;
		for (BOWImgMatches::iterator i = geoFiltered.begin();
			i != geoFiltered.end(); ++i){
			if (mostBins < i->second.size()) {
				fourthMost = thirdMost;
				thirdMost = secondMost;
				secondMost = mostBins;
				mostBins = i->second.size();
			}
			else if (secondMost < i->second.size()) {
				fourthMost = thirdMost;
				thirdMost = secondMost;
				secondMost = i->second.size();
			}
			else if (thirdMost < i->second.size()){
				fourthMost = thirdMost;
				thirdMost = i->second.size();
			}
			else if (fourthMost < i->second.size()){
				fourthMost = i->second.size();
			}
		}

		BOWImgMatches ret;
		for (map<int, vector<MyDMatch>>::iterator i = geoFiltered.begin();
			i != geoFiltered.end(); ++i){
			if (i->second.size() >= secondMost) ret.insert(*i);
		}

		return make_pair(true, ret);
	}

	return make_pair(false, imgIndex);
}

template <typename ImType>
BOWImgMatches BOWMatcher<ImType>::geometricFiltering(BOWImgMatches &im, int k){

	k = (k >= 4 ? k : 4);
	int kClosestThresh = k*0.75;

	BOWImgMatches newMatches;

	map<int, double> trackerDefault;

	for (int i_k = 0; i_k < k; ++i_k){
		trackerDefault.insert(make_pair(i_k, 100000.0));
	}

	map<int, double> distTrakerq, distTrakerdb;
	map<int, MyDMatch> kClosestq, kClosestdb;

	for (BOWImgMatches::iterator i = im.begin(); i != im.end(); ++i){
		for (vector<MyDMatch>::iterator ref = i->second.begin(); ref != i->second.end(); ++ref){
			kClosestdb.clear();
			kClosestq.clear();
			distTrakerq = trackerDefault;
			distTrakerdb = trackerDefault;
			

			double distQ, distDB;

			//Finds the k closest points to the reference query and database keypoint
			for (vector<MyDMatch>::iterator comp = i->second.begin(); comp != i->second.end(); ++comp){
				if (*comp == *ref) continue;
				distQ = getDist(ref->query_kp, comp->query_kp);
				distDB = getDist(ref->train_kp, comp->train_kp);

				/*int indexQ = findMinValueIndex(distTrakerq, distQ, k);
				int indexDB = findMinValueIndex(distTrakerdb, distDB, k);*/

				insertClosestNeighbor(k, distQ, *comp, distTrakerq, kClosestq);
				insertClosestNeighbor(k, distDB, *comp, distTrakerdb, kClosestdb);
			}

			vector<MyDMatch> dbMatch, qMatch;

			int valid_match_count = 0;
			for (int i_kq = 0; i_kq < k; ++i_kq){
				for (int i_kdb = 0; i_kdb < k; ++i_kdb){
					if (kClosestq[i_kq] == kClosestdb[i_kdb]){
						++valid_match_count;
						break;
					}
				}
			}

			

			//Counts the number of MyDMatch objects that are in both dbMatch and qMatch
			/*for (vector<MyDMatch>::iterator closeQ = qMatch.begin(); closeQ != qMatch.end(); ++closeQ){
				if (std::find(dbMatch.begin(), dbMatch.end(), *closeQ) != dbMatch.end()) ++valid_match_count;
			}*/

			//If number of matched points that are nearest neighbors to the matched reference query 
			//and database keypoint is >= a threshold then keep the match otherwise ignore it 
			if (valid_match_count >= kClosestThresh){
				if (newMatches.find(ref->imgIdx) != newMatches.end()){
					newMatches[ref->imgIdx].push_back(*ref);
				}
				else {
					newMatches.insert(make_pair(ref->imgIdx, vector<MyDMatch>()));
					newMatches[ref->imgIdx].push_back(*ref);
				}
			}
		}
	}
	return newMatches;
}

template <typename ImType>
int BOWMatcher<ImType>::findMinValueIndex(map<int, double> &indexMinMap, double value, int k){
	for (int i = 0; i < k; ++i){
		if (value < indexMinMap[i]){
			return i;
		}
	}
	return ERROR;
}

template <typename ImType>
void BOWMatcher<ImType>::insertClosestNeighbor(int k, double value, MyDMatch &m, map<int, double> &indexMinMap,
	map<int, MyDMatch> &nearestNeighborMap){
	MyDMatch tmpM;
	double tmpV;
	int index = 0;
	bool found = false;
	for (int i = 0; i < k; ++i){
		if (!found){
			if (value < indexMinMap[i]){
				found = true;
				index = i;
			}
		}
		if (found){
			if (i == index){
				tmpM = nearestNeighborMap[i];
				tmpV = indexMinMap[i];
				nearestNeighborMap[i] = m;
				indexMinMap[i] = value;
			}
			else {
				MyDMatch tmp_m = nearestNeighborMap[i];
				nearestNeighborMap[i] = tmpM;
				tmpM = tmp_m;
				double tmp_v = indexMinMap[i];
				indexMinMap[i] = tmpV;
				tmpV = tmp_v;
			}
		}
	}
}

template <typename ImType>
double BOWMatcher<ImType>::getDist(KeyPoint &kp1, KeyPoint &kp2){
	double interior = pow(kp1.pt.x - kp2.pt.x, 2) + pow(kp1.pt.y - kp2.pt.y, 2);
	return sqrt(interior);
}

template <typename ImType>
int BOWMatcher<ImType>::verify(BOWImgMatches &matches, ImageProvider<ImType> &db, ImageContainer &query){

	if (matches.size() == 0) return -1;

	BOWFundRes fundamentals = buildFundimentalMat(matches);

	double best_fit = 0, second_best = 0;
	int img = ERROR;

	map<int, double> image_inliers;

#if INSPECT
	//Find inliesrs to the fundamentals
	for (BOWFundRes::iterator i = fundamentals.begin();
		i != fundamentals.end(); ++i){
		vector <MyDMatch> tmp;
		int mean = sumInliers(matches[i->first], i->second.second, tmp);
		if (tmp.size() > 0){
			showMatches(db[i->first], query, tmp, i->second.first);
		}
		if (best_fit < mean){
			
			inspectEpipole(db[i->first], query, tmp, i->second.first);
			second_best = best_fit;
			best_fit = mean;
			img = i->first;
		}
		else if (second_best < mean) {
			second_best = mean;
		}
		image_inliers.insert(make_pair(i->first, mean));
	}
#else
	//Find inliesrs to the fundamentals
	for (BOWFundRes::iterator i = fundamentals.begin();
		i != fundamentals.end(); ++i){

		if (best_fit < i->second.second){
			second_best = best_fit;
			best_fit = i->second.second;
			img = i->first;
		}
		else if (second_best < i->second.second) {
			second_best = i->second.second;
		}
		image_inliers.insert(make_pair(i->first, i->second.second));
	}
#endif

#if INSPECT
	if(img >= 0){
		showEpilines(db[img],query,fundamentals[img].first);
	}
#endif
	if (best_fit >= mParams.inlierThresh){
		return img;
	}

	img = ERROR;
	//Remove all images with less than the second best number of inliers
	map<int, ImType> better_matches;
	for (map<int, double>::iterator i = image_inliers.begin(); i != image_inliers.end(); ++i){
		if (i->second >= second_best){
			better_matches.insert(make_pair(i->first, db[i->first]));
		}
	}

	//Rematch the images
	BOWImgMatches matches2 = doubleCheckMatches(better_matches, query);

	//Find fundamental matricies
	BOWFundRes fundimentals = buildFundimentalMat(matches2);

	best_fit = 0;

#if INSPECT
	//Find the image with the best number of inliers
	for (BOWFundRes::iterator i = fundimentals.begin();
		i != fundimentals.end(); ++i){
		vector<MyDMatch> tmp;
		int mean = sumInliers(matches2[i->first], i->second.second, tmp);

		if (best_fit < mean){
			showMatches(db[i->first], query, tmp, i->second.first, false);
			best_fit = mean;
			img = i->first;
		}
	}
	if(img >= 0){
		showEpilines(db[img],query,fundimentals[img].first);
	}
#else
	//Find the image with the best number of inliers
	for (BOWFundRes::iterator i = fundimentals.begin();
		i != fundimentals.end(); ++i){

		if (best_fit < i->second.second){
			best_fit = i->second.second;
			img = i->first;
		}
	}
#endif

	return (best_fit >= mParams.inlierThresh ? img : ERROR);
}

template <typename ImType>
BOWFundRes BOWMatcher<ImType>::buildFundimentalMat(BOWImgMatches matches){
	BOWFundRes ret;

	//Goes through each db image and finds the fundamental matrix between each image and query image
	for (BOWImgMatches::iterator i = matches.begin();
		i != matches.end(); ++i){
		BOWFundimental p = findFund(buildObjSceneCorr(i->second));
		ret.insert(make_pair(i->first, p));
	}
	return ret;
}

template <typename ImType>
BOWFundimental BOWMatcher<ImType>::findFund(BOWObjectScene train_scene){
	vector<unsigned char> inliers(train_scene.second.size());

	Mat fund = findFundamentalMat(train_scene.first, //db image points
		train_scene.second, //query image points
		CV_FM_RANSAC, //Algorithm to use
		mParams.fund_f_maxDist, //Max error distance after transformation
		mParams.confidence, //confidence in the resulting matrix
		inliers); //stores found inliers

#if INSPECT
	return pair<Mat, vector<unsigned int>>(fund, vector<unsigned int>(inliers.begin(), inliers.end()));
#else
	int sum = 0;
	for (size_t i = 0; i < inliers.size(); ++i){ sum += inliers[i]; }
	return pair<Mat, int>(fund, sum);
#endif
}

template <typename ImType>
BOWImgMatches BOWMatcher<ImType>::doubleCheckMatches(map<int, ImType> &db, ImageContainer &query, double min_dist){
	BFMatcher tempm(NORM_HAMMING2, mParams.compactResults);

	BOWImgMatches newMatches;

	if (!mParams.compactResults){
		map<int, vector<vector<DMatch>>> tmpDMatch;

		for (map<int, ImType>::iterator i = db.begin(); i != db.end(); ++i){
			vector<vector<DMatch>> received;
			tempm.knnMatch(query.getDescriptor(), i->second.getDescriptor(), received, mParams.k);

			tmpDMatch.insert(make_pair(i->first, received));
		}

		BOWKNNRes tmpRes = convertDMatch(tmpDMatch, db, query);

		BOWFilteredRes tmpfRes = filterMatchingImages(tmpRes);
		newMatches = tmpfRes.second;

		return newMatches;
	}
	else {

		for (map<int, ImType>::iterator i = db.begin(); i != db.end(); ++i){
			vector<DMatch> received;
			tempm.match(query.getDescriptor(), i->second.getDescriptor(), received);

			newMatches.insert(make_pair(i->first, convertDMatch(received, i->second, query, i->first)));
		}

		return geometricFiltering(newMatches, 3);
	}
}

template <typename ImType>
int BOWMatcher<ImType>::sumInliers(vector<unsigned int> &inliers){
	int b = 0, index = 0;
	for (vector<unsigned int>::iterator i = inliers.begin();
		i != inliers.end(); ++i){
		b += *i;
	}
	return b;
}

#if INSPECT
template <typename ImType>
int BOWMatcher<ImType>::sumInliers(vector<MyDMatch> &matches, vector<unsigned int> &inliers, vector<MyDMatch> &fittingMatches){
	int b = 0, index = 0;
	for (vector<unsigned int>::iterator i = inliers.begin();
		i != inliers.end(); ++i){
		b += *i;
		if (*i){
			fittingMatches.push_back(matches[index]);
		}
		index++;
	}
	return b;
}
template <typename ImType>
void BOWMatcher<ImType>::showMatches(ImType &db, ImageContainer &query, vector<MyDMatch> &inliers, Mat F, bool step){
	BOWObjectScene objscene = buildObjSceneCorr(inliers);
	namedWindow("DBInliers",CV_WINDOW_KEEPRATIO);
	namedWindow("QueryInliers",CV_WINDOW_KEEPRATIO);
	if (!step){
		if (db.loadImage()){
			Mat r;
			db.getMat(r);
			KeyPoint p;
			for (size_t i = 0; i < inliers.size(); ++i) {
				p = inliers[i].train_kp;

				circle(r, p.pt, 2, Scalar(0, 0, 255), -1);
				char buff[33];
				itoa(i, buff, 10);
				putText(r, buff, Point(p.pt.x + 4, p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
					Scalar(255, 13, 255));
			}
			vector<Vec3f> eplines;
			computeCorrespondEpilines(objscene.second, 2, F, eplines);
			for (vector<Vec3f>::iterator t = eplines.begin(); t != eplines.end(); ++t){
				float y1 = (-1 * ((*t)[2] / (*t)[1]));
				float y2 = (-1 * ((*t)[0] / (*t)[1])*db.imageSize().width) - ((*t)[2] / (*t)[1]);
				Point p1(0, y1), p2(db.imageSize().width, y2);
				line(r, p1, p2, Scalar::all(-1));
			}
			
			imshow("DBInliers", r);
		}
		else {
			cout << "No DB image to show" << endl;
		}
		if (query.hasImage()){
			Mat r;
			query.getMat(r);
			KeyPoint p;
			for (size_t i = 0; i < inliers.size(); ++i) {
				p = inliers[i].query_kp;

				circle(r, p.pt, 2, Scalar(0, 0, 255), -1);
				char buff[33];
				itoa(i, buff, 10);
				putText(r, buff, Point(p.pt.x + 4, p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
					Scalar(255, 13, 255));
			}
			vector<Vec3f> eplines;
			computeCorrespondEpilines(objscene.first, 1, F, eplines);
			for (vector<Vec3f>::iterator t = eplines.begin(); t != eplines.end(); ++t){
				float y1 = (-1 * ((*t)[2] / (*t)[1]));
				float y2 = (-1 * ((*t)[0] / (*t)[1])*query.imageSize().width) - ((*t)[2] / (*t)[1]);
				Point p1(0, y1), p2(query.imageSize().width, y2);
				line(r, p1, p2, Scalar::all(-1));
			}
			imshow("QueryInliers", r);
		}
		else {
			cout << "No Query Image to show" << endl;
		}
	}
	else if (query.hasImage() && db.loadImage()) {
		Mat q, d;

		vector<Vec3f> eplinesdb, eplinesq;
		computeCorrespondEpilines(objscene.first, 1, F, eplinesq);
		computeCorrespondEpilines(objscene.second, 2, F, eplinesdb);
		for (size_t t = 0; t < eplinesdb.size(); ++t){
			db.getMat(d);
			query.getMat(q);

			KeyPoint q_p = inliers[t].query_kp;
			circle(q, q_p.pt, 2, Scalar(0, 0, 255), -1);
			char q_buff[33];
			itoa(t, q_buff, 10);
			putText(q, q_buff, Point(q_p.pt.x + 4, q_p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
				Scalar(255, 13, 255));

			KeyPoint d_p = inliers[t].train_kp;
			circle(d, d_p.pt, 2, Scalar(0, 0, 255), -1);
			char d_buff[33];
			itoa(t, d_buff, 10);
			putText(d, d_buff, Point(d_p.pt.x + 4, d_p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
				Scalar(255, 13, 255));

			float q_y1 = (-1 * (eplinesq[t][2] / eplinesq[t][1]));
			float q_y2 = (-1 * (eplinesq[t][0] / eplinesq[t][1])*query.imageSize().width) - (eplinesq[t][2] / eplinesq[t][1]);
			Point q_p1(0, q_y1), q_p2(query.imageSize().width, q_y2);
			line(q, q_p1, q_p2, Scalar(0, 100, 255));

			float db_y1 = (-1 * (eplinesdb[t][2] / eplinesdb[t][1]));
			float db_y2 = (-1 * (eplinesdb[t][0] / eplinesdb[t][1])*db.imageSize().width) - (eplinesdb[t][2] / eplinesdb[t][1]);
			Point db_p1(0, db_y1), db_p2(db.imageSize().width, db_y2);
			line(d, db_p1, db_p2, Scalar(0, 100, 255));

			imshow("QueryInliers", q);
			imshow("DBInliers", d);
			waitKey();
		}
	}

	waitKey();
}
template <typename ImType>
void BOWMatcher<ImType>::inspectEpipole(ImType &db, ImageContainer &query, vector<MyDMatch> &inliers, Mat F){
	BOWObjectScene objscene = buildObjSceneCorr(inliers);
	SVD svd(F, SVD::FULL_UV);
	namedWindow("DBInliers",CV_WINDOW_KEEPRATIO);
	namedWindow("QueryInliers",CV_WINDOW_KEEPRATIO);
	if (db.loadImage()){
		Mat r;
		db.getMat(r);
		KeyPoint p;
		for (size_t i = 0; i < inliers.size(); ++i) {
			p = inliers[i].train_kp;

			circle(r, p.pt, 2, Scalar(0, 0, 255), -1);
			char buff[33];
			itoa(i, buff, 10);
			putText(r, buff, Point(p.pt.x + 4, p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
				Scalar(255, 13, 255));
		}
		vector<Vec3f> eplines;
		computeCorrespondEpilines(objscene.second, 2, F, eplines);
		for (vector<Vec3f>::iterator t = eplines.begin(); t != eplines.end(); ++t){
			float y1 = (-1 * ((*t)[2] / (*t)[1]));
			float y2 = (-1 * ((*t)[0] / (*t)[1])*db.imageSize().width) - ((*t)[2] / (*t)[1]);
			Point p1(0, y1), p2(db.imageSize().width, y2);
			line(r, p1, p2, Scalar::all(-1));
		}
		imshow("DBInliers", r);
	}
	else {
		cout << "No DB image to show" << endl;
	}
	if (query.hasImage()){
		Mat r;
		query.getMat(r);
		KeyPoint p;
		for (size_t i = 0; i < inliers.size(); ++i) {
			p = inliers[i].query_kp;

			circle(r, p.pt, 2, Scalar(0, 0, 255), -1);
			char buff[33];
			itoa(i, buff, 10);
			putText(r, buff, Point(p.pt.x + 4, p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
				Scalar(255, 13, 255));
		}
		vector<Vec3f> eplines;
		computeCorrespondEpilines(objscene.first, 1, F, eplines);
		for (vector<Vec3f>::iterator t = eplines.begin(); t != eplines.end(); ++t){
			float y1 = (-1 * ((*t)[2] / (*t)[1]));
			float y2 = (-1 * ((*t)[0] / (*t)[1])*query.imageSize().width) - ((*t)[2] / (*t)[1]);
			Point p1(0, y1), p2(query.imageSize().width, y2);
			line(r, p1, p2, Scalar::all(-1));
		}
		Mat ep = svd.u.col(2) / svd.u.at<double>(2, 2);
		ep = ep.t();
		circle(r, Point(ep.at<double>(0, 0), ep.at<double>(0, 1)), 4, Scalar(0, 0, 255), 2);
		imshow("QueryInliers", r);
	}
	else {
		cout << "No Query Image to show" << endl;
	}

	cout << svd.u.col(2) / svd.u.at<double>(2, 2) << endl;

}
template <typename ImType>
void BOWMatcher<ImType>::showEpilines(ImType &db, ImageContainer &query, Mat F){
	if(query.hasImage() && db.loadImage()){
		namedWindow("DBInliers",CV_WINDOW_KEEPRATIO);
		namedWindow("QueryInliers",CV_WINDOW_KEEPRATIO);
		Mat q, d,d2;
		db.getMat(d2);
		vector<Point2f> q_points;
		for(size_t i = 0; i < query.getKeyPoints().size(); ++i){
			q_points.push_back(query.getKeyPoint(i).pt);
		}
		for(size_t i = 0; i < db.getKeyPoints().size(); ++i){
			KeyPoint d_p = db.getKeyPoint(i);
			circle(d2, d_p.pt, 2, Scalar(255, 0, 0), -1);
			/*char d_buff[33];
			itoa(i, d_buff, 10);
			putText(d2, d_buff, Point(d_p.pt.x + 4, d_p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
				Scalar(255, 150, 255));*/
		}

		vector<Vec3f> eplinesdb, eplinesq;
		//computeCorrespondEpilines(objscene.first, 1, F, eplinesq);
		computeCorrespondEpilines(q_points, 2, F, eplinesdb);
		for (size_t t = 0; t < eplinesdb.size(); ++t){
			d2.copyTo(d);
			query.getMat(q);

			KeyPoint q_p = query.getKeyPoint(t);
			circle(q, q_p.pt, 2, Scalar(0, 0, 255), -1);
			char q_buff[33];
			itoa(t, q_buff, 10);
			putText(q, q_buff, Point(q_p.pt.x + 4, q_p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
				Scalar(255, 13, 255));

			/*KeyPoint d_p = inliers[t].train_kp;
			circle(d, d_p.pt, 2, Scalar(0, 0, 255), -1);
			char d_buff[33];
			itoa(t, d_buff, 10);
			putText(d, d_buff, Point(d_p.pt.x + 4, d_p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
			Scalar(255, 13, 255));*/

			/*float q_y1 = (-1 * (eplinesq[t][2] / eplinesq[t][1]));
			float q_y2 = (-1 * (eplinesq[t][0] / eplinesq[t][1])*query.imageSize().width) - (eplinesq[t][2] / eplinesq[t][1]);
			Point q_p1(0, q_y1), q_p2(query.imageSize().width, q_y2);
			line(q, q_p1, q_p2, Scalar(0, 100, 255));*/

			float db_y1 = (-1 * (eplinesdb[t][2] / eplinesdb[t][1]));
			float db_y2 = (-1 * (eplinesdb[t][0] / eplinesdb[t][1])*db.imageSize().width) - (eplinesdb[t][2] / eplinesdb[t][1]);
			Point db_p1(0, db_y1), db_p2(db.imageSize().width, db_y2);
			line(d, db_p1, db_p2, Scalar(0, 0, 255),2);

			imshow("QueryInliers", q);
			imshow("DBInliers", d);
			waitKey();
		}
	}
}
#endif