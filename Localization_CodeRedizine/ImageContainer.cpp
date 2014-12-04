#include "ImageContainer.h"

const string ImageContainer::DESCRIPTOR = "descriptor_mat";
const string ImageContainer::KEYPOINTS = "keypoint_list";
const string ImageContainer::CROSSRATIOS = "cross_ratios_list";


void ImageContainer::setKeyPoints(vector<KeyPoint> kps){
	keyPoints.clear();
	keyPoints.insert(keyPoints.begin(), kps.begin(), kps.end());
}

KeyPoint ImageContainer::getKeyPoint(size_t index){
	KeyPoint kp;

	if(index < 0 || index >= keyPoints.size()) return kp;

	kp = keyPoints[index];
	return kp;
}

const vector<KeyPoint> ImageContainer::getKeyPoints(){
	return keyPoints;
}

const Mat ImageContainer::getDescriptor(){
	return descriptor;
}

void ImageContainer::setIndex(size_t index){
	img_index = index;
}

map<string,string> ImageContainer::save(){
	map<string,string> serealization;

	assert(!descriptor.empty());
	assert(!keyPoints.empty());

	//wrting descriptor
	FileStorage imDescript("mydata.xml",FileStorage::MEMORY|FileStorage::WRITE);
	imDescript << DESCRIPTOR << descriptor;
	serealization[DESCRIPTOR] = imDescript.releaseAndGetString();

	//writing key points
	FileStorage im_key("mydata.xml",FileStorage::MEMORY|FileStorage::WRITE);
	im_key << KEYPOINTS << "[";
	for(vector<KeyPoint>::iterator i = keyPoints.begin();
		i != keyPoints.end(); ++i){
			KeyPoint p = *i;
			im_key << "{";
			im_key << "angle" << p.angle;
			im_key << "class_id" << p.class_id;
			im_key << "octave" << p.octave;
			im_key << "point" << "{";
			im_key << "x" << p.pt.x;
			im_key << "y" << p.pt.y;
			im_key << "}";
			im_key << "response" << p.response;
			im_key << "size" << p.size;
			im_key << "}";
	}
	im_key << "]";

	serealization[KEYPOINTS] = im_key.releaseAndGetString();

	return serealization;
}