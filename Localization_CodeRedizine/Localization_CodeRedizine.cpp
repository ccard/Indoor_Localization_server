// Localization_CodeRedizine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LocalizationManager.h"
#include "DBImage.h"
#include "MyMat.h"
#include "DBProvider.h"
#include "MatProvider.h"
#include "LSHMatching.h"

#if DEBUG
const static int VIDEO = 0;
const static int SUBSAMPLE = 1;
const static int VIEWSTATS = 2;
#endif

int _tmain(int argc, _TCHAR* argv[])
{
#if DEBUG
	//Flags for what to run
	int option = SUBSAMPLE;

	LocalizationManager<MyMat,DBProvider<MyMat>,LSHMatching<MyMat>> manage("Images\\db_images_new.txt");
	switch(option){
	case VIDEO:
		manage.performVideoTesting("Images\\3rd_floor_brown_se_stair.mp4","Images\\VideoTestResults.csv",100,true,15,28000);
		break;
	case SUBSAMPLE:
		manage.performSubsample("SubSampleResults.csv");
		break;
	case VIEWSTATS:
		break;
	default:
		cerr << "Incorrect options" << endl;
	}
#else

#endif
	system("PAUSE");
	return 0;
}

