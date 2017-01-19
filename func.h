#pragma once
#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <baseapi.h>
#include <experimental\filesystem>

#define MAGICWORD "SHA"
#define TESSDATADIR "e:\\resources\\tesseract\\tessdata"
#define DEFAULT_IN_WS_DIR "E:/Photos/img/scan_ws" 
#define DEFAULT_IN_FM_DIR "E:/Photos/img/scan_form" 
#define DEFAULT_OUT_DIR "D:/tmp/output"

using namespace std;
using namespace cv;

extern tesseract::TessBaseAPI tessr; 
void roiOcr(cv::Mat &src_color);

int horizonProjection(cv::Mat &gray, std::vector<int> &hp);
int verticalProjection(cv::Mat &gray, std::vector<int> &vp);
cv::Mat rotateImg(cv::Mat &src, double &angle);
void systemInit(string &inDir,string &outDir,int &type);
void systemEnd();

