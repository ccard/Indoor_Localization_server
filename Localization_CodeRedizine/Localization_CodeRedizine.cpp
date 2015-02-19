// Localization_CodeRedizine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LocalizationManager.h"
#include "LocalizationManager.cpp"
#include "DBImage.h"
#include "MyMat.h"
#include "DBProvider.h"
#include "DBProvider.cpp"
#include "MatProvider.h"
#include "MatProvider.cpp"
#include "MapBuilder.h"
#include "MapBuilder.cpp"
#include "MapMatching.h"
#include "MapMatching.cpp"
#include "LSHMatching.h"
#include "LSHMatching.cpp"

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
	bool map_or_match = true;
	if(map_or_match){
		MapBuilder<MyMat,DBProvider<MyMat>,MapMatching<MyMat>> mb("Images\\db_images_new_3rd.txt");
		mb.buildMap("Test_map_3");
	} else {
		//Flags for what to run
		int option = EXAMIN;

		LocalizationManager<MyMat,DBProvider<MyMat>,LSHMatching<MyMat>> manage("Images\\local_map.txt");
		DBProvider<MyMat> db;
		switch(option){
		case VIDEO:
			manage.performVideoTesting("Images\\3rd_floor_brown_se_stair.mp4","Images\\VideoTestResults.csv",100,true,15,28000);
			break;
		case SUBSAMPLE:
			manage.performSubsample("full_db_fix_test_1.csv");
			break;
		case VIEWSTATS:
			break;
		case ROCTEST:
			if(db.open("Images\\local_map_test_set.txt")){
				manage.performTestingStats(db,"local_map_test_20.csv");
			}
			break;
		case EXAMIN:
			manage.evaluateMatches("Images\\local_map_test_set\\local_map_test_image_0001.JPG");
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

