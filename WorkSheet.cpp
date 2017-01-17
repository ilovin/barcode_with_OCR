// barCode.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <iostream>
#include <memory>//unique_ptr
#include <string>//touuper
#include <regex>
#include <zbar.h>
#include <baseapi.h>
#include <opencv2\opencv.hpp>
#include "page.h"
#include "DFT.h"
#include <baseapi.h>
#include <allheaders.h>
#include <set>

#define STR(s) #s

//#define DEBUG 
using namespace std;
using namespace cv;
using namespace zbar;

#define amplify_scale 1.9 //1.9  1
#define MAGICWORD "SHA"
#define TOINT(x) (static_cast<int>(x*amplify_scale))

#ifndef isInit
#define TOODD(x) (TOINT(x)%2==0?(TOINT(x)+1):TOINT(x))
//drewcompute 
//#define blur_w amplify_scale*15
#define th_bar_drew 160//210 160
#define th_bar 50//70 50
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
#define oneBar_h TOODD(40)
#define twoBar_h TOODD(70)
#define isInit 
#endif // !isInit

//int main()
//{
//	string fileName = "img/ws/ws3_1.jpg";
//	Mat src_color = imread(fileName, 1);
//	Mat src;
//	if (src_color.empty())
//	{
//		cerr << "cannot open the IMG" << endl;
//		cin.get();
//		return -1;
//	}
//	WorkSheet ws(src_color);
//	ws.getBarCodePositon();
//	ws.drawRotated();
//	//ws.drawRects();
//	
//	//imshow("class", ws.getSrc_color());
//	//cvtColor(src_color, src, CV_BGR2GRAY);
//	//normalize(src,src,0,255,NORM_MINMAX);  
//
//	//while (src.rows>900||src.cols>1600)
//	//{
//	//	resize(src, src, Size(src.cols * 2 / 3, src.rows * 2 / 3));
//	//}
//	//vector<Rect> possibleRect;
//
//	//decode(src, possibleRect);
//	//draw RotateBox
//
//	imshow("color_res", ws.getProcessedImg());
//	waitKey(0);
//    return 0;
//}

//extern tesseract::TessBaseAPI tessr; 

struct decodeInformation
{
	double pos = -1;
	string info;
	decodeInformation(double x, string y):pos(x),info(y) {}
};



WorkSheet::WorkSheet(cv::Mat &img) :Page(img) {
	//CoInitialize(NULL);
	//  Initailzie COM  could not init
	CoInitialize(NULL);
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
	BcIter->Encodings = (EBarcodeEncoding)129;//106
	BcIter->Algorithm = cibBestRecognition;
}

WorkSheet::WorkSheet()
{
	BcIter = Ci->CreateBarcodePro();
	BcIter->AutoDetect1D = ciTrue;
	BcIter->ValidateOptChecksum = ciFalse;
	BcIter->Directions = (FBarcodeDirections)(cibHorz | cibVert | cibDiag);
	BcIter->Type = (FBarcodeType) (cibfPostnet);
	BcIter->Encodings = (EBarcodeEncoding)129;//106
	BcIter->Algorithm = cibBestRecognition;
}

WorkSheet::~WorkSheet()
{
	CoUninitialize();
}

void imgEnhance_barcode(cv::Mat &src) {
	if (src.channels() > 1) cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
	//bitwise_not(src, src);
	cv::blur(src, src, cv::Size(1, 2));
	double alpha = 0.5;
	int beta = 0;
	for (int y = 0; y < src.rows; y++)
	{
		for (int x = 0; x < src.cols; x++)
		{
			src.at<uchar>(y, x) = saturate_cast<uchar>(alpha*src.at<uchar>(y, x) + beta);
			//for( int c = 0; c < src.channels(); c++ )
			//{
			//    //src.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( src.at<Vec3b>(y,x)[c] ) + beta );
			//    src.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( src.at<Vec3b>(y,x)[c] ) + beta );
			//}
		}
	}
}

void WorkSheet::zbar_decode() {
	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	for (auto &r : rects) {
		cout << (r.br() + r.tl()).y*1.0 / 2 / src_gray.rows<<endl;
		//Rect tmp = Rect(r.tl().x, r.tl().y, (r.br().x - r.tl().x) / 4 * 4, r.br().y - r.tl().y);
		Mat roi; //= Mat(src, tmp);
		src_gray(r).convertTo(roi, src_gray.type(), 1, 0);
		//imgEnhance_barcode(roi);
		//threshold(roi, roi, 0, 255, THRESH_BINARY|THRESH_OTSU);
		//imshow("roi", roi);
		//waitKey(0);
		int width = roi.cols;
		int height = roi.rows;
		uchar *raw = (uchar *)(roi.data);
		Image image(width, height, "Y800", raw, width*height);
		int n = scanner.scan(image);
		for (Image::SymbolIterator symbol = image.symbol_begin();
			symbol != image.symbol_end(); ++symbol) {
			cout << "decoded:" << symbol->get_type_name()
				<< " symbol : " << symbol->get_data() << endl;
		}
		image.set_data(NULL, 0);
	}
}

void WorkSheet::inlite_decode()
{
	vector<decodeInformation> infoList;
	int cnt = 0;
	stringstream ss;
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
		double pos = (r.tl().y + r.br().y)*1.0 / 2 / src_gray.rows;
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
			t = cv::getTickCount();
			string res;
			if (!ReadBarcodePro_my (BcIter, s, 1,res)) continue;
			infoList.push_back(decodeInformation(pos, res));

			tt = (cv::getTickCount() - t) / cv::getTickFrequency();
			cout << res<<" use time " << tt << endl;
	    }
	    catch (_com_error &ex)
	    {
	    	dump_com_error(ex);
	    	cout << "Press Enter to continue" << endl;
	    	cout.flush();
	    	getchar();
	    }
	}
	//try to allocate to serial and approach
	double firstPos, secondPos=0.25;
	for (auto &i:infoList) {
		if (i.pos<0.1)
		{
			secondPos = i.pos + 0.2;
			if (i.info.size() < 5) continue;
			if (i.info.find(MAGICWORD)!=string::npos)
			{
				string str = i.info;
				serial = str.substr(str.find(MAGICWORD), str.find(".")-str.find(MAGICWORD));
			}
		}
		else if (i.pos<secondPos)
		{
			if (i.info.size() < 5) continue;
			approach = i.info;
		}
		else
		{
			string str = i.info;
			if (str.size() < 5||!serial.empty()||str.find(MAGICWORD)==string::npos) continue;
			serial = str.substr(str.find(MAGICWORD), str.find(".")-str.find(MAGICWORD));
		}
	}
}

void WorkSheet::drawRotated()
{
	for (auto &r:rRects)
	{
		Point2f rect_points[4];
		r.points(rect_points);
		for (int i = 0; i < 4; i++)
		{
			line(src_color_clone, rect_points[i], rect_points[(i + 1) % 4], Scalar(0, 255, 0), 1, 8);
		}
	}
}

void WorkSheet::drawRects()
{
	for (auto &r : rects) {
		cv::rectangle(src_color_clone, r, Scalar(255, 0, 0), 1, 8, 0);
	}
	for (auto &r : ocrRects) {
		cv::rectangle(src_color_clone, r, Scalar(0, 0, 255), 1, 8, 0);
	}
}

string WorkSheet::getSerial()
{
	return serial;
}

string WorkSheet::getApproach()
{
	return approach;
}

Mat preprocess2(Mat& im)
{
    // 1) assume white on black and does local thresholding
    // 2) only allow voting top is white and buttom is black(buttom text line)
    Mat thresh;
    //thresh=255-im;
    thresh=im.clone();
    adaptiveThreshold(thresh,thresh,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,15,-1);
	//threshold(thresh, thresh, 100, 255, THRESH_BINARY);
    Mat ret = Mat::zeros(im.size(),CV_8UC1);
    for(int x=1;x<thresh.cols-1;x++)
    {
        for(int y=1;y<thresh.rows-1;y++)
        {
            bool toprowblack = thresh.at<uchar>(y-1,x)==0 ||  thresh.at<uchar>(y-1,x-1)==0     || thresh.at<uchar>(y-1,x+1)==0;
            bool belowrowblack = thresh.at<uchar>(y+1,x)==0 ||  thresh.at<uchar>(y+1,    x-1)==0 || thresh.at<uchar>(y+1,x+1)==0;

            uchar pix=thresh.at<uchar>(y,x);
            if((!toprowblack && pix==255 && belowrowblack))
            {
                ret.at<uchar>(y,x) = 255;
            }
        }
    }
    return ret;
}

void imgEnhance(cv::Mat &src) {
	//vector<Mat> splitBGR(src.channels());
	//split(src, splitBGR);
	//for (auto &c : splitBGR)
	//	equalizeHist(c, c);
	//merge(splitBGR, src);
	if(src.channels()>1) cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
	bitwise_not(src,src);
	double alpha = 2;
	int beta = 0;
	for( int y = 0; y < src.rows; y++ )
    {
        for( int x = 0; x < src.cols; x++ )
        {
			src.at<uchar>(y, x) = saturate_cast<uchar>(alpha*src.at<uchar>(y, x) + beta);
            //for( int c = 0; c < src.channels(); c++ )
            //{
            //    //src.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( src.at<Vec3b>(y,x)[c] ) + beta );
            //    src.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( src.at<Vec3b>(y,x)[c] ) + beta );
            //}
        }
    }

	//equalizeHist(src, src);
	cv::blur(src, src, cv::Size(1, 2));
	//cv::adaptiveThreshold(src, src, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY,15,0);
	//src = 255 - src;
	cv::threshold(src, src,0,255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	cv::Mat kernal = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(1, 2));
	cv::morphologyEx(src, src, cv::MORPH_CLOSE, kernal);
	//cv::morphologyEx(src, src, cv::MORPH_OPEN, kernal);

	Mat canny_output; //example from OpenCV Tutorial
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//int thresh = 100;
	//Canny(src, canny_output, thresh, thresh*2, 3);//with or without, explained later.
	//imshow("canny", canny_output);
	//waitKey(0);
	cv::findContours(src, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0,0));
	for( int i = 0; i< contours.size(); i++ )
    {
		//Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		//drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
		if (contours[i].size()<6)
		{
			drawContours(src, contours, i, Scalar::all(0), CV_FILLED);
		}
		if (contourArea(contours[i])<12)
			drawContours(src, contours, i, Scalar::all(0), CV_FILLED);
    }
	//cv::morphologyEx(src, src, cv::MORPH_CLOSE, kernal);
	//dilate(src, src, kernal);
	
	//for (vector<vector<Point> >::iterator it = contours.begin(); it!=contours.end(); )
	//{
	//	{
	//	}
	//}
	//cv::blur(src, src, cv::Size(4, 4));
	//cv::imshow("enhance", src);
	//cv::waitKey(0);
}

void WorkSheet::putOcrText()
{
	cv::Size textSize;
	int baseline = 0;
	int fontFace = FONT_HERSHEY_COMPLEX;
	double fontScale = 1;
	std::string text;
	if (!ocrRects.size()) {
		text = "no barcode detected";
		textSize = getTextSize(text, fontFace, fontScale,1, &baseline);
		cv::Point textOrg(0, textSize.height);
		cv::putText(src_color_clone,text, textOrg, fontFace, fontScale, Scalar(0, 0, 255));
	}
	else
	{
		vector<decodeInformation> infoList;
		int count=0;
		for (auto &r:ocrRects)
		{
			if (!r.area())
				continue;
			Mat roi = src_color_clone(r);
			imgEnhance(roi);
			double pos = (r.tl().y + r.br().y)*1.0 / 2 / src_gray.rows;
			//cout << pos << endl;
			//double resizeScale = 30.0 / roi.rows;
			//resize(roi, roi, Size(roi.cols*resizeScale,roi.rows*resizeScale));
			//tess.SetImage((uchar*)roi.data, roi.size().width, roi.size().height, 
			tess->SetImage((uchar*)roi.data, roi.size().width, roi.size().height, 
			roi.channels(), roi.step1());
			tess->Recognize(0);
			//Boxa* boxes = tess->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
			//tess.Recognize(0);

			//draw the box
			//for (int i = 0; i < int(boxes->n); i++)
			//{
			//	Box* box = (boxes->box)[i];
			//	Rect r((int)box->x, (int)box->y, (int)box->w, (int)box->h);
			//	rectangle(roi, r, Scalar::all(255));
			//}

#ifdef DEBUG
			//show the ocr roi
			if (!count)
				imshow("roi1", roi);
			else imshow("roi2", roi);
			count++;
#endif // DEBUG

			std::vector<int> hp(roi.cols);
			horizonProjection(roi, hp);
			//for (int k = 0; k < hp.size(); k++)
			//{
			//	hp[k] = 0;
			//	for (int j = 0; j < roi.rows; j++)
			//		hp[k] += roi.ptr<uchar>(j)[k] > 128 ? 1 : 0;
			//	cout << hp[k] << " ";
			//}
			//cout << endl;
			//text = std::unique_ptr<char[]>(tess.GetUTF8Text()).get();
			text = std::unique_ptr<char[]>(tess->GetUTF8Text()).get();
			//cout << text << endl;
			//delete the end \n\r
			text.erase(text.find_last_not_of(" \n\r\t") + 1);
			//delete the non alphanumric
			int 	idx = text.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-_");
			while (idx!=std::string::npos)
			{
				text.erase(idx,1);
				idx = text.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-_");
			}
			//delete the start not alpha, and end not alphanumric
			int last = text.find_last_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
			//std::transform(text.begin(), text.end(),text.begin(), toupper);
			int first = text.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
			if (first!=std::string::npos&&last!=std::string::npos)
				text = text.substr(first, last - first+1);
			//merge space and -_
			std::regex rxs("\\s+");
			std::regex rxm("-+");
			std::regex rxx("_+");
			text = std::regex_replace(text, rxs, " ");
			text = std::regex_replace(text, rxm, "-");
			text = std::regex_replace(text, rxx, "_");
			infoList.push_back(decodeInformation(pos, text));
			//cout << text << endl;
			textSize = getTextSize(text, fontFace, fontScale,1, &baseline);
			cv::Point textOrg(r.tl().x, r.tl().y);
			cv::putText(src_color_clone,text, textOrg, fontFace, fontScale, Scalar(255, 0, 0));
		}
		if (infoList.size())
		{
			if (serial.empty()&&infoList[0].info.find(MAGICWORD)!=string::npos)
				serial = infoList[0].info;
			if (approach.empty())
			{
				if (infoList.size()==1&&infoList[0].info.find(MAGICWORD)==string::npos
					&&infoList[0].info.size()>minStringSize)
					approach = infoList[0].info;
				if (infoList.size()>1&&infoList[1].pos>0.1&&infoList[1].pos<0.2
					&&infoList[1].info.size()>minStringSize)
					approach = infoList[1].info;
			}

		}
	}
}

void WorkSheet::drewCompute() {
	int scale = 1, delta = 0, ddepth = CV_16S;
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Sobel(src_gray, grad_x, ddepth, 1, 0, -1, scale, delta, BORDER_DEFAULT);
	Sobel(src_gray, grad_y, ddepth, 0, 1, -1, scale, delta, BORDER_DEFAULT);
	Mat gradient;
	convertScaleAbs(grad_x, abs_grad_x);
	convertScaleAbs(grad_y, abs_grad_y);
	//subtract(grad_x, grad_y, gradient);
	subtract(abs_grad_x, abs_grad_y, gradient);
	convertScaleAbs(gradient, gradient);
	Mat blurred,gblurred;
	//GaussianBlur(gradient, gblurred, Size(15, 9),0,0);
	blur(abs_grad_x, blurred, Size(blur_w, blur_h));
	//blur(gradient, blurred, Size(blur_w, blur_h));
	Mat th;
	threshold(blurred, th, th_bar_drew, 255, THRESH_BINARY);
	//imshow("thdrew", th);
	Mat kernal = getStructuringElement(MORPH_RECT, Size(kernal_l_w,kernal_l_h));
	//Mat kernalo = getStructuringElement(MORPH_RECT, Size(71,11));
	Mat kernalo = getStructuringElement(MORPH_RECT, Size(kernal_o_w,kernal_o_h));
	Mat closed;
	morphologyEx(th, closed, MORPH_CLOSE, kernal);
	erode(closed, closed,kernalo,Point(-1,-1),2);
	dilate(closed, closed,kernalo,Point(-1,-1),4);
	//morphologyEx(closed, closed, MORPH_OPEN, kernalo);

	//contour
	std::vector<std::vector<cv::Point> > contours;
	//std::vector<std::vector<cv::Point> > contours_poly(contours.size());
	std::vector<Vec4i> hierarchy;
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
			rRects.push_back(rRect);
			rects.push_back(rect);
			//cout << rRect.angle << endl;
		}
	}
	sort(rects.begin(), rects.end(),
		[&](Rect x, Rect y) {return (x.br() + x.tl()).y < (y.br() + y.tl()).y; });
	sort(rRects.begin(), rRects.end(),
		[](cv::RotatedRect a, cv::RotatedRect b) {return a.center.y < b.center.y; });

	if (!rects.size()) return;
	double angle = 0;
	if (rects.size()==1)
	{
		if (rRects[0].angle >= -45) angle = -rRects[0].angle;
		else angle = 90 + rRects[0].angle;
		std::cout << "angle: " << angle << endl;
	}
	else{
	for (auto r:rects)
	{
		if ((r.br()+r.tl()).y*1.0/2/src_gray.rows>0.1)
		{
			Mat roi = src_gray(r).clone();
			cv::Size size = roi.size();
			cv::bitwise_not(roi, roi);
			roi = preprocess2(roi);
			//adaptiveThreshold(roi, roi, 255, CV_ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,15,-2);
			//imshow("roi", roi);
			//cv::waitKey(0);
			std::vector<cv::Vec4i> lines;
			cv::HoughLinesP(roi, lines, 1, CV_PI / 720, 100, size.width / 2.f, TOINT(30));
			cv::Mat disp_lines(size, CV_8UC3, cv::Scalar::all(0));
			unsigned nb_lines = lines.size();
			if (!nb_lines) {
				cerr << "cannot drew the pic" << endl;
				return;
			}
			for (int i = 0; i < nb_lines; i++)
			{
				cv::line(disp_lines, cv::Point(lines[i][0], lines[i][1]),
					cv::Point(lines[i][2], lines[i][3]), Scalar(255, rand()%255, 0));
				angle += atan2((double)lines[i][3] - lines[i][1],
					(double)lines[i][2] - lines[i][0]);
				//cout << angle << endl;
			}
			angle /= nb_lines;
			angle = angle*180/CV_PI;
			std::cout << "angle: " << angle << endl;
			//imshow("res", disp_lines);
			break;
		}
	}
	}
	//cout << "the angle " << angle << endl;
	if (angle!=0)
	{
		cv::Point2f center(src_gray.cols / 2, src_gray.rows / 2);
		Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
		cv::Rect bbox = cv::RotatedRect(center, src_gray.size(), angle).boundingRect();
		//cout << bbox << endl;
		rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
		rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;
		Mat dst;
		warpAffine(src_gray, src_gray, rot, bbox.size());
		warpAffine(src_color_clone, src_color_clone, rot, bbox.size());
		//warpAffine(src_gray, dst, rot, bbox.size());
		//imshow("warpAffine", src_gray);
	}

}

void WorkSheet::ocrByChar()
{
	//tessrInit();
	cv::Size textSize;
	int baseline = 0;
	int fontFace = FONT_HERSHEY_COMPLEX;
	double fontScale = 1;
	std::string text;
	if (!ocrRects.size()) {
		text = "no barcode detected";
		textSize = getTextSize(text, fontFace, fontScale,1, &baseline);
		cv::Point textOrg(0, textSize.height);
		cv::putText(src_color_clone,text, textOrg, fontFace, fontScale, Scalar(0, 0, 255));
	}
	else
	{
		string winName;
		stringstream ss;
		int cnt = 0;
		for (auto &r : ocrRects)
		{
			if (!r.area())
				continue;
			vector<int> charPos;
			vector<cv::Rect> charRoi;
			Mat roi = src_color_clone(r);
			imgEnhance(roi);
			vector<int> hp(roi.cols);
			horizonProjection(roi, hp);
			for (auto &i:hp)
				cout << i << " ";
			cout << endl;
			//vector<int> vp(roi.rows);
			vector<int>::iterator it=hp.begin();
			int pos = 0;
			while (it!=hp.end())
			{
				//it = find_if(it, hp.end(), [&](int x) {return x > 0.08*roi.rows; });
				it = find_if(it, hp.end(), [&](int x) {return x > 1; });
				if (it == hp.end()) break;
				pos = it-hp.begin();
				charPos.push_back(pos);
				//pos = pos < 0 ? 0 : pos;
				line(roi, Point(pos, 0), Point(pos,roi.rows - 1), Scalar::all(255));
				it = find_if(it+1, hp.end(), [&](int x) {return x < 1; });
			}
			charPos.push_back(it - hp.begin());//the last char end
            //roi the char
			if (charPos.size() > 1 && charPos[0] > 1) charPos[0] -= 1;
			for (int i = 1; i < charPos.size(); i++)
				charRoi.push_back(Rect(Point(charPos[i - 1], 0), Point(charPos[i], roi.rows - 1)));
			auto ocr = [&](cv::Rect &roiC) {
				if (!roiC.area()) return;
				cv::Mat roiPic = roi(roiC).clone();
				//imgEnhance(roiPic);
				tessr.SetImage((uchar*)roiPic.data, roiPic.size().width, roiPic.size().height,
					roiPic.channels(), roiPic.step1());
				tessr.Recognize(0);
				string res = std::unique_ptr<char[]>(tessr.GetUTF8Text()).get();
				res.erase(res.find_last_not_of("\t\r\n") + 1);
				cout << res;
			};
			for (auto &rChar : charRoi) {
				ocr(rChar);
			}
			cout << endl;

			ss << cnt++;
			winName = ss.str();
			imshow(winName, roi);
		}

	}
}


void WorkSheet::getBarCodePositon()
{
	if (!rects.size()) return;
	rRects.clear();
	rects.clear();
	int scale = 1, delta = 0, ddepth = CV_16S;
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Sobel(src_gray, grad_x, ddepth, 1, 0, -1, scale, delta, BORDER_DEFAULT);
	Sobel(src_gray, grad_y, ddepth, 0, 1, -1, scale, delta, BORDER_DEFAULT);
	Mat gradient;
	convertScaleAbs(grad_x, abs_grad_x);
	convertScaleAbs(grad_y, abs_grad_y);
	//subtract(grad_x, grad_y, gradient);
	subtract(abs_grad_x, abs_grad_y, gradient);
	convertScaleAbs(gradient, gradient);
	//if (gradient.empty())
	//{
	//	cerr << "cannot produce gradient" << endl;
	//	cin.get();
	//	return - 2;
	//}
	Mat blurred,gblurred;
	//GaussianBlur(gradient, gblurred, Size(15, 9),0,0);
	//blur(abs_grad_x, blurred, Size(blur_w, blur_h));
	blur(gradient, blurred, Size(blur_w, blur_h));
	Mat th;
	threshold(blurred, th, th_bar, 255, THRESH_BINARY);
	//threshold(blurred, th, 0, 255, THRESH_BINARY|THRESH_OTSU);
	//imshow("th", th);
	//waitKey(0);
	Mat kernal = getStructuringElement(MORPH_RECT, Size(kernal_l_w,kernal_l_h));
	//Mat kernalo = getStructuringElement(MORPH_RECT, Size(71,11));
	Mat kernalo = getStructuringElement(MORPH_RECT, Size(kernal_o_w,kernal_o_h));
	Mat closed;
	morphologyEx(th, closed, MORPH_CLOSE, kernal);
	erode(closed, closed,kernalo,Point(-1,-1),2);
	dilate(closed, closed,kernalo,Point(-1,-1),3);
	//morphologyEx(closed, closed, MORPH_OPEN, kernalo);

	//contour
	std::vector<std::vector<cv::Point> > contours;
	//std::vector<std::vector<cv::Point> > contours_poly(contours.size());
	std::vector<Vec4i> hierarchy;
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
			rRects.push_back(rRect);
			rects.push_back(rect);
			//cout << rRect.angle << endl;
		}
	}
	sort(rects.begin(), rects.end(),
		[&](Rect x, Rect y) {return (x.br() + x.tl()).y < (y.br() + y.tl()).y; });
	sort(rRects.begin(), rRects.end(),
		[](cv::RotatedRect a, cv::RotatedRect b) {return a.center.y < b.center.y; });
}
void WorkSheet::findOcrRects() {
	if (!rects.size()) return;
	if (rects.size() == 1) {
		Rect r(Point(rects[0].tl().x, rects[0].br().y+TOINT(20)), Size(rects[0].width, oneBar_h));
		ocrRects.push_back(r);
	}
	else{
		for (int i = 0; i < 2; i++)
		{
			Rect r(Point(rects[i].tl().x, rects[i].br().y+TOINT(5)), Size(rects[i].width, twoBar_h));
			//Mat roi = src_gray(r).clone();
			Mat roi = src_gray(r).clone();
			//bitwise_not(roi,roi);
			threshold(roi, roi, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
			//roi = preprocess2(roi);

			//vertical projection
			//std::cout << roi.size() << endl;
			std::vector<int> vp(roi.rows);
			verticalProjection(roi, vp);
			//cout << endl;

			//next,find the top and bottom of the character
			int top, bottom;
			int count = 0;
			std::vector<int>::reverse_iterator it,itt;
			it = find_if(vp.rbegin(), vp.rend(), [=](int x) {return x > 0.5*roi.cols; });
			it = find_if(it, vp.rend(), [=](int x) {return x < 0.1*roi.cols; });
			bottom = it.base() - vp.begin();
			//while (count<1&&it!=vp.rend())
			//{
			//	itt = it;
			//	it = find_if(++it, vp.rend(), [=](int x) {return x < 0.05*roi.cols; });
			//	if (it == itt + 1) count++;
			//	else count = 0;
			//}
			//bottom -= 2;
			it = find_if(it, vp.rend(), [=](int x) {return x > 0.15*roi.cols; });
			it = find_if(it, vp.rend(), [=](int x) {return x < 0.05*roi.cols; });
			count = 0;
			while (count<TOINT(5)&&it!=vp.rend())
			{
				itt = it;
				it = find_if(++it, vp.rend(), [=](int x) {return x < 0.05*roi.cols; });
				if (it == itt + 1) count++;
				else count = 0;
			}
			top = it.base() - vp.begin();
			//cout << "top " << top << "bottom " << bottom << endl;
			roi = roi(Rect(0, top, roi.cols, bottom - top));
			//rects[i] = roi;

			//Next,cut the left and right
			//horizon projection
			std::vector<int> hp(roi.cols);
			horizonProjection(roi, hp);
			//cout << endl;
			//find the border of left and rigtht
			int left, right;
			it = find_if(hp.rbegin(), hp.rend(), [=](int x) {return x > 0.1*roi.rows; });
			right = it.base() - hp.begin()+TOINT(10);
			count = 0;
			it = find_if(it, hp.rend(), [=](int x) {return x < TOINT(2); });
			while (count<TOINT(13)&&it!=hp.rend())
			{
				itt = it;
				it = find_if(++it, hp.rend(), [=](int x) {return x < TOINT(2); });
				if (it == itt + 1) count++;
				else count = 0;
			}
			//cout << "end -beging:: " << hp.rend().base() - hp.begin() <<hp.rbegin().base()-hp.end()<< endl;
			left = it.base() - hp.begin();
			//cout << "left: " << left << "right: " << right << endl;
			roi = roi(Rect(left, 0, roi.cols - left, roi.rows));
			r = Rect(cv::Point(r.tl().x+left,r.tl().y+top),cv::Size(right-left,bottom-top));
			ocrRects.push_back(r);
			
			//imshow("roi", roi);
			//waitKey(0);
		}
	}

}
void WorkSheet::dftAllRect() {
	for (auto r:rects)
	{
		Mat dft_res,dft;
		Mat roi = src_gray(r);
		DFT(roi, dft_res);
		dft = show_spectrum_magnitude(dft_res, true);
		imshow("dft", dft);
		cv::waitKey(0);
	}
}

void WorkSheet::process() {
	if (ocrEngineInit()) { cin.get(); return; }
	//double t, tt;
	//t = cv::getTickCount();
	drewCompute();
	//tt = (cv::getTickCount() - t) / cv::getTickFrequency();
	//cout << "drew time" << tt<<endl;
	//t = cv::getTickCount();
	getBarCodePositon();
	//tt = (cv::getTickCount() - t) / cv::getTickFrequency();
	//cout << "getBar time" << tt<<endl;
	//t = cv::getTickCount();
	findOcrRects();
	//tt = (cv::getTickCount() - t) / cv::getTickFrequency();
	//cout << "findocr time" << tt<<endl;
	//t = cv::getTickCount();

	inlite_decode();
	//tt = (cv::getTickCount() - t) / cv::getTickFrequency();
	//cout << "decode time" << tt<<endl;
	//t = cv::getTickCount();

	//zbar_decode();
	putOcrText();
	//tt = (cv::getTickCount() - t) / cv::getTickFrequency();
	//cout << "putOcr time" << tt<<endl;
	//t = cv::getTickCount();

	//roiOcr(src_color_clone);
	//ocrByChar();
	drawRects();
	//while (src_color_clone.rows>1000||src_color_clone.cols>1900)
	//	resize(src_color_clone, src_color_clone, Size(src_color_clone.cols * 2 / 3, src_color_clone.rows * 2 / 3));
	//ws.drawRotated();
}
