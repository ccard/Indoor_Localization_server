#include "MapBuilder.h"

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void MapBuilder<ImgType,ImgProviderType,ImgMatcherType>::buildMap(){
	//TODO: put rebuild code here
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
NearImages MapBuilder<ImgType, ImgProviderType, ImgMatcherType>::matchImage(ImgType q){
	NearImages r;
	int i = match.find(q, db, r);
#if INSPECT
	cout << "R: " << r.first << endl << "t: " << r.second << endl;
	namedWindow("QueryImg", CV_WINDOW_KEEPRATIO);
	namedWindow("DBImg", CV_WINDOW_KEEPRATIO);
	imshow("QueryImg", q);
	if (db[i].loadImage()){
		imshow("DBImg", db[i]);
	}
#endif
	return r;
}