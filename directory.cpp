#include "stdafx.h"
#include "directory.h"
#include <experimental\filesystem>
#include <iostream>
#include <set>
namespace fs = std::experimental::filesystem;
using namespace std;

std::string suffix[6] = { "jpg","png","jpeg","tif","gif","bmp" };
static std::set<std::string> suffixSet(suffix, suffix + sizeof(suffix) / sizeof(suffix[0]));

int getImgList(std::string &path, std::vector<std::string> &imgList, bool isRecursive) {
	if (!fs::exists(path))
	{
		std::cerr << "cannot open the directory: " << path<<endl;
		return -1;
	}
	//else
	//{
	//	std::cout << "total num in directory is: " << endl;
	//}
	//fs::directory_iterator itD(path);
	//fs::recursive_directory_iterator itRd(path);
	if (isRecursive)
	{
		for (auto &p:fs::recursive_directory_iterator(path))
		{
			string filePath = p.path().string();
			string suf = filePath.substr(filePath.rfind('.') + 1);
			transform(suf.begin(), suf.end(), suf.begin(), ::tolower);
			if (suffixSet.find(suf)!=suffixSet.end())
				imgList.push_back(filePath);
		}
	}
	else
	{
		for (auto &p : fs::directory_iterator(path)) {
			string filePath = p.path().string();
			string suf = filePath.substr(filePath.rfind('.') + 1);
			transform(suf.begin(), suf.end(), suf.begin(), ::tolower);
			if (suffixSet.find(suf)!=suffixSet.end())
				imgList.push_back(filePath);

		}
	}
	return 0;
}