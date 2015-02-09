#include "MapMatching.h"

template <typename ImType>
int MapMatching<ImType>::find(ImageContainer& query, ImageProvider<ImType> &db){
	KNNRes matches;
	if (knnMatch(query, db, matches)) {
		FilteredRes filteredImgs = filterMatchingImages(matches);
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
int MapMatching<ImType>::find(ImageContainer& query, ImageProvider<ImType> &db, NearImages &r){
	KNNRes matches;
	if (knnMatch(query, db, matches)) {
		FilteredRes filteredImgs = filterMatchingImages(matches);
		if (filteredImgs.first){
			return verify(filteredImgs.second, db, query, r);
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
bool MapMatching<ImType>::knnMatch(ImageContainer &query, ImageProvider<ImType> &db,
	KNNRes &match, vector<Mat> &masks = vector<Mat>()){
	vector<vector<DMatch>> m;
	const Mat des = query.getDescriptor();
	lshMatcher.knnMatch(des, m, mParams.k);
	//Converts DMatch to MyDMatch
	match = convertDMatch(m, db, query);
	return match.size() > 0;
}

template <typename ImType>
KNNRes MapMatching<ImType>::convertDMatch(vector<vector<DMatch>> matches,
	ImageProvider<ImType> &db, ImageContainer &query){
	KNNRes newMatches;

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
KNNRes MapMatching<ImType>::convertDMatch(vector<vector<DMatch>> matches,
	map<int, ImType> &db, map<int, int> tmpdb_to_db, ImageContainer &query){
	KNNRes newMatches;

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
KNNRes MapMatching<ImType>::convertDMatch(map<int, vector<vector<DMatch>>> &matches,
	map<int, ImType> &db, ImageContainer &query){
	KNNRes newMatches;

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
vector<MyDMatch> MapMatching<ImType>::convertDMatch(vector<DMatch> matches,
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
ObjectScene MapMatching<ImType>::buildObjSceneCorr(vector<MyDMatch> matches){
	vector<Point2f> train, scene;

	for (vector<MyDMatch>::iterator i = matches.begin();
		i != matches.end(); ++i){
		train.push_back(i->train_kp.pt);
		scene.push_back(i->query_kp.pt);
	}

	return make_pair(train, scene);
}

template <typename ImType>
FilteredRes MapMatching<ImType>::filterMatchingImages(KNNRes &matches){
	ImgMatches imgIndex;
	KNNRes imgMatches;
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
					imgIndex[j->second[0].imgIdx].push_back(match);
				}
				else {
					imgIndex.insert(pair<int, vector<MyDMatch>>(match.imgIdx, vector<MyDMatch>()));
					imgIndex[j->second[0].imgIdx].push_back(match);
				}
			}
		}

		ImgMatches geoFiltered = geometricFiltering(imgIndex, mParams.geo_k);

		size_t mostBins = 0, secondMost = 0, thirdMost = 0, fourthMost = 0;
		for (ImgMatches::iterator i = geoFiltered.begin();
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

		ImgMatches ret;
		for (map<int, vector<MyDMatch>>::iterator i = geoFiltered.begin();
			i != geoFiltered.end(); ++i){
			if (i->second.size() >= secondMost) ret.insert(*i);
		}

		return make_pair(true, ret);
	}

	return make_pair(false, imgIndex);
}

template <typename ImType>
ImgMatches MapMatching<ImType>::geometricFiltering(ImgMatches &im, int k){

	k = (k >= 4 ? k : 4);
	int kClosestThresh = k*0.75;

	ImgMatches newMatches;

	map<int, double> trackerDefault;

	for (int i_k = 0; i_k < k; ++i_k){
		trackerDefault.insert(make_pair(i_k, 100000.0));
	}

	map<int, double> distTrakerq, distTrakerdb;
	map<int, MyDMatch> kClosestq, kClosestdb;

	for (ImgMatches::iterator i = im.begin(); i != im.end(); ++i){
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
int MapMatching<ImType>::findMinValueIndex(map<int, double> &indexMinMap, double value, int k){
	for (int i = 0; i < k; ++i){
		if (value < indexMinMap[i]){
			return i;
		}
	}
	return ERROR;
}

template <typename ImType>
void MapMatching<ImType>::insertClosestNeighbor(int k, double value, MyDMatch &m, map<int, double> &indexMinMap,
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
double MapMatching<ImType>::getDist(KeyPoint &kp1, KeyPoint &kp2){
	double interior = pow(kp1.pt.x - kp2.pt.x, 2) + pow(kp1.pt.y - kp2.pt.y, 2);
	return sqrt(interior);
}

template <typename ImType>
int MapMatching<ImType>::verify(ImgMatches &matches, ImageProvider<ImType> &db, ImageContainer &query){

	if (matches.size() == 0) return -1;

	FundRess fundamentals = buildFundimentalMat(matches);

	double best_fit = 0, second_best = 0;
	int img = ERROR;

	map<int, double> image_inliers;

#if INSPECT
	//Find inliesrs to the fundamentals
	for (FundRes::iterator i = fundamentals.begin();
		i != fundamentals.end(); ++i){
		vector <MyDMatch> tmp;
		int mean = sumInliers(matches[i->first], i->second.second, tmp);

		if (best_fit < mean){
			showMatches(db[i->first], query, tmp, i->second.first);
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
	for (FundRess::iterator i = fundamentals.begin();
		i != fundamentals.end(); ++i){

		int mean = sumInliers(i->second.second);

		if (best_fit < mean){
			second_best = best_fit;
			best_fit = mean;
			img = i->first;
		}
		else if (second_best < mean) {
			second_best = mean;
		}
		image_inliers.insert(make_pair(i->first, mean));
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
	ImgMatches matches2 = doubleCheckMatches(better_matches, query);

	//Find fundamental matricies
	FundRess fundimentals = buildFundimentalMat(matches2);

	best_fit = 0;

#if INSPECT
	//Find the image with the best number of inliers
	for (FundRess::iterator i = fundimentals.begin();
		i != fundimentals.end(); ++i){
		vector<MyDMatch> tmp;
		int mean = sumInliers(matches2[i->first], i->second.second, tmp);

		if (best_fit < mean){
			showMatches(db[i->first], query, tmp, i->second.first, true);
			best_fit = mean;
			img = i->first;
		}
	}
#else
	//Find the image with the best number of inliers
	for (FundRess::iterator i = fundimentals.begin();
		i != fundimentals.end(); ++i){

		int mean = sumInliers(i->second.second);

		if (best_fit < mean){
			best_fit = mean;
			img = i->first;
		}
	}
#endif

	return (best_fit >= mParams.inlierThresh ? img : ERROR);
}

template <typename ImType>
int MapMatching<ImType>::verify(ImgMatches &matches, ImageProvider<ImType> &db, ImageContainer &query, NearImages &r){

	if (matches.size() == 0) return -1;

	NearImages tempR;

	Rat tempr;

	map<int, Rat> temp_R;

	FundRess fundamentals = buildFundimentalMat(matches);

	double best_fit = 0, second_best = 0,third_best=0;
	int img = ERROR;

	map<int, double> image_inliers;

#if INSPECT
	//Find inliesrs to the fundamentals
	for (FundRess::iterator i = fundamentals.begin();
		i != fundamentals.end(); ++i){
		vector <MyDMatch> tmp;
		int mean = sumInliers(matches[i->first], i->second.second, tmp);

		if (best_fit < mean){
			showMatches(db[i->first], query, tmp, i->second.first);
			inspectEpipole(db[i->first], query, tmp, i->second.first);
			tempR = findRandT(i->second.first, tmp[0], db[i->first].imageSize());
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
	for (FundRess::iterator i = fundamentals.begin();
		i != fundamentals.end(); ++i){
		vector <MyDMatch> tmp;
		int mean = sumInliers(matches[i->first], i->second.second, tmp);

		if (best_fit < mean){
			temp_R.insert(make_pair(i->first, findRandT(i->second.first, tmp[0], db[i->first].imageSize())));
			third_best = second_best;
			second_best = best_fit;
			best_fit = mean;
			img = i->first;
		}
		else if (second_best < mean) {
			temp_R.insert(make_pair(i->first,findRandT(i->second.first, tmp[0], db[i->first].imageSize())));
			third_best = second_best;
			second_best = mean;
		} else if (third_best < mean) {
			temp_R.insert(make_pair(i->first, findRandT(i->second.first, tmp[0], db[i->first].imageSize())));
			third_best = mean;
		}
		image_inliers.insert(make_pair(i->first, mean));
	}
#endif
	
	//Remove all images with less than the second best number of inliers
	for (map<int, double>::iterator i = image_inliers.begin(); i != image_inliers.end(); ++i){
		if (i->second >= third_best && tempR.size() < 3){
			tempR.push_back(make_pair(db[i->first].getIndex(), temp_R[i->first]));
		}
	}

	r = tempR;
	return img;
//	if (best_fit >= mParams.inlierThresh){
//		return img;
//	}
//
//	img = ERROR;
//	//Remove all images with less than the second best number of inliers
//	map<int, ImType> better_matches;
//	for (map<int, double>::iterator i = image_inliers.begin(); i != image_inliers.end(); ++i){
//		if (i->second >= second_best){
//			better_matches.insert(make_pair(i->first, db[i->first]));
//		}
//	}
//
//	//Rematch the images
//	ImgMatches matches2 = doubleCheckMatches(better_matches, query);
//
//	//Find fundamental matricies
//	FundRess fundimentals = buildFundimentalMat(matches2);
//
//	best_fit = 0;
//
//#if INSPECT
//	//Find the image with the best number of inliers
//	for (FundRess::iterator i = fundimentals.begin();
//		i != fundimentals.end(); ++i){
//		vector<MyDMatch> tmp;
//		int mean = sumInliers(matches2[i->first], i->second.second, tmp);
//
//		if (best_fit < mean){
//			tempR = findRandT(i->second.first, tmp[0], db[i->first].imageSize());
//			showMatches(db[i->first], query, tmp, i->second.first, true);
//			best_fit = mean;
//			img = i->first;
//		}
//	}
//#else
//	//Find the image with the best number of inliers
//	for (FundRess::iterator i = fundimentals.begin();
//		i != fundimentals.end(); ++i){
//		vector <MyDMatch> tmp;
//		int mean = sumInliers(matches[i->first], i->second.second, tmp);
//
//		if (best_fit < mean){
//			tempr = findRandT(i->second.first, tmp[0], db[i->first].imageSize());
//			best_fit = mean;
//			img = i->first;
//		}
//	}
//#endif
	//r = (best_fit >= mParams.inlierThresh ? tempr : r);
	//return (best_fit >= mParams.inlierThresh ? img : ERROR);
}

template <typename ImType>
Rat MapMatching<ImType>::findRandT(Mat F, MyDMatch &dm, Size s){
	//Based on camera rebel t2i 
	Matx33d temp_k(2789.596, 0, s.width / 2,
		0, 2789.596, s.height / 2,
		0, 0, 1);
	Mat k(temp_k);
	Mat E = k.t()*F*k; //Essential matrix

	//Singular Value Decomposition(SVD) of the essential matrix to get rotation and
	//Translation matricies
	SVD svd(E.clone(), SVD::MODIFY_A);
	Matx33d w(0, -1, 0,
		1, 0, 0,
		0, 0, 1);
	Mat_<double> R1 = svd.u*Mat(w)*svd.vt;
	if (determinant(R1) < 0){
		R1 = -1 * R1;
	}
	Mat_<double> t1 = svd.u.col(2);
	Mat_<double> R2 = svd.u*Mat(w).t()*svd.vt;
	if (determinant(R2) < 0){
		R2 = -1 * R2;
	}
	Mat_<double> t2 = -svd.u.col(2);

	//This froms the reference matrix for the camera of the database image
	Matx34d M1_T(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0);

	Mat M1(M1_T);
	vector<Mat> M2s;

	//Matricies represent the H matrix from database camera to the query camera
	Matx44d M2_11(R1(0, 0), R1(0, 1), R1(0, 2), t1(0),
		R1(1, 0), R1(1, 1), R1(1, 2), t1(1),
		R1(2, 0), R1(2, 1), R1(2, 2), t1(2),
		0, 0, 0, 1);
	M2s.push_back(Mat(M2_11));

	Matx44d M2_12(R1(0, 0), R1(0, 1), R1(0, 2), t2(0),
		R1(1, 0), R1(1, 1), R1(1, 2), t2(1),
		R1(2, 0), R1(2, 1), R1(2, 2), t2(2),
		0, 0, 0, 1);
	M2s.push_back(Mat(M2_12).clone());

	Matx44d M2_21(R2(0, 0), R2(0, 1), R2(0, 2), t1(0),
		R2(1, 0), R2(1, 1), R2(1, 2), t1(1),
		R2(2, 0), R2(2, 1), R2(2, 2), t1(2),
		0, 0, 0, 1);
	M2s.push_back(Mat(M2_21));

	Matx44d M2_22(R2(0, 0), R2(0, 1), R2(0, 2), t2(0),
		R2(1, 0), R2(1, 1), R2(1, 2), t2(1),
		R2(2, 0), R2(2, 1), R2(2, 2), t2(2),
		0, 0, 0, 1);
	M2s.push_back(Mat(M2_22));

	//The points to test the H matrix
	vector<Point2f> q, d;
	q.push_back(dm.query_kp.pt);
	d.push_back(dm.train_kp.pt);

	int index = 0;

	for (vector<Mat>::iterator i = M2s.begin(); i != M2s.end(); ++i){
		Mat M2((*i), Range(0, 3), Range(0, 4));//Recovers a 3x4 matrix from i
		Mat recon;
		triangulatePoints(k*M1, k*M2, d, q, recon);//recontstructs the 3d points

		//Inversts H from camera 1 to 2 to 2 to 1
		Mat M2_i = i->inv();

		Mat_<double> P1est = recon.col(0);//The db point 
		Mat_<double> P2est = M2_i*P1est;//the db point translated to the query camera

		//if both points are in front of both cameras then we found the rotation and translation
		if (P1est(2) > 0 && P2est(2) > 0){
			break;
		}
		++index;
	}

	switch (index){
	case 0:
		return make_pair(R1, t1);
	case 1:
		return make_pair(R1, t2);
	case 2:
		return make_pair(R2, t1);
	case 3:
		return make_pair(R2, t2);
	default:
		return make_pair(Mat(), Mat());
	}
}

template <typename ImType>
FundRess MapMatching<ImType>::buildFundimentalMat(ImgMatches matches){
	FundRess ret;

	//Goes through each db image and finds the fundamental matrix between each image and query image
	for (ImgMatches::iterator i = matches.begin();
		i != matches.end(); ++i){
		Fund p = findFund(buildObjSceneCorr(i->second));
		ret.insert(make_pair(i->first, p));
	}
	return ret;
}

template <typename ImType>
Fund MapMatching<ImType>::findFund(ObjectScene train_scene){
	vector<unsigned char> inliers(train_scene.second.size());

	Mat fund = findFundamentalMat(train_scene.first, //db image points
		train_scene.second, //query image points
		CV_FM_RANSAC, //Algorithm to use
		mParams.fund_f_maxDist, //Max error distance after transformation
		mParams.confidence, //confidence in the resulting matrix
		inliers); //stores found inliers

	return pair<Mat, vector<unsigned int>>(fund, vector<unsigned int>(inliers.begin(), inliers.end()));
}

template <typename ImType>
ImgMatches MapMatching<ImType>::doubleCheckMatches(map<int, ImType> &db, ImageContainer &query, double min_dist){
	BFMatcher tempm(NORM_HAMMING2, mParams.compactResults);

	ImgMatches newMatches;

	if (!mParams.compactResults){
		map<int, vector<vector<DMatch>>> tmpDMatch;

		for (map<int, ImType>::iterator i = db.begin(); i != db.end(); ++i){
			vector<vector<DMatch>> received;
			tempm.knnMatch(query.getDescriptor(), i->second.getDescriptor(), received, mParams.k);

			tmpDMatch.insert(make_pair(i->first, received));
		}

		KNNRes tmpRes = convertDMatch(tmpDMatch, db, query);

		FilteredRes tmpfRes = filterMatchingImages(tmpRes);
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
int MapMatching<ImType>::sumInliers(vector<unsigned int> &inliers){
	int b = 0, index = 0;
	for (vector<unsigned int>::iterator i = inliers.begin();
		i != inliers.end(); ++i){
		b += *i;
	}
	return b;
}

template <typename ImType>
int MapMatching<ImType>::sumInliers(vector<MyDMatch> &matches, vector<unsigned int> &inliers, vector<MyDMatch> &fittingMatches){
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

#if INSPECT
template <typename ImType>
void MapMatching<ImType>::showMatches(ImType &db, ImageContainer &query, vector<MyDMatch> &inliers, Mat F, bool step = false){
	ObjectScene objscene = buildObjSceneCorr(inliers);
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
void MapMatching<ImType>::inspectEpipole(ImType &db, ImageContainer &query, vector<MyDMatch> &inliers, Mat F){
	ObjectScene objscene = buildObjSceneCorr(inliers);
	SVD svd(F, SVD::FULL_UV);
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
		circle(r, Point(ep.at<double>(0, 0), ep.at<double>(0, 1)), 4, Scalar(0, 0, 255), 2);
		imshow("QueryInliers", r);
	}
	else {
		cout << "No Query Image to show" << endl;
	}

	cout << svd.u.col(2) / svd.u.at<double>(2, 2) << endl;
}
#endif