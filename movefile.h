#pragma once
#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include "bar.h"
#include "directory.h"

#define WORKSHEET 0
#define REPORT 1

using namespace std;
using namespace cv;

int moveFiles(string &inF, string &outF, int fileType, bool IsRecursive);