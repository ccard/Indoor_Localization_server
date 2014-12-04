#include "MyMat.h"
#include "MyLine.h"

void MyMat::initORB(){
	initOrb = true;
	if(gray.empty()) cvtColor(*this,gray,CV_BGR2GRAY);
	orbDescriptor = ORB(orbParams.nfeatures,orbParams.scaleFactor,orbParams.nlevels,
		orbParams.edgeThresh,orbParams.firstLevel,orbParams.WTA_K,orbParams.scoreType,orbParams.patchSize);
}

void MyMat::makeMask(int maskType, int side_radius, int side2){
	switch(maskType){
	case ImageContainer::CIRCLE:
		mask = Mat::zeros(side_radius+side_radius,
			side_radius+side_radius,CV_8U);
		circle(mask,Point(side_radius,side_radius),side_radius,
			Scalar(1,1,1),-1);
		break;
	case ImageContainer::SQUARE:
		mask = Mat::zeros(side_radius,side2,CV_8U);
		mask(Rect(0,0,side2,side_radius)) = 1;
		break;
	}
}

bool MyMat::calcORB(){
	if(!initOrb) return false;
	if(mask.empty()) return false;

	orbDescriptor(gray,mask,keyPoints,descriptor);
	return true;
}

Size MyMat::imageSize(){
	if(this->empty()) return Size(0,0);
	return Size(this->cols,this->rows);
}

bool MyMat::hasImage(){
	return !this->empty();
}

bool MyMat::loadImage(){
	if(!hasImage()) return true;
	if(name.size() == 0) return false;
	Mat tmp = imread(name);
	cv::resize(tmp,tmp,Size(612,816));
	tmp.copyTo(*this);
	tmp.release();
	return (!hasImage());
}