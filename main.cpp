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

	moveFiles(filePath, outputPath, type, false);
	//moveFiles(filePath, outputPath, WORKSHEET, false);

	systemEnd();
	//cin.get();
    return 0;
}