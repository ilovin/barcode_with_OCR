#include "stdafx.h"
#include "movefile.h"
#include <experimental\filesystem>
#include <filesystem>
//#include <Windows.h>

namespace fs = std::experimental::filesystem;
int cpFiles(string &inF, string &imgName, string &outF, string &serial,string &approach, string lastSerial) {
	if (serial.empty()) 
	{
		if (lastSerial.empty())
			serial = "!no_serial";
		else serial = lastSerial;
	}
	if (approach.empty()) approach = "Appendix";
	string outFolder = outF + "/"+serial;
	string countFile = outFolder + "/" + "countFile.txt";
	string suf = imgName.substr(imgName.find_last_of("."));

	//string inFileName = inF + "/" + imgName;
	string inFileName = imgName;
	if (!fs::exists(outFolder))
	{
		fs::create_directory(outFolder);
		//create countfile.txt
		std::ofstream ofs(countFile, ios::trunc | ios::out);

		string outFileName = outFolder + "/" + "1." + approach + suf;
		CopyFile(inFileName.c_str(), outFileName.c_str(),1);
		DWORD Error = GetLastError();
		if (unsigned int(Error)) cout <<"Error copy file"<< Error << endl;
		ofs << "1";
		ofs.close();

	}
	else
	{
		std::ifstream ifs(countFile);
		string cnt_str;
		int cnt;
		ifs >> cnt;
		ifs.close();
		std::ofstream ofs(countFile, ios::trunc | ios::out);
		stringstream ss;
		ss << ++cnt;
		string outFileName = outFolder + "/" + ss.str()+"." + approach + suf;
		CopyFile(inFileName.c_str(), outFileName.c_str(),0);
		ofs << ss.str();
		ofs.close();
	}

}

string cpFiles(string &inF, string &imgName, string &outF, 
	std::set<string>&serial ,string &lastName) {
	string name;
	if (!serial.size()) 
	{
		if (lastName.empty())
			name = "!no_serial";
		else name = lastName;
	}
	else
	{
		for (auto &s : serial) {
			cout << "serial detected: " << s << endl;
			name = name + "_"+s;
		}
		name.erase(0,1);
	}
	string outFolder = outF + "/"+name;
	string countFile = outFolder + "/" + "countFile.txt";
	string suf = imgName.substr(imgName.find_last_of("."));

	//string inFileName = inF + "/" + imgName;
	string inFileName = imgName;
	if (!fs::exists(outFolder))
	{
		fs::create_directory(outFolder);
		//create countfile.txt
		std::ofstream ofs(countFile, ios::trunc | ios::out);

		string outFileName = outFolder + "/" + "1." + suf;
		CopyFile(inFileName.c_str(), outFileName.c_str(),1);
		DWORD Error = GetLastError();
		if (unsigned int(Error)) cout <<"Error copy file"<< Error << endl;
		ofs << "1";
		ofs.close();

	}
	else
	{
		std::ifstream ifs(countFile);
		string cnt_str;
		int cnt;
		ifs >> cnt;
		ifs.close();
		std::ofstream ofs(countFile, ios::trunc | ios::out);
		stringstream ss;
		ss << ++cnt;
		string outFileName = outFolder + "/" + ss.str()+"." + suf;
		CopyFile(inFileName.c_str(), outFileName.c_str(),0);
		ofs << ss.str();
		ofs.close();
	}
	return name;
}
int moveFiles(string &inF, string &outF, int fileType, bool IsRecursive) {
	if (!fs::exists(outF)) {
		try
		{
			fs::create_directory(outF);
		}
		catch (const std::exception&e)
		{
			cerr << e.what()<< endl;
			return -1;
		}
	}
	std::vector<std::string> imgList;
	switch (fileType)
	{
	case WORKSHEET:
		//tessrInit();
		if (!getImgList(inF, imgList, IsRecursive)) {
			cout << "total " << imgList.size() << " file" << endl;
			int key = 0;
			string lastSerial;
			for (auto &img:imgList)
			{
				//string fileName = "img/ws/ws5_03.jpg";
				cout << img << endl;
				Mat src_color = imread(img, 1);
				if (src_color.empty())
				{
					cerr << "cannot open the IMG: " << img << endl;
				}
				WorkSheet ws(src_color);
				ws.process();
				cpFiles(inF,img, outF, ws.getSerial(), ws.getApproach(), lastSerial);
				lastSerial = ws.getSerial();

				//ws.roiOcr();

				namedWindow("color_res", cv::WINDOW_AUTOSIZE);
				imshow("color_res", ws.getProcessedImg());
				key = waitKey(0);
				if (key == 27) break;
			}
		}
		break;
	case FORM:
		if (!getImgList(inF, imgList, IsRecursive)) {
			cout << "total " << imgList.size() << " file" << endl;
			int key = 0;
			for (auto &img:imgList)
			{
				//string fileName = "img/ws/ws5_03.jpg";
				cout << img << endl;
				Mat src_color = imread(img, 1);
				if (src_color.empty())
				{
					cerr << "cannot open the IMG: " << img << endl;
				}
				Form fm(src_color);
				fm.process();
				string lastSerial;
				lastSerial = cpFiles(inF,img, outF, fm.getSerials(), lastSerial);

				//ws.roiOcr();
				//namedWindow("res", cv::WINDOW_NORMAL);
				//imshow("res", fm.getProcessedImg());
				//key = waitKey(0);
				//if (key == 27) break;
			}
		}

		break;
	case REPORT:
		break;
	default:
		break;
	}
	return 0;
}
