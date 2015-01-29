// Localization_CodeRedizine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LocalizationManager.h"
#include "DBImage.h"
#include "MyMat.h"
#include "DBProvider.h"
#include "MatProvider.h"
#include "MapBuilder.h"
#include "MapMatching.h"
#include "LSHMatching.h"

#if DEBUG
const static int VIDEO = 0;
const static int SUBSAMPLE = 1;
const static int VIEWSTATS = 2;
const static int ROCTEST = 3;
const static int EXAMIN = 4;
#endif

int _tmain(int argc, _TCHAR* argv[])
{
#if DEBUG
	bool map_or_match = false;
	if(map_or_match){
		Mat tmp = imread("Images\\new_2nd_floor_brown\\2nd_floor_brown_new_7265.JPG");
		MyMat q;
		tmp.copyTo(q);
		resize(q,q,Size(612,816));
		q.makeMask();
		q.initDescriptor();
		q.calcDescriptor();
		MapBuilder<MyMat,DBProvider<MyMat>,MapMatching<MyMat>> mb("Images\\db_images_new.txt");
		mb.matchImage(q);
	} else {
		//Flags for what to run
		int option = SUBSAMPLE;

		LocalizationManager<MyMat,DBProvider<MyMat>,LSHMatching<MyMat>> manage("Images\\db_images_new.txt");
		DBProvider<MyMat> db;
		switch(option){
		case VIDEO:
			manage.performVideoTesting("Images\\3rd_floor_brown_se_stair.mp4","Images\\VideoTestResults.csv",100,true,15,28000);
			break;
		case SUBSAMPLE:
			manage.performSubsample("SubSampleResults_fix.csv");
			break;
		case VIEWSTATS:
			break;
		case ROCTEST:
			if(db.open("Images\\db_images_roc_test_set.txt")){
				manage.performTestingStats(db,"ROC_RES_in_22.csv");
			}
			break;
		case EXAMIN:
			manage.evaluateMatches("Images\\new_2nd_floor_brown\\2nd_floor_brown_old_6689.JPG");
			break;
		default:
			cerr << "Incorrect options" << endl;
		}
	}
#else

#endif
	system("PAUSE");
	return 0;
}

