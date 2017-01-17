#include "stdafx.h"
#include <iostream>
#include "movefile.h"
#include <experimental\filesystem>

//#define STR(s) #s

using namespace std;

int main()
{
	//string filePath = "./img/scan2";
	//string img = "./img/ws/ws5_03.jpg";
	string filePath = "E:/Photos/img/scan3";
	string outputPath = "D:/tmp/output";
	std::experimental::filesystem::remove_all(outputPath.c_str());

	moveFiles(filePath, outputPath, WORKSHEET, false);
	cin.get();
    return 0;
}