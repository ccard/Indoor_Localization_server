// Localization_CodeRedizine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LocalizationManager.h"
#include "DBImage.h"
#include "MyMat.h"
#include "DBProvider.h"
#include "MatProvider.h"


int _tmain(int argc, _TCHAR* argv[])
{
#if DEBUG
	//Flags for what to run
	bool video = true;
	if(video){
		LocalizationManager<MyMat,MatProvider<MyMat>,MyMat> manage("Images\\db_images_new.txt");
		manage.performVideoTesting("Images\\3rd_floor_brown_se_stair.mp4","Images\\res.txt",true);
	}
#else

#endif
	return 0;
}

