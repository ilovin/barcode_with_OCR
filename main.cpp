#include "stdafx.h"
#include <iostream>
#include "movefile.h"
#include <experimental\filesystem>
#include "page.h"

//#define STR(s) #s

using namespace std;

int main()
{
	//string filePath = "./img/scan2";
	//string img = "./img/ws/ws5_03.jpg";
	systemInit();
	string filePath = "E:/Photos/img/scan_form";
	//string filePath = "E:/Photos/img/scan_ws";
	string outputPath = "D:/tmp/output";

	std::experimental::filesystem::remove_all(outputPath.c_str());

	moveFiles(filePath, outputPath, FORM, false);
	//moveFiles(filePath, outputPath, WORKSHEET, false);

	systemEnd();
	cin.get();
    return 0;
}