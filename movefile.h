#pragma once
#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include "page.h"
#include "directory.h"

using namespace std;
using namespace cv;

int moveFiles(string &inF, string &outF, int fileType, bool IsRecursive);