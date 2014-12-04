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
	LocalizationManager<DBImage,MatProvider<MyMat>,MyMat> manage("test");
	MatProvider<MyMat> t("test");
	manage.performTestingStats(t,"gest.csv");
#else

#endif
	return 0;
}

