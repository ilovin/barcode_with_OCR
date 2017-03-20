#include "stdafx.h"
#include <iostream>
#include "movefile.h"
#include <experimental\filesystem>
#include "page.h"
#include "func.h"

//#define STR(s) #s

using namespace std;

int main( int argc, char** argv )
{
	//string filePath = "./img/scan2";
	//string img = "./img/ws/ws5_03.jpg";
	string filePath;
	//string filePath = "E:/Photos/img/scan_ws";
	string outputPath;
	int type;
	systemInit(filePath,outputPath,type);

	//std::experimental::filesystem::remove_all(outputPath.c_str());

	Mat src = imread("e:/Photos/img/scan_ws/CCF01152017 - Copy (2).jpg");
	vector<string>res;
	moveFiles(src, 0, res);
	cout << "The result:" << endl;
	for (auto &s:res)
	{
		cout << s << endl;
	}
	//moveFiles(filePath, outputPath, type, false);
	//moveFiles(filePath, outputPath, WORKSHEET, false);

	systemEnd();

	cin.get();
    return 0;
}