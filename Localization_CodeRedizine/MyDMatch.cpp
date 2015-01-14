#include "MyDMatch.h"

MyDMatch::MyDMatch(DMatch m)
{
	imgIdx = m.imgIdx;
	queryIdx = m.queryIdx;
	trainIdx = m.trainIdx;
	distance = m.distance;
}

MyDMatch::MyDMatch(void){
	imgIdx = -1;
	queryIdx = -1;
	trainIdx = -1;
	distance = -1;
}

bool MyDMatch::operator<(const MyDMatch m)const{
	return queryIdx < m.queryIdx;
}

bool MyDMatch::operator()(const MyDMatch &lhs, const MyDMatch &rhs){
	return lhs.queryIdx < rhs.queryIdx;
}

bool MyDMatch::operator==(const MyDMatch &m) const{
	bool match = imgIdx == m.imgIdx;
	match &= (queryIdx == m.queryIdx);
	match &= (trainIdx == m.trainIdx);
	match &= (distance == m.distance);
	return match;
}
