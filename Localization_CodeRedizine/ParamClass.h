#pragma once
class ParamClass
{
public:
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
		int nfeatures,nlevels,edgeThresh,firstLevel,
		WTA_K,scoreType,patchSize;
		float scaleFactor;
	};

	struct ORBParams orbParam;

	/**
	 * Stores the parameters for the detecting line features
	 *
	 * cannyThresh1: the minium thresh hold
	 * cannyThresh2: the maximum thresh hold
	 * apperature: size of the sobel filter
	 * houghThresh: min number of matches in a bin for a line
	 * minLength: minimum lenght of a detected line
	 * rho: the resolution of the rho distance
	 * theta: the resolution of the the angle houghtransform
	 */
	struct LineParams{
		double cannyThresh1, cannyThresh2, apperature,
			houghThresh, minLength, maxGap, rho, theta;
	};
	struct LineParams lineParam;
};

