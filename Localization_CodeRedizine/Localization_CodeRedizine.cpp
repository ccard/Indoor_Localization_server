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
#else
const static int CREATEDB = 0;
const static int CREATEMAP = 1;
const static int MATCH = 2;
#endif

int _tmain(int argc, _TCHAR* argv[])
{
#if DEBUG
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  !!!    This section was only for development please see the !!!
	  !!!   the precompiler block below for the actual            !!!
	  !!!   example code                                          !!!
	  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	*/
	bool map_or_match = false;
	if(map_or_match){
		MapBuilder<MyMat,DBProvider<MyMat>,MapMatching<MyMat>> mb("Images\\db_images_new_3rd.txt");
		mb.buildMap("Test_map_fix");
	} else {
		//Flags for what to run
		int option = EXAMIN;

		LocalizationManager<MyMat,DBProvider<MyMat>,LSHMatching<MyMat>> manage("Images\\db_images_new_2.txt");
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
			if(db.open("Images\\roc_testset_TP_and_FN.txt")){
				manage.performTestingStats(db,"roc_TP_and_FP_35.csv");
			}
			break;
		case EXAMIN:
			manage.evaluateMatches("Images\\new_3rd_floor_brown\\3rd_flloor_brown_new_6961.JPG");
			break;
		default:
			cerr << "Incorrect options" << endl;
		}
	}
#else
	int OPTION = CREATEDB;

	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  !!!      Please refer to the "How to Run" section in the  !!!
	  !!!      README.md for more details                       !!!
	  !!!                                                       !!!
	  !!!  The comments below and in the README.md are only     !!!
	  !!!  applicable to the provided code and expected file    !!!
	  !!!  formats (see README.md).  If you wish to use your    !!!
	  !!!  own classes that extend the interfaces the comments  !!!
	  !!!  can be ignored. The acctual lines of code below will !!!
	  !!!  not need to be changed derastically.                 !!!                
	  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	//(recomend) Use this option to precompute image descriptors. This makes loading the
	//database orders of magnitude faster
	if (CREATEDB == OPTION){
		MatProvider<MyMat> db;
		//Replace "<file name>" with your file and must
		//be in the format descirbed in the How to Run section in README.md
		if (db.open("<file name>")){
			//Replace "<list output file>" with the file name that will store the list
			//of database xml files
			//Replace "<directory to save the db>" with the directory that the xml files
			//will be written to
			if (!db.saveImages("<list output file>", "<directory to save the db>")){
				cerr << "Failed to save the database as specified please check that:" << endl;
				cerr << "  1) The program can create <list output file>." << endl;
				cerr << "  2) The <directory to save the db> exists and can be written to." << endl;
			}
		}
	}
	//Creates an graph of how the images are related
	else if (CREATEMAP == OPTION){
		/*
			Option 1 (if descriptors where precomputed using CREATEDB option):
			   - MyMat can be replaced with any class that extends ImageContainer
			   - Replace "<file name>" where "<file name>" is the 
			     "<list output file>" created in the CREATEDB section
			Option 2 (if descriptors where not precomputed using CREATEDB option):
			   - MyMat must be used
			   - DBProvider<MyMat> must be replaced with MatProvider<MyMat>
			   - Replace "<file name>" with your file and must
				 be in the format descirbed in the How to Run section in README.md
		 */
		MapBuilder<MyMat, DBProvider<MyMat>, MapMatching<MyMat>> mb("<relative path> + <file name>");
		
		// Replace "<file prefix" with prefix that you want (without the extention e.g. .txt)
		// that you want to use the code will create two files using this prefix
		mb.buildMap("<file prefix");
	}
	// Match an image to the database
	else if (MATCH == OPTION){
		//Replace "<image file>" with the query image file name (i.e. '*.jpg')
		Mat input_image = imread("<image file>");
		MyMat query_image;
		input_image.copyTo(query_image);
		resize(query_image, query_image, Size(612, 816));
		query_image.makeMask();
		query_image.initDescriptor();
		query_image.calcDescriptor();

		/*
			Option 1 (if descriptors where precomputed using CREATEDB option):
			   - MyMat can be replaced with any class that extends ImageContainer
			   - Replace "<file name>" where "<file name>" is the
				 "<list output file>" created in the CREATEDB section
			Option 2 (if descriptors where not precomputed using CREATEDB option):
				- MyMat must be used
				- DBProvider<MyMat> must be replaced with MatProvider<MyMat>
				- Replace "<file name>" with your file and must
				  be in the format descirbed in the How to Run section in README.md
		*/
		LocalizationManager<MyMat, DBProvider<MyMat>, LSHMatching<MyMat>> lm("<relative path> + <file name>");

		int image_match_index = lm == query_image;
		if (image_match_index >= 0){
			namedWindow("Query Image", CV_WINDOW_KEEPRATIO);
			namedWindow("Matched Image", CV_WINDOW_KEEPRATIO);

			imshow("Query Image", query_image);

			// Replace MyMat with the same objects used for
			// the Localization manager
			MyMat db_image(lm.getDBImage(image_match_index));

			if (db_image.loadImage()){
				Mat tmp_db;
				db_image.getMat(tmp_db);
				imshow("Matched Image", tmp_db);
			}
			else {
				cerr << "Unable to load the database image, either because the file can not be opened or because it does not have image" << endl;
			}
		}
	}
	else {
		cerr << "Imvalid option!" << endl;
	}
#endif
	system("PAUSE");
	return 0;
}

