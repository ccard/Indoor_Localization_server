#pragma once
/**
 * Author: Chris Card
 *
 * This interface encapsulates images used for localization and displaying those images 
 * (not required but nice)
 *
 * This is the core of the program all other classes use the Image container to encapsulate image
 * information
 *
 * Usage:
 *  At the very least other classes must be able to retrieve the descriptor
 *  and the keypoints from any class implementing this interface
 */

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\nonfree\features2d.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include "ParamClass.h"

//Defines orbparams object
#ifndef _ORBPARAMS
	#define _ORBPARAMS
	/**
	*  This stores the paramerters for orb discriptors
	*
	*  nfeatures: the max # of features to return default 500
	*  scalefactor: the scale factor for each level of the scale pyramid
	*  default 1.2
	*  nlevels: the number of levels in the scale pyramid default 8
	*  edgeThresh: the edge threshold score defualt 31
	*  firstlevel: the level that will be the first level of the pyramid default 0
	*  WTA_K: The number of points to use to calculate each element of the BREIF
	*  descriptor default 2
	*  scoretype: the way to rank features default HARRIS score
	*  patchsize: the size of the patch used by the brief descriptors
	*/
  	struct ORBParams{
		ORBParams() : nfeatures(1500), scaleFactor(1.2f),nlevels(8), edgeThresh(31),firstLevel(3),WTA_K(3),scoreType(cv::ORB::HARRIS_SCORE),patchSize(31),_init(true) {}
		int nfeatures,nlevels,edgeThresh,firstLevel,
		WTA_K,scoreType,patchSize;
		float scaleFactor;
		bool _init;//Ensures that this is only initialized once
	};
	static struct ORBParams orbParams;
#endif

	//Defines the debug flag to determine if other classes should compile in debug mode
#ifndef DEBUG
	//1 for true 0 for false
#define DEBUG 1
#endif

		//Defines the debug flag to determine if other classes should compile in debug mode
#ifndef INSPECT
	//1 for true 0 for false
#define INSPECT 1
#endif

	//Defines custom assert object for asserts at run time this code was taken from 
	// http://stackoverflow.com/questions/3767869/adding-message-to-assert
#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
			std::system("PAUSE"); \
            std::exit(EXIT_FAILURE); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

using namespace cv;
using namespace std;

class ImageContainer {
public:
	const static int CIRCLE = 0;
	const static int SQUARE = 1;

	//Serialize keys
	const static string DESCRIPTOR;
	const static string KEYPOINTS;
	const static string CROSSRATIOS;

	ImageContainer(string name = "none"){
		init();
	}

	ImageContainer(string img_file,Mat descriptor, vector<KeyPoint> kps){
		init();
		this->name = img_file;
		descriptor.copyTo(this->descriptor);
		keyPoints = kps;
	}

	~ImageContainer(){
		descriptor.release();
		keyPoints.clear();
	}

	/**
	*  This method constructs the mask of a specified shape and size
	*
	*  @param: the mask type defaults to circle
	*  @param: the size of the mask default nine
	*/
	virtual void makeMask(int maskType = ImageContainer::CIRCLE, int side_radius = 9, int side2 = 16) = 0;

	/**
	 * Intializes the orb object based on the parameters
	 *
	 * @param: the parm object containing the orb pramaters
	 */
	virtual void initDescriptor() = 0;

	/**
	 * Calculates the orb descriptor of the image
	 */
	virtual bool calcDescriptor() = 0;
	
	/**
	 * returns the size of the drawable image
	 *
	 * @return: the size of the displayable image
	 */
	virtual Size imageSize() = 0;

	/**
	 * Determines if the ImageContainer has an image to display
	 */
	virtual bool hasImage() = 0;

	virtual bool loadImage() = 0;

	virtual void getMat(Mat &mat) = 0;

	void setIndex(size_t index);

	size_t getIndex(){ return img_index; }

	void setKeyPoints(vector<KeyPoint> kps);

	KeyPoint getKeyPoint(size_t index);

	const vector<KeyPoint> getKeyPoints();

	const Mat getDescriptor();

	bool operator <(const ImageContainer &rhs){
		return img_index < rhs.img_index;
	}

	bool operator== (const ImageContainer &rhs){
		return img_index == rhs.img_index;
	}

	bool operator== (const int rhs){
		return img_index == rhs;
	}

	map<string,string> save();

	void setName(string name){ this->name = name; }

	string getName(){ return name; }

protected:
	Mat mask,descriptor,gray;
	vector<KeyPoint> keyPoints;
	ORB orbDescriptor;
	bool initOrb,initLines;
	size_t img_index;
	string name;

	void init(){
		if(!orbParams._init){
			orbParams.nfeatures = 1500;//Number of key points to find
			orbParams.scaleFactor = 1.2f;//Scaling factor
			orbParams.nlevels = 8; //number of levels
			orbParams.edgeThresh = 31;//edge threshold
			orbParams.firstLevel = 3; //first level
			orbParams.WTA_K = 3; //number of features used to calculate a feature point
			orbParams.scoreType = ORB::HARRIS_SCORE; //Ranking features
			orbParams.patchSize = 31; //size of patch used by brief
			orbParams._init = true; //Ensures that this is only set once
		}
	}
};

