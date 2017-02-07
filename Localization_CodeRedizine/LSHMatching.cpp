#include "LSHMatching.h"

template <typename ImType>
int LSHMatching<ImType>::find(ImageContainer &_queryImage, ImageProvider<ImType> &_db)
{
	KNNRes matches;
	if (knnMatch(_queryImage, _db, matches)) 
	{
		FilteredRes filteredImgs = filterMatchingImages(matches);
		if (filteredImgs.first)
		{
			return verify(filteredImgs.second, _db, _queryImage);
		}
		else 
		{
			return ERROR;
		}
	}
	else 
	{
		return ERROR;
	}
}

template <typename ImType>
bool LSHMatching<ImType>::knnMatch(ImageContainer &_queryImage, ImageProvider<ImType> &_db,
	KNNRes &_match, vector<Mat> &_masks = vector<Mat>())
{
	vector<vector<DMatch>> m;
	const Mat des = _queryImage.getDescriptor();
#if DEBUG
	clock_t start = clock();
#endif
	lshMatcher.knnMatch(des, m, mParams.k);
#if DEBUG
	cout << "LSH Matching time for k=" << mParams.k << ", is " << ((float)(clock() - start)) / CLOCKS_PER_SEC << " sec" << endl;
#endif
	//Converts DMatch to MyDMatch
	_match = convertDMatch(m, _db, _queryImage);
	return _match.size() > 0;
}


template <typename ImType>
KNNRes LSHMatching<ImType>::convertDMatch(vector<vector<DMatch>> _matches,
	ImageProvider<ImType> &_db, ImageContainer &_queryImage)
{
	KNNRes newMatches;

	for (vector<vector<DMatch>>::iterator imagesMatchList = _matches.begin();
		imagesMatchList != _matches.end(); ++imagesMatchList)
	{
		for (vector<DMatch>::iterator imageMatchList = (*imagesMatchList).begin();
			imageMatchList != (*imagesMatchList).end(); ++imageMatchList)
		{
			MyDMatch match(*imageMatchList);
			match.train_kp = _db[imageMatchList->imgIdx].getKeyPoint(imageMatchList->trainIdx);
			match.query_kp = _queryImage.getKeyPoint(imageMatchList->queryIdx);
			if (newMatches.find(match.imgIdx) != newMatches.end())
			{
				if (newMatches[match.imgIdx].find(match.queryIdx) != newMatches[match.imgIdx].end())
				{
					newMatches[match.imgIdx][match.queryIdx].push_back(match);
				}
				else 
				{
					newMatches[match.imgIdx].insert(pair<int, vector<MyDMatch>>(match.queryIdx, vector<MyDMatch>()));
					newMatches[match.imgIdx][match.queryIdx].push_back(match);
				}
			}
			else 
			{
				map<int, vector<MyDMatch>> tmp;
				tmp.insert(pair<int, vector<MyDMatch>>(match.queryIdx, vector<MyDMatch>()));
				newMatches.insert(pair<int, map<int, vector<MyDMatch>>>(match.imgIdx, tmp));
				newMatches[match.imgIdx][match.queryIdx].push_back(match);
			}
		}
	}

	return newMatches;
}

template <typename ImType>
KNNRes LSHMatching<ImType>::convertDMatch(vector<vector<DMatch>> _matches,
	map<int, ImType> &_db, map<int, int> tmpdb_to_db, ImageContainer &_queryImage)
{
	KNNRes newMatches;

	for (vector<vector<DMatch>>::iterator imagesMatchList = _matches.begin();
		imagesMatchList != _matches.end(); ++imagesMatchList)
	{
		for (vector<DMatch>::iterator imageMatchList = (*imagesMatchList).begin();
			imageMatchList != (*imagesMatchList).end(); ++imageMatchList)
		{
			DMatch dMatch = *imageMatchList;
			dMatch.imgIdx = tmpdb_to_db[imageMatchList->imgIdx];
			MyDMatch match(dMatch);
			match.train_kp = _db[match.imgIdx].getKeyPoint(imageMatchList->trainIdx);
			match.query_kp = _queryImage.getKeyPoint(imageMatchList->queryIdx);
			if (newMatches.find(match.imgIdx) != newMatches.end())
			{
				if (newMatches[match.imgIdx].find(match.queryIdx) != newMatches[match.imgIdx].end())
				{
					newMatches[match.imgIdx][match.queryIdx].push_back(match);
				}
				else 
				{
					newMatches[match.imgIdx].insert(pair<int, vector<MyDMatch>>(match.queryIdx, vector<MyDMatch>()));
					newMatches[match.imgIdx][match.queryIdx].push_back(match);
				}
			}
			else 
			{
				map<int, vector<MyDMatch>> tmp;
				tmp.insert(pair<int, vector<MyDMatch>>(match.queryIdx, vector<MyDMatch>()));
				newMatches.insert(pair<int, map<int, vector<MyDMatch>>>(match.imgIdx, tmp));
				newMatches[match.imgIdx][match.queryIdx].push_back(match);
			}
		}
	}

	return newMatches;
}

template <typename ImType>
KNNRes LSHMatching<ImType>::convertDMatch(map<int, vector<vector<DMatch>>> &_matches,
	map<int, ImType> &_db, ImageContainer &_queryImage){
	KNNRes newMatches;

	for (map<int, vector<vector<DMatch>>>::iterator imagesMatchMap = _matches.begin(); imagesMatchMap != _matches.end(); ++imagesMatchMap)
	{
		for (vector<vector<DMatch>>::iterator imagesMatchList = imagesMatchMap->second.begin();
			imagesMatchList != imagesMatchMap->second.end(); ++imagesMatchList)
		{
			for (vector<DMatch>::iterator imageMatchList = (*imagesMatchList).begin();
				imageMatchList != (*imagesMatchList).end(); ++imageMatchList)
			{
				DMatch dMatch = *imageMatchList;
				dMatch.imgIdx = imagesMatchMap->first;
				MyDMatch match(dMatch);
				match.train_kp = _db[match.imgIdx].getKeyPoint(imageMatchList->trainIdx);
				match.query_kp = _queryImage.getKeyPoint(imageMatchList->queryIdx);
				if (newMatches.find(match.imgIdx) != newMatches.end())
				{
					if (newMatches[match.imgIdx].find(match.queryIdx) != newMatches[match.imgIdx].end())
					{
						newMatches[match.imgIdx][match.queryIdx].push_back(match);
					}
					else 
					{
						newMatches[match.imgIdx].insert(pair<int, vector<MyDMatch>>(match.queryIdx, vector<MyDMatch>()));
						newMatches[match.imgIdx][match.queryIdx].push_back(match);
					}
				}
				else 
				{
					map<int, vector<MyDMatch>> tmp;
					tmp.insert(pair<int, vector<MyDMatch>>(match.queryIdx, vector<MyDMatch>()));
					newMatches.insert(pair<int, map<int, vector<MyDMatch>>>(match.imgIdx, tmp));
					newMatches[match.imgIdx][match.queryIdx].push_back(match);
				}
			}
		}
	}

	return newMatches;
}

template <typename ImType>
vector<MyDMatch> LSHMatching<ImType>::convertDMatch(vector<DMatch> _matches,
	ImageContainer &_trainImage, ImageContainer &_queryImage, int _trainIdx)
{
	vector<MyDMatch> newMatches;

	for (vector<DMatch>::iterator imageMatchList = _matches.begin();
		imageMatchList != _matches.end(); ++imageMatchList)
	{
		MyDMatch match(*imageMatchList);
		match.imgIdx = _trainIdx;
		match.train_kp = _trainImage.getKeyPoint(imageMatchList->trainIdx);
		match.query_kp = _queryImage.getKeyPoint(imageMatchList->queryIdx);
		newMatches.push_back(match);
	}

	return newMatches;
}

template <typename ImType>
ObjectScene LSHMatching<ImType>::buildObjSceneCorr(vector<MyDMatch> _matches)
{
	vector<Point2f> train, scene;

	for (vector<MyDMatch>::iterator imageMatchList = _matches.begin(); imageMatchList != _matches.end(); ++imageMatchList)
	{
		train.push_back(imageMatchList->train_kp.pt);
		scene.push_back(imageMatchList->query_kp.pt);
	}

	return make_pair(train, scene);
}

template <typename ImType>
FilteredRes LSHMatching<ImType>::filterMatchingImages(KNNRes &_matches)
{
	ImgMatches imgIndex;
	KNNRes imgMatches;
	vector<vector<MyDMatch>> threshmatch;
	if (mParams.pointdiff_maxDist >= 1.0f) return make_pair(false, imgIndex);

	if (!_matches.empty())
	{
		imgMatches = _matches;

		//Filters the feature matches and constructs the final form of the structure storing the matches
		for (map<int, map<int, vector<MyDMatch>>>::iterator imagesMatchMap = imgMatches.begin();
			imagesMatchMap != imgMatches.end(); ++imagesMatchMap)
		{
			for (map<int, vector<MyDMatch>>::iterator imageMatchMap = imagesMatchMap->second.begin();
				imageMatchMap != imagesMatchMap->second.end(); ++imageMatchMap)
			{
				MyDMatch match = imageMatchMap->second[0];
				if (imageMatchMap->second.size() >= 2)
				{
					MyDMatch first = imageMatchMap->second[0];
					MyDMatch second = imageMatchMap->second[1];
					double dist = first.distance / second.distance;
					//if the distances ratio pass the test then add the match
					if (dist <= mParams.pointdiff_maxDist)
					{
						match = first;
					}
					else 
					{
						continue;
					}
				}
				if (imgIndex.find(match.imgIdx) != imgIndex.end())
				{
					imgIndex[match.imgIdx].push_back(match);
				}
				else 
				{
					imgIndex.insert(pair<int, vector<MyDMatch>>(match.imgIdx, vector<MyDMatch>()));
					imgIndex[match.imgIdx].push_back(match);
				}
			}
		}

		ImgMatches geoFiltered = geometricFiltering(imgIndex, mParams.geo_k);

		size_t mostBins = 0, secondMost = 0, thirdMost = 0, fourthMost = 0;
		for (ImgMatches::iterator filteredImageMatches = geoFiltered.begin();
			filteredImageMatches != geoFiltered.end(); ++filteredImageMatches)
		{
			if (mostBins < filteredImageMatches->second.size())
			{
				fourthMost = thirdMost;
				thirdMost = secondMost;
				secondMost = mostBins;
				mostBins = filteredImageMatches->second.size();
			}
			else if (secondMost < filteredImageMatches->second.size())
			{
				fourthMost = thirdMost;
				thirdMost = secondMost;
				secondMost = filteredImageMatches->second.size();
			}
			else if (thirdMost < filteredImageMatches->second.size())
			{
				fourthMost = thirdMost;
				thirdMost = filteredImageMatches->second.size();
			}
			else if (fourthMost < filteredImageMatches->second.size())
			{
				fourthMost = filteredImageMatches->second.size();
			}
		}

		ImgMatches ret;
		for (map<int, vector<MyDMatch>>::iterator filteredImageMatches = geoFiltered.begin();
			filteredImageMatches != geoFiltered.end(); ++filteredImageMatches)
		{
			if (filteredImageMatches->second.size() >= secondMost) ret.insert(*filteredImageMatches);
		}

		return make_pair(true, ret);
	}

	return make_pair(false, imgIndex);
}

template <typename ImType>
ImgMatches LSHMatching<ImType>::geometricFiltering(ImgMatches &_imageMatches, int _k){

	_k = (_k >= 4 ? _k : 4);
	int kClosestThresh = _k * 0.75;

	ImgMatches newMatches;

	map<int, double> trackerDefault;

	for (int i_k = 0; i_k < _k; ++i_k)
	{
		trackerDefault.insert(make_pair(i_k, 100000.0));
	}

	map<int, double> distTrakerq, distTrakerdb;
	map<int, MyDMatch> kClosestq, kClosestdb;

	for (ImgMatches::iterator imageMatches = _imageMatches.begin(); imageMatches != _imageMatches.end(); ++imageMatches)
	{
		for (vector<MyDMatch>::iterator refernceMatches = imageMatches->second.begin(); refernceMatches != imageMatches->second.end(); ++refernceMatches)
		{
			kClosestdb.clear();
			kClosestq.clear();
			distTrakerq = trackerDefault;
			distTrakerdb = trackerDefault;
			

			double distQ, distDB;

			//Finds the k closest points to the reference query and database keypoint
			for (vector<MyDMatch>::iterator comparisonMatches = imageMatches->second.begin(); comparisonMatches != imageMatches->second.end(); ++comparisonMatches)
			{
				if (*comparisonMatches == *refernceMatches) continue;
				distQ = getDist(refernceMatches->query_kp, comparisonMatches->query_kp);
				distDB = getDist(refernceMatches->train_kp, comparisonMatches->train_kp);

				/*int indexQ = findMinValueIndex(distTrakerq, distQ, k);
				int indexDB = findMinValueIndex(distTrakerdb, distDB, k);*/

				insertClosestNeighbor(_k, distQ, *comparisonMatches, distTrakerq, kClosestq);
				insertClosestNeighbor(_k, distDB, *comparisonMatches, distTrakerdb, kClosestdb);
			}

			vector<MyDMatch> dbMatch, qMatch;

			int valid_match_count = 0;
			for (int i_kq = 0; i_kq < _k; ++i_kq)
			{
				for (int i_kdb = 0; i_kdb < _k; ++i_kdb)
				{
					if (kClosestq[i_kq] == kClosestdb[i_kdb])
					{
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
			if (valid_match_count >= kClosestThresh)
			{
				if (newMatches.find(refernceMatches->imgIdx) != newMatches.end())
				{
					newMatches[refernceMatches->imgIdx].push_back(*refernceMatches);
				}
				else 
				{
					newMatches.insert(make_pair(refernceMatches->imgIdx, vector<MyDMatch>()));
					newMatches[refernceMatches->imgIdx].push_back(*refernceMatches);
				}
			}
		}
	}
	return newMatches;
}

template <typename ImType>
int LSHMatching<ImType>::findMinValueIndex(map<int, double> &indexMinMap, double value, int k){
	for (int i = 0; i < k; ++i){
		if (value < indexMinMap[i]){
			return i;
		}
	}
	return ERROR;
}

template <typename ImType>
void LSHMatching<ImType>::insertClosestNeighbor(int k, double value, MyDMatch &m, map<int, double> &indexMinMap,
	map<int, MyDMatch> &nearestNeighborMap)
{
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
double LSHMatching<ImType>::getDist(KeyPoint &kp1, KeyPoint &kp2){
	double interior = pow(kp1.pt.x - kp2.pt.x, 2) + pow(kp1.pt.y - kp2.pt.y, 2);
	return sqrt(interior);
}

template <typename ImType>
int LSHMatching<ImType>::verify(ImgMatches &matches, ImageProvider<ImType> &_db, ImageContainer &_queryImage){

	if (matches.size() == 0) return -1;

	FundRes fundamentals = buildFundimentalMat(matches);

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
			showMatches(_db[i->first], _queryImage, tmp, i->second.first);
			inspectEpipole(_db[i->first], _queryImage, tmp, i->second.first);
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
	for (FundRes::iterator i = fundamentals.begin();
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
		showEpilines(_db[img],_queryImage,fundamentals[img].first);
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
			better_matches.insert(make_pair(i->first, _db[i->first]));
		}
	}

	//Rematch the images
	ImgMatches matches2 = doubleCheckMatches(better_matches, _queryImage);

	//Find fundamental matricies
	FundRes fundimentals = buildFundimentalMat(matches2);

	best_fit = 0;

#if INSPECT
	//Find the image with the best number of inliers
	for (FundRes::iterator i = fundimentals.begin();
		i != fundimentals.end(); ++i){
		vector<MyDMatch> tmp;
		int mean = sumInliers(matches2[i->first], i->second.second, tmp);

		if (best_fit < mean){
			showMatches(_db[i->first], _queryImage, tmp, i->second.first, false);
			best_fit = mean;
			img = i->first;
		}
	}
	if(img >= 0){
		showEpilines(_db[img],_queryImage,fundimentals[img].first);
	}
#else
	//Find the image with the best number of inliers
	for (FundRes::iterator i = fundimentals.begin();
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
FundRes LSHMatching<ImType>::buildFundimentalMat(ImgMatches matches){
	FundRes ret;

	//Goes through each db image and finds the fundamental matrix between each image and query image
	for (ImgMatches::iterator i = matches.begin();
		i != matches.end(); ++i){
		Fundimental p = findFund(buildObjSceneCorr(i->second));
		ret.insert(make_pair(i->first, p));
	}
	return ret;
}

template <typename ImType>
Fundimental LSHMatching<ImType>::findFund(ObjectScene train_scene){
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
ImgMatches LSHMatching<ImType>::doubleCheckMatches(map<int, ImType> &_db, ImageContainer &_queryImage, double min_dist){
	BFMatcher tempm(NORM_HAMMING2, mParams.compactResults);

	ImgMatches newMatches;

	if (!mParams.compactResults){
		map<int, vector<vector<DMatch>>> tmpDMatch;

		for (map<int, ImType>::iterator i = _db.begin(); i != _db.end(); ++i){
			vector<vector<DMatch>> received;
			tempm.knnMatch(_queryImage.getDescriptor(), i->second.getDescriptor(), received, mParams.k);

			tmpDMatch.insert(make_pair(i->first, received));
		}

		KNNRes tmpRes = convertDMatch(tmpDMatch, _db, _queryImage);

		FilteredRes tmpfRes = filterMatchingImages(tmpRes);
		newMatches = tmpfRes.second;

		return newMatches;
	}
	else {

		for (map<int, ImType>::iterator i = _db.begin(); i != _db.end(); ++i){
			vector<DMatch> received;
			tempm.match(_queryImage.getDescriptor(), i->second.getDescriptor(), received);

			newMatches.insert(make_pair(i->first, convertDMatch(received, i->second, _queryImage, i->first)));
		}

		return geometricFiltering(newMatches, 3);
	}
}

template <typename ImType>
int LSHMatching<ImType>::sumInliers(vector<unsigned int> &inliers){
	int b = 0, index = 0;
	for (vector<unsigned int>::iterator i = inliers.begin();
		i != inliers.end(); ++i){
		b += *i;
	}
	return b;
}

#if INSPECT
template <typename ImType>
int LSHMatching<ImType>::sumInliers(vector<MyDMatch> &matches, vector<unsigned int> &inliers, vector<MyDMatch> &fittingMatches){
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
void LSHMatching<ImType>::showMatches(ImType &_db, ImageContainer &_queryImage, vector<MyDMatch> &inliers, Mat F, bool step){
	ObjectScene objscene = buildObjSceneCorr(inliers);
	namedWindow("DBInliers",CV_WINDOW_KEEPRATIO);
	namedWindow("QueryInliers",CV_WINDOW_KEEPRATIO);
	if (!step){
		if (_db.loadImage()){
			Mat r;
			_db.getMat(r);
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
				float y2 = (-1 * ((*t)[0] / (*t)[1])*_db.imageSize().width) - ((*t)[2] / (*t)[1]);
				Point p1(0, y1), p2(_db.imageSize().width, y2);
				line(r, p1, p2, Scalar::all(-1));
			}
			
			imshow("DBInliers", r);
		}
		else {
			cout << "No DB image to show" << endl;
		}
		if (_queryImage.hasImage()){
			Mat r;
			_queryImage.getMat(r);
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
				float y2 = (-1 * ((*t)[0] / (*t)[1])*_queryImage.imageSize().width) - ((*t)[2] / (*t)[1]);
				Point p1(0, y1), p2(_queryImage.imageSize().width, y2);
				line(r, p1, p2, Scalar::all(-1));
			}
			imshow("QueryInliers", r);
		}
		else {
			cout << "No Query Image to show" << endl;
		}
	}
	else if (_queryImage.hasImage() && _db.loadImage()) {
		Mat q, d;

		vector<Vec3f> eplinesdb, eplinesq;
		computeCorrespondEpilines(objscene.first, 1, F, eplinesq);
		computeCorrespondEpilines(objscene.second, 2, F, eplinesdb);
		for (size_t t = 0; t < eplinesdb.size(); ++t){
			_db.getMat(d);
			_queryImage.getMat(q);

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
			float q_y2 = (-1 * (eplinesq[t][0] / eplinesq[t][1])*_queryImage.imageSize().width) - (eplinesq[t][2] / eplinesq[t][1]);
			Point q_p1(0, q_y1), q_p2(_queryImage.imageSize().width, q_y2);
			line(q, q_p1, q_p2, Scalar(0, 100, 255));

			float db_y1 = (-1 * (eplinesdb[t][2] / eplinesdb[t][1]));
			float db_y2 = (-1 * (eplinesdb[t][0] / eplinesdb[t][1])*_db.imageSize().width) - (eplinesdb[t][2] / eplinesdb[t][1]);
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
void LSHMatching<ImType>::inspectEpipole(ImType &_db, ImageContainer &_queryImage, vector<MyDMatch> &inliers, Mat F){
	ObjectScene objscene = buildObjSceneCorr(inliers);
	SVD svd(F, SVD::FULL_UV);
	namedWindow("DBInliers",CV_WINDOW_KEEPRATIO);
	namedWindow("QueryInliers",CV_WINDOW_KEEPRATIO);
	if (_db.loadImage()){
		Mat r;
		_db.getMat(r);
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
			float y2 = (-1 * ((*t)[0] / (*t)[1])*_db.imageSize().width) - ((*t)[2] / (*t)[1]);
			Point p1(0, y1), p2(_db.imageSize().width, y2);
			line(r, p1, p2, Scalar::all(-1));
		}
		imshow("DBInliers", r);
	}
	else {
		cout << "No DB image to show" << endl;
	}
	if (_queryImage.hasImage()){
		Mat r;
		_queryImage.getMat(r);
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
			float y2 = (-1 * ((*t)[0] / (*t)[1])*_queryImage.imageSize().width) - ((*t)[2] / (*t)[1]);
			Point p1(0, y1), p2(_queryImage.imageSize().width, y2);
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
void LSHMatching<ImType>::showEpilines(ImType &_db, ImageContainer &_queryImage, Mat F){
	if(_queryImage.hasImage() && _db.loadImage()){
		namedWindow("DBInliers",CV_WINDOW_KEEPRATIO);
		namedWindow("QueryInliers",CV_WINDOW_KEEPRATIO);
		Mat q, d,d2;
		_db.getMat(d2);
		vector<Point2f> q_points;
		for(size_t i = 0; i < _queryImage.getKeyPoints().size(); ++i){
			q_points.push_back(_queryImage.getKeyPoint(i).pt);
		}
		for(size_t i = 0; i < _db.getKeyPoints().size(); ++i){
			KeyPoint d_p = _db.getKeyPoint(i);
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
			_queryImage.getMat(q);

			KeyPoint q_p = _queryImage.getKeyPoint(t);
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