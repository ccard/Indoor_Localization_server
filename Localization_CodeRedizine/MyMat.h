#pragma once

/**
* Author: Chris Card
* This class contains a drawable mat used to calculate the images
*/
#include "ImageContainer.h"
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\nonfree\features2d.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\opencv.hpp"
#include "MyLine.h"

using namespace cv;
using namespace std;

class MyMat :
	public ImageContainer, public Mat
{
public:
	MyMat(): ImageContainer(){ init(); }

	MyMat(string img_file,Mat descriptor, vector<KeyPoint> kps): ImageContainer(img_file,descriptor, kps){}

	void initDescriptor();

	void makeMask(int maskType = ImageContainer::CIRCLE, int side_radius = 9, int side2 = 16);

	bool calcDescriptor();

	Size imageSize();

	bool hasImage();

	bool loadImage();

private:
};

