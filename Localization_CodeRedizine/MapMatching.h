#pragma once
/**
* Author: Chris Card
* This class is the matching class that will be used to construct the map and has find method that returns the rotation
* and translation
*/

#include "Matcher.h"
#include "MyDMatch.h"
#include "LSHMatching.h"

#ifndef _MAPMATCHTYPES
#define _MAPMATCHTYPES
//Defines type defs and structures to return from the methods
typedef pair<Mat, vector<unsigned int>> Fund; //Defines pair from fundimentl matrix to the inliers array
typedef map<int, Fund> FundRess; //What is returned form finding fundimental matricies from multiple imagesv

typedef pair<Mat,Mat> Rat; //What is returned from find, firt is the image number second pair is R,t
#endif

template <typename ImType>
class MapMatching :
	public Matcher<ImType>
{
public:
	MapMatching(){ 
		lastIndex = 0;
		lshMatcher = FlannBasedMatcher(new flann::LshIndexParams(1,31,2));
		if(!mParams._init){
			mParams.k = 20;
			mParams.inlierThresh = 16;
			mParams.confidence = 0.99;
			mParams.pointdiff_maxDist = 0.8;
			mParams.fund_f_maxDist = 0;
			mParams.geo_k = 6;
			mParams.compactResults = false;
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

	int find(ImageContainer& query, ImageProvider<ImType> &db);

	int find(ImageContainer& query, ImageProvider<ImType> &db, Rat &r);

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
	bool knnMatch(ImageContainer &query, ImageProvider<ImType> &db,
		KNNRes &match, vector<Mat> &masks = vector<Mat>());


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
		ImageProvider<ImType> &db, ImageContainer &query);

	KNNRes convertDMatch(vector<vector<DMatch>> matches,
		map<int, ImType> &db, map<int, int> tmpdb_to_db, ImageContainer &query);

	KNNRes convertDMatch(map<int, vector<vector<DMatch>>> &matches,
		map<int, ImType> &db, ImageContainer &query);

	vector<MyDMatch> convertDMatch(vector<DMatch> matches,
		ImageContainer &train, ImageContainer &query, int train_idx);

	/**
	* This method builds the object and scene points and returns them so that
	* a homography can be estimated using the ransac method
	* 
	* @param: the vector of corresponding matching points from the training images
	*
	* @return: a pair of vectors first being the training keypoints and the second being
	* the query key points
	*/
	ObjectScene buildObjSceneCorr(vector<MyDMatch> matches);

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
	FilteredRes filterMatchingImages(KNNRes &matches);

	/**
	 * Goes through all the images and their feature matches and ensures that each match is 
	 * geometrically consistant.  
	 * See section 4.3 of the paper http://www.maninnet.com/cgi-bin/nph-surf.cgi/000/http/www.robots.ox.ac.uk/~vgg/publications/2009/Sivic09a/sivic09a.pdf
	 * for more detail on the process
	 *
	 * @param: the map of image indecies to the list of matches
	 * @param: the number of nearest neighbors to each key point
	 * 
	 * @return: the map database image indecies to the list of matches that satisfies the geometric constraints
	 */
	ImgMatches geometricFiltering(ImgMatches &im, int k);

	/**
	 * Finds the first index in the map whos value is bigger than the passed in value
	 *
	 * @param: map whos keys go from 0-(k-1) where the 0 index has the smallest value
	 * @param: the value to compare with
	 * @param: upper bound of the map keys (whos keys go from 0-(k-1))
	 *
	 * @return: the index in the map the first index in the map whos value is bigger than 
	 *          the passed in value, otherwise returns ERROR
	 */
	int findMinValueIndex(map<int, double> &indexMinMap, double value, int k);

	/**
	 * Inserts the value and MyDMatch objects into their respective arrays and slides the
	 * values to the next highest slot
	 *
	 * @param: the index in the maps to insert into
	 * @param: the number of keys in the maps (the maps keys go from 0-(k-1))
	 * @param: the value to insert into indexMinMap
	 * @param: the MyDMatch object to insert into nearestNeighborMap
	 * @param: map whos keys go from 0-(k-1) where the 0 index has the smallest value
	 * @param: map whos keys go from 0-(k-1) where the 0 index has the dmatch object 
	 *         corresponding to the smallest value in indexMinMap
	 */
	void insertClosestNeighbor(int k, double value, MyDMatch &m, map<int, double> &indexMinMap,
		map<int, MyDMatch> &nearestNeighborMap);

	/**
	 * Calculates the euclidean distance between two keypoints
	 *
	 * @param: the first keypoint
	 * @param: the second keypoint
	 *
	 * @return: the euclidean distance between the two keypoints
	 */
	double getDist(KeyPoint &kp1, KeyPoint &kp2);

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
	int verify(ImgMatches &matches, ImageProvider<ImType> &db, ImageContainer &query);

	int verify(ImgMatches &matches, ImageProvider<ImType> &db, ImageContainer &query, Rat &r);

	/**
	 * Find the (R)otation and (t)ranslation matricies from F
	 *
	 * @param: the fundamental matrix
	 *
	 * @return: the Rat object with the first being R and second being t
	 */
	Rat findRandT(Mat F, MyDMatch &dm, Size s);

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
	FundRess buildFundimentalMat(ImgMatches matches);

	/**
	* Finds the fundimental matrix and returns it along with the inliers
	*
	* @param: train and scene points, first train second scene
	*
	* @return: par from thr fund matrix to the inliers
	*/
	Fund findFund(ObjectScene train_scene);

	/**
	* This function rematches the points to ensure a better fit then what lsh might return
	*
	* @param: the list of training images to match against
	* @param: the query image
	* @param: the error with in the min distance to for a match to be kept
	* 
	* @return: vector of mydmathc objects
	*/
	ImgMatches doubleCheckMatches(map<int, ImType> &db, ImageContainer &query, double min_dist = 0.2);

	/**
	 * Sums the inliers 
	 */
	int sumInliers(vector<unsigned int> &inliers);

	int sumInliers(vector<MyDMatch> &matches, vector<unsigned int> &inliers, vector<MyDMatch> &fittingMatches);

#if INSPECT
	void showMatches(ImType &db, ImageContainer &query, vector<MyDMatch> &inliers, Mat F,bool step = false);
	void inspectEpipole(ImType &db, ImageContainer &query, vector<MyDMatch> &inliers, Mat F);
#endif
};

