#include "LocalizationManager.h"

#if DEBUG
template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
bool LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>::performTestingStats(ImgProviderType &pr, string file){

	map<string, string> results;
	cout << "Starting tests..." << endl;
	clock_t start, average = 0;
	for (int i = 0; i < pr.size(); ++i){
		start = clock();
		int img = match.find(pr[i], db);
		average = average + (clock()-start);

		if (ImgMatcherType::ERROR >= img){
			results.insert(make_pair(pr[i].getName(), "no match"));
		}
		else {
			results.insert(make_pair(pr[i].getName(), db[img].getName()));
		}
	}

	double avg = ((double)average)/pr.size();
	cout << "Average query time: " << avg/CLOCKS_PER_SEC << " seconds" << endl;

	cout << "Writing to file " << file << "..." << endl;
	ofstream out(file.c_str());

	for (map<string, string>::iterator i = results.begin(); i != results.end(); ++i){
		out << i->first << "," << i->second << endl;
	}

	out.close();
	cout << "Finished..." << endl;
	return true;
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>::performSubsample(string file){
	cout << "Performing subsample testing. The results will be outputed to '" << file << "' in csv format." << endl;
	cout << "The format will be 'group,query_file_name,guessed_file_name'." << endl;
	vector<set<int>> testSets = generateTestingSet(30, 5);
	vector<pair<int, pair<string, string>>> results, privres;

#ifdef _OPENMP
	cout << "Starting tests in parallel...." << endl;
#pragma omp parallel private(privres) shared(results) num_threads(NUM_THREADS)
	{
#pragma omp for
		for (int i = 0; i < testSets.size(); ++i){
			cout << "Running group: " << i << endl;
			ImgProviderType ndb;
			vector<ImgType> testSet;
#pragma omp critical
			{
				createTestingSet(testSets[i], testSet, ndb);
			}
			ImgMatcherType nmatch;
			nmatch << ndb;
			nmatch.train();

			for (vector<ImgType>::iterator j = testSet.begin(); j != testSet.end(); ++j){
				int img = nmatch.find(*j, ndb);
				if (img <= ImgMatcherType::ERROR){
					privres.push_back(make_pair(i, make_pair(j->getName(), "no match")));
				}
				else {
					privres.push_back(make_pair(i, make_pair(j->getName(), ndb[img].getName())));
				}
			}

			testSet.clear();
		}
#pragma omp critical
			{
				results.insert(results.end(), privres.begin(), privres.end());
			}
	}
#else
	clock_t start, average = 0;
	cout << "Starting sequential tests...." << endl;
	for (size_t i = 0; i < testSets.size(); ++i){
		cout << "Running group: " << i << endl;
		ImgProviderType ndb;
		vector<ImgType> testSet;
		createTestingSet(testSets[i], testSet, ndb);
		ImgMatcherType nmatch;
		nmatch << ndb;
		nmatch.train();

		for (vector<ImgType>::iterator j = testSet.begin(); j != testSet.end(); ++j){
			start = clock();
			int img = nmatch.find(*j, ndb);
			if (img <= ImgMatcherType::ERROR){
				results.push_back(make_pair(i, make_pair(j->getName(), "no match")));
			}
			else {
				results.push_back(make_pair(i, make_pair(j->getName(), ndb[img].getName())));
			}
			average = average + (clock() - start);
		}

		testSet.clear();
	}
	double avgtime = ((double)average) / 150;
	cout << "Average time: " << avgtime / CLOCKS_PER_SEC << endl;
#endif
	cout << "Writing results to the file ..... " << endl;
	ofstream out(file);
	for (size_t i = 0; i < results.size(); ++i){
		char buff[10];
		itoa(results[i].first, buff, 10);
		string tmps = string(buff) + "," + results[i].second.first + "," + results[i].second.second;
		out << tmps << endl;
	}
	out.close();
	cout << "Finished!" << endl;
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>::performVideoTesting(string vFile, string outFile, int minNumORB, bool manualComparison, int sampleFrequency, double seek){
	VideoCapture vid(vFile.c_str());
	if (vid.isOpened()){
		if (manualComparison){
			double correct = 0, falsePositives = 0;
			size_t num_sampled = 0;

			namedWindow("Frame");
			namedWindow("Match");
			Mat frame, empty(20, 20, CV_16S);
#if INSPECT
			vid.set(CV_CAP_PROP_POS_MSEC, seek);
#endif
			do{
				vid >> frame;
				MyMat img;
				frame.copyTo(img);
				//resize(img,img,Size(816,612));
				img.initDescriptor();
				img.makeMask();
				img.calcDescriptor();

				if (img.getKeyPoints().size() < minNumORB) continue;

				++num_sampled;
				int img_num = match.find(img, db);

				imshow("Frame", drawKeyPoints(img));

				if (img_num <= ImgMatcherType::ERROR){
					imshow("Match", empty);
					cout << "no match: " << img_num << endl;
				}
				else {
					ImgType tmpimg = db[img_num];

					if (tmpimg.loadImage()){
						imshow("Match", drawKeyPoints(tmpimg));
					}
					else {
						cout << "failed to load image" << endl;
					}
				}

				cout << "Is this correct [y]es or [n]o?";
				char yn = waitKey();
				cout << yn << endl;
				if (yn == 'y'){
					correct++;
				}
				else {
					falsePositives++;
				}

				cout << "Running accuracy: " << ((correct / num_sampled) * 100) << "%" << endl;

				for (int i = 1; i < sampleFrequency - 1; ++i) {
					vid.grab();
				}
				vid >> frame;
			} while (!frame.empty());

			ofstream out(outFile);
			out << "Correct,False Positives,Number of Samples" << endl;
			out << correct << "," << falsePositives << "," << num_sampled << endl;
			out.close();

			cout << "Num correct: " << correct << endl;
			cout << "False positives: " << falsePositives << endl;
			cout << "Accuracy: " << ((correct / num_sampled) * 100) << "%" << endl;

		}
		else {
			cout << "The results will be written to '" << outFile << "' in csv fromat." << endl;
			cout << "The format of the file will be 'timestamp,fram number,matched database image'" << endl;
			map<double, pair<int, string>> results;
			double num_sampled = 0;
			time_t running = 0;

			while (vid.grab()) {
				time_t start = time(NULL);
				++num_sampled;
				MyMat m;
				vid.retrieve(m);
				//resize(m,m,Size(612,816));
				m.initDescriptor();
				m.makeMask();
				m.calcDescriptor();
				double timestamp = vid.get(CV_CAP_PROP_POS_MSEC);
				int frame = vid.get(CV_CAP_PROP_POS_FRAMES);

				int img = match.find(m, db);
				pair<int, string> tmp(frame, (img <= ImgMatcherType::ERROR ? "No Match" : db.getImage(img).getName()));
				results.insert(make_pair(timestamp, tmp));
				running += (time(NULL) - start);
				for (int i = 1; i < sampleFrequency - 1; ++i) {
					vid.grab();
				}
			}

			ofstream out(outFile);
			for (map<double, pair<int, string>>::iterator i = results.begin(); i != results.end(); ++i){
				out << i->first << "," << i->second.first << "," << i->second.second << endl;
			}
			out.close();
			cout << "Finished .... " << endl;
			cout << "Average time per frame: " << (running / num_sampled) << endl;
		}
	}
	else {
		ASSERT(false, "Failed to open video file " << vFile);
	}
	vid.release();
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>::evaluateMatches(string imageFile){
	MyMat q;
	Mat r = imread(imageFile);
	r.copyTo(q);
	resize(q, q, Size(612, 816));
	q.makeMask();
	q.initDescriptor();
	clock_t orb_start = clock();
	q.calcDescriptor();
	cout << " Orb time: " << ((float)(clock()-orb_start))/CLOCKS_PER_SEC << " sec" << endl;
	clock_t start = clock();
	int image = match.find(q, db);
	cout << "time: " << ((float)(clock() - start)) / CLOCKS_PER_SEC << endl;
	namedWindow("Query", CV_WINDOW_KEEPRATIO);
	namedWindow("Retrieved", CV_WINDOW_KEEPRATIO);
	if (image > ImgMatcherType::ERROR){
		imshow("Query", drawKeyPoints(q));
		if (db[image].loadImage()){
			Mat rt;
			db[image].getMat(rt);
			imshow("Retrieved", rt);
		}
		waitKey();
	}
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>::evaluateMatches(ImgProviderType &images){

}
#endif


#if DEBUG
template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
void LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>::createTestingSet(set<int> indicies, vector<ImgType> &testSet, ImgProviderType &newdb){
	for (size_t i = 0; i < db.size(); ++i){
		if (indicies.find(i) != indicies.end()){
			testSet.push_back(db[i]);
		}
		else {
			newdb.addImage(db[i]);
		}
	}
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
vector<set<int>> LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>::generateTestingSet(size_t groupSize, int num_tests){
	vector<set<int>> ret;

	for (int i = 0; i < num_tests; ++i){
		set<int> group;
		srand(time(NULL)*rand());
		while (group.size() < groupSize){
			int index = rand() % db.size();
			if (group.find(index) == group.end()) group.insert(index);
		}
		ret.push_back(group);
	}
	return ret;
}

template<typename ImgType, typename ImgProviderType, typename ImgMatcherType>
Mat LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>::drawKeyPoints(MyMat &im, bool draw_arrows, Scalar color, bool draw_circle){
	KeyPoint p;
	Mat r;
	im.copyTo(r);
	for (size_t i = 0; i < im.getKeyPoints().size(); ++i) {
		p = im.getKeyPoint(i);

		if (draw_circle) circle(r, p.pt, p.size, color, 1);

		if (draw_arrows){
			double x = p.pt.x + p.size*cos(p.angle);
			double y = p.pt.y + p.size*sin(p.angle);

			int mag = 5;
			double pi = 3.141592653;
			double x1 = x - mag*cos(p.angle + pi / 4);
			double y1 = y - mag*sin(p.angle + pi / 4);

			double x2 = x - mag*cos(p.angle - pi / 4);
			double y2 = y - mag*sin(p.angle - pi / 4);

			line(r, p.pt, Point(x, y), color);
			line(r, Point(x, y), Point(x1, y1), color);
			line(r, Point(x, y), Point(x2, y2), color);
		}
		circle(r, p.pt, 2, Scalar(0, 0, 255), -1);
		char buff[33];
		itoa(i, buff, 10);
		putText(r, buff, Point(p.pt.x + 4, p.pt.y + 4), FONT_HERSHEY_COMPLEX, .5,
			Scalar(255, 13, 255));
	}
	return r;
}
#endif