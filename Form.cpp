#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include "page.h"

using namespace std;
using namespace cv;

#define amplify_scale 1.9 //1.9  1
#define TOINT(x) (static_cast<int>(x*amplify_scale))

#ifndef isInit
#define TOODD(x) (TOINT(x)%2==0?(TOINT(x)+1):TOINT(x))
//drewcompute 
//#define blur_w amplify_scale*15
#define th_bar_detect_raw 40//210 160
#define th_bar_detect_precious 55//210 160
//#define th_bar 50//70 50
#define blur_w TOODD(15)
#define blur_h TOODD(9)
#define kernal_l_w TOODD(31)
#define kernal_l_h TOODD(9)
#define kernal_o_w TOODD(31)
#define kernal_o_h TOODD(9)
#define barcodeMinArea TOINT(8000*amplify_scale*amplify_scale)
//ocr
#define minStringSize 5
//findOcrRects
//#define oneBar_h TOODD(40)
//#define twoBar_h TOODD(70)
#define isInit 
#endif // !isInit

void Form::process() {
	//if (ocrEngineInit()) { cin.get(); return; }
	findBarcodeRect();
	inlite_decode();

	drawRects();
	drawRotated();
}

Form::Form(cv::Mat &img):Page(img)
{
	CoInitialize(NULL);
	//  Initailzie COM  could not init
	HRESULT hr = Ci.CreateInstance(__uuidof(CiServer));
	if (FAILED(hr)) _com_issue_error (hr);
	long nMasterId = 0;
	if (nMasterId > 0)
		Ci->OpenExt ((long) GetModuleHandle(NULL), nMasterId, 0);
	BcIter = Ci->CreateBarcodePro();
	BcIter->AutoDetect1D = ciTrue;
	BcIter->ValidateOptChecksum = ciFalse;
	BcIter->Directions = (FBarcodeDirections)(cibHorz | cibVert | cibDiag);
	BcIter->Type = (FBarcodeType) (cibfPostnet);
	BcIter->Encodings = (EBarcodeEncoding)106;//106
	BcIter->Algorithm = cibBestRecognition;
}

Form::Form() {
	new(this)Form(cv::Mat());
}

Form::~Form() 
{
	CoUninitialize();
}

cv::Mat gradientProcess(Mat &gray_src, int thresh) {
	int scale = 1, delta = 0, ddepth = CV_16S;
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Sobel(gray_src, grad_x, ddepth, 1, 0, -1, scale, delta, BORDER_DEFAULT);
	Sobel(gray_src, grad_y, ddepth, 0, 1, -1, scale, delta, BORDER_DEFAULT);
	Mat gradient;
	convertScaleAbs(grad_x, abs_grad_x);
	convertScaleAbs(grad_y, abs_grad_y);
	//subtract(grad_x, grad_y, gradient);
	subtract(abs_grad_x, abs_grad_y, gradient);
	convertScaleAbs(gradient, gradient);
	Mat blurred,gblurred;
	//GaussianBlur(gradient, gblurred, Size(15, 9),0,0);
	//blur(abs_grad_x, blurred, Size(blur_w, blur_h));
	blur(gradient, blurred, Size(blur_w, blur_h));
	Mat th;
	threshold(blurred, th, thresh, 255, THRESH_BINARY);
	//namedWindow("grad_x", cv::WINDOW_NORMAL);
	//imshow("grad_x", th);
	Mat kernal = getStructuringElement(MORPH_RECT, Size(kernal_l_w,kernal_l_h));
	//Mat kernalo = getStructuringElement(MORPH_RECT, Size(71,11));
	Mat kernalo = getStructuringElement(MORPH_RECT, Size(kernal_o_w,kernal_o_h));
	Mat closed;
	morphologyEx(th, closed, MORPH_CLOSE, kernal);
	erode(closed, closed,kernalo,Point(-1,-1),2);
	dilate(closed, closed,kernalo,Point(-1,-1),4);
	morphologyEx(closed, closed, MORPH_OPEN, kernalo);
	//namedWindow("after_morpho", cv::WINDOW_NORMAL);
	//imshow("after_morpho", closed);
	return closed;

}
void Form::findBarcodeRect() {
	//int scale = 1, delta = 0, ddepth = CV_16S;
	//Mat grad_x, grad_y;
	//Mat abs_grad_x, abs_grad_y;
	//Sobel(src_gray, grad_x, ddepth, 1, 0, -1, scale, delta, BORDER_DEFAULT);
	//Sobel(src_gray, grad_y, ddepth, 0, 1, -1, scale, delta, BORDER_DEFAULT);
	//Mat gradient;
	//convertScaleAbs(grad_x, abs_grad_x);
	//convertScaleAbs(grad_y, abs_grad_y);
	////subtract(grad_x, grad_y, gradient);
	//subtract(abs_grad_x, abs_grad_y, gradient);
	//convertScaleAbs(gradient, gradient);
	//Mat blurred,gblurred;
	////GaussianBlur(gradient, gblurred, Size(15, 9),0,0);
	////blur(abs_grad_x, blurred, Size(blur_w, blur_h));
	//blur(gradient, blurred, Size(blur_w, blur_h));
	//Mat th;
	//threshold(blurred, th, th_bar_detect_raw, 255, THRESH_BINARY);
	//namedWindow("grad_x", cv::WINDOW_NORMAL);
	//imshow("grad_x", th);
	//Mat kernal = getStructuringElement(MORPH_RECT, Size(kernal_l_w,kernal_l_h));
	////Mat kernalo = getStructuringElement(MORPH_RECT, Size(71,11));
	//Mat kernalo = getStructuringElement(MORPH_RECT, Size(kernal_o_w,kernal_o_h));
	//Mat closed;
	//morphologyEx(th, closed, MORPH_CLOSE, kernal);
	//erode(closed, closed,kernalo,Point(-1,-1),2);
	//dilate(closed, closed,kernalo,Point(-1,-1),4);
	////morphologyEx(closed, closed, MORPH_OPEN, kernalo);
	//namedWindow("after_morpho", cv::WINDOW_NORMAL);
	//imshow("after_morpho", closed);

	Mat closed = gradientProcess(src_gray, th_bar_detect_raw);
	//contour
	std::vector<std::vector<cv::Point> > contours;
	//std::vector<std::vector<cv::Point> > contours_poly(contours.size());
	std::vector<Vec4i> hierarchy;
	std::vector<Rect> tmp_rects;
	std::vector<RotatedRect> tmp_rRects;
	//vector<RotatedRect> possibleRect;
	findContours(closed, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
	{
		if (contourArea(contours[i]) > barcodeMinArea)
		{
			std::vector<cv::Point> c_poly;
			approxPolyDP(Mat(contours[i]), c_poly, 3, true);
			Rect rect = boundingRect(Mat(c_poly));
			RotatedRect rRect = minAreaRect(contours[i]);
			tmp_rRects.push_back(rRect);
			tmp_rects.push_back(rect);
			//cout << rRect.angle << endl;
		}
	}
	//more precious way
	for (int i = 0; i < tmp_rects.size(); i++)
	{
		cv::Rect r = tmp_rects[i];
		Mat roi = src_gray(r).clone();
		double angle = tmp_rRects[i].angle;
		//cout << "angle: " << angle << endl;
		//if (angle >= -45) angle = angle;
		if(angle<-45) angle = 90 + angle;
		rotateImg(roi, angle);
		closed = gradientProcess(roi, th_bar_detect_precious);
		findContours(closed, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
		bool IsExits = false;
		for (auto &c : contours) {
			if (contourArea(c) > barcodeMinArea) IsExits = true;
		}
		if (IsExits) {
			rRects.push_back(tmp_rRects[i]);
			rects.push_back(tmp_rects[i]);
		}
		//namedWindow("after_morpho", cv::WINDOW_NORMAL);
		//imshow("after_morpho", closed);
		//waitKey(0);
	}


	sort(rects.begin(), rects.end(),
		[&](Rect x, Rect y) {return (x.br() + x.tl()).y < (y.br() + y.tl()).y; });
	sort(rRects.begin(), rRects.end(),
		[](cv::RotatedRect a, cv::RotatedRect b) {return a.center.y < b.center.y; });
}


void Form::drawRects()
{
	for (auto &r : rects) {
		cv::rectangle(src_color_clone, r, Scalar(255, 0, 0), 6, 8, 0);
	}
}

void Form::drawRotated()
{
	for (auto &r:rRects)
	{
		Point2f rect_points[4];
		r.points(rect_points);
		for (int i = 0; i < 4; i++)
		{
			line(src_color_clone, rect_points[i], rect_points[(i + 1) % 4], Scalar(0, 255, 0), 6, 8);
		}
	}
}

void Form::inlite_decode()
{
	int cnt = 0;
	stringstream ss;
	vector<string> decodeInfo;
	for (auto r : rects) {
		Mat roi; //= Mat(src, tmp);
		src_gray(r).convertTo(roi, src_gray.type(), 1, 0);
		vector<int> compression_params;
        compression_params.push_back(IMWRITE_JPEG_QUALITY);
        compression_params.push_back(100);
		ss.str("");
		ss << cnt++;
		string imgWrite = ".\\tmp\\tmp" + ss.str() + ".jpg";
		imwrite(imgWrite, roi,compression_params);
		//waitKey(0);
		double t, tt;
	    try
	    {
	    	//char s[256]=".\\tmp\\tmp.jpg";
			char s[256];
			strcpy(s, imgWrite.c_str());
			//strncpy(s, "d:\\Document\\Project\\Barcode\\Bar\\Bar\\tmp1.jpg",256);
			if (strlen(s) == 0)
				continue;
				// Delete quotes ("xxxx")
			if ((strlen(s) > 2)  && (*s == '\"') && (*(s + strlen(s) - 1) == '\"'))
			{
				strcpy (s, s+1);
				*(s + strlen(s) - 1) = 0;
			}
#ifdef DEBUG
			t = cv::getTickCount();
#endif // !DEBUG
			string res;
			if (!ReadBarcodePro_my (BcIter, s, 1,res)) continue;
			decodeInfo.push_back(res);
			cout << res << endl;
			
#ifdef DEBUG
			tt = (cv::getTickCount() - t) / cv::getTickFrequency();
			cout << res<<" use time " << tt << endl;
#endif // !DEBUG
	    }
	    catch (_com_error &ex)
	    {
	    	dump_com_error(ex);
	    	cout << "Press Enter to continue" << endl;
	    	cout.flush();
	    	getchar();
	    }
	}
	//allocate decode information
	for (auto &code : decodeInfo) {
		if (code.find(MAGICWORD) != string::npos&&code.size() >= minStringSize)
			serials.insert(code);
	}
}

std::set<std::string> Form::getSerials()
{
	return serials;
}
