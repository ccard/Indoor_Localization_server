#include "MapBuilder.h"

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void MapBuilder<ImgType,ImgProviderType,ImgMatcherType>::buildMap(){

}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
ImgMatch MapBuilder<ImgType, ImgProviderType, ImgMatcherType>::matchImage(ImgType q){
	Rat r;
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
	return make_pair(i, r);
}