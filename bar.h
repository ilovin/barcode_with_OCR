#pragma once
#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <zbar.h>
#include <baseapi.h>
#include "ClearImageBarcode.h"

void tessrInit();
class Page
{
public:
	Page(cv::Mat &img);
	Page() {};
	~Page() ;
	cv::Mat getSrc_color() { return src_color; }
	cv::Mat getProcessedImg() { return src_color_clone; }
	cv::Mat getGrayImg() { return src_gray; }
	virtual void drewCompute() = 0;
	virtual void putOcrText() = 0;
	int ocrEngineInit();
	void roiOcr();

protected:
	static void on_mouse(int event, int x, int y, int flags, void *param);
	const cv::Mat src_color;
	cv::Mat src_color_clone;
	cv::Mat src_gray;
	tesseract::TessBaseAPI *tess=new tesseract::TessBaseAPI();
};

class WorkSheet:public Page
{
public:
	void process();
	void drewCompute();
	void getBarCodePositon();
	void findOcrRects();

	void zbar_decode();//barcode decode
	void inlite_decode();
	void putOcrText();//ocr the ocrRect and put text on the img
	void ocrByChar();
	void dftAllRect();

	void drawRotated();
	void drawRects();

	std::string getSerial();
	std::string getApproach();
	WorkSheet(cv::Mat &img);
	WorkSheet() ;
	~WorkSheet() ;

private:
	std::vector<cv::RotatedRect> rRects;//barcode
	std::vector<cv::Rect> rects;//barcode
	std::vector<cv::Rect> ocrRects;//text under barcode
	std::string serial;
	std::string approach;
	ICiServerPtr Ci;
	ICiBarcodeProPtr BcIter;
};
