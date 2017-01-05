#pragma once
#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <zbar.h>
#include <baseapi.h>

class Page
{
public:
	Page(cv::Mat &img);
	~Page() {};
	cv::Mat getSrc_color() { return src_color; }
	cv::Mat getProcessedImg() { return src_color_clone; }
	cv::Mat getGrayImg() { return src_gray; }
	virtual void drewCompute() {};
	virtual void putOcrText() {};
	int ocrEngineInit();

protected:
	const cv::Mat src_color;
	cv::Mat src_color_clone;
	cv::Mat src_gray;
	tesseract::TessBaseAPI *tess=new tesseract::TessBaseAPI();
};

class WorkSheet:public Page
{
public:
	virtual void putOcrText();
	virtual void drewCompute();
	void dftAllRect();
	void decode();//barcode decode
	void drawRotated();
	void drawRects();
	void getBarCodePositon();
	void findOcrRects();
	WorkSheet(cv::Mat &img);
	~WorkSheet() {};

private:
	std::vector<cv::RotatedRect> rRects;
	std::vector<cv::Rect> rects;
	std::vector<cv::Rect> ocrRects;
	std::string indNum="indNum";
	std::string approach = "approach";
};
