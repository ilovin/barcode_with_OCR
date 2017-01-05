// barCode.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <memory>//unique_ptr
#include <string>//touuper
#include <zbar.h>
#include <baseapi.h>
#include <opencv2\opencv.hpp>
#include "bar.h"
#include "DFT.h"
#include <baseapi.h>

#define STR(s) #s

using namespace std;
using namespace cv;
using namespace zbar;

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

Page::Page(cv::Mat &img):src_color(img)
{
	if (src_color.channels()<3)
	{
		cerr << "this is not a color img" << endl;
	}
	src_color.copyTo(src_color_clone);
	cvtColor(src_color, src_gray, CV_BGR2GRAY);
	normalize(src_gray,src_gray,0,255,NORM_MINMAX);  
}

int Page::ocrEngineInit()
{
	if (tess->Init("e:\\resources\\tesseract\\tessdata","eng",tesseract::OEM_TESSERACT_CUBE_COMBINED))//OEM_CUBE_ONLY;OEM_TESSERACT_LSTM_COMBINED;OEM_LSTM_ONLY;OEM_TESSERACT_CUBE_COMBINED
	{
		cerr << "OCRTess:could not initialize teseract" << endl;
		return -1;
	}
	tess->SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_LINE);
	//cout<<"white "<<tess->SetVariable("tessedit_char_whitelist","0123456789");
	cout<<"whiteList "<<tess->SetVariable("tessedit_char_whitelist","0123456789ABCDEFGHIGKLMNOPQRSTUVWXYZ-_");
	//if (tess.Init("e:\\resources\\tesseract\\tessdata","eng",tesseract::OEM_LSTM_ONLY))
	//{
	//	cerr << "OCRTess:could not initialize teseract" << endl;
	//	return -1;
	//}
	//tess.SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_LINE);
	//cout<<"white "<<tess.SetVariable("tessedit_char_whitelist",
	//	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	//cout<<" black "<<tess.SetVariable("tessedit_char_blacklist","!?@#$%&*()<>_-+=/:;'\"");
}

WorkSheet::WorkSheet(cv::Mat &img) :Page(img) {}

void WorkSheet::decode() {
	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	for (auto &r : rects) {
		cout << (r.br() + r.tl()).y*1.0 / 2 / src_gray.rows<<endl;
		//Rect tmp = Rect(r.tl().x, r.tl().y, (r.br().x - r.tl().x) / 4 * 4, r.br().y - r.tl().y);
		Mat roi; //= Mat(src, tmp);
		src_gray(r).convertTo(roi, src_gray.type(), 1, 0);
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
	cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
	//cv::blur(src, src, cv::Size(1, 1));
	cv::adaptiveThreshold(src, src, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY,15,11);
	//cv::threshold(src, src,0,255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	cv::Mat kernal = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2, 2));
	src = 255 - src;
	cv::morphologyEx(src, src, cv::MORPH_OPEN, kernal);
	//cv::blur(src, src, cv::Size(4, 4));
	cv::imshow("enhance", src);
	cv::waitKey(0);
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
		textSize = getTextSize(text, fontFace, fontScale,2, &baseline);
		cv::Point textOrg(0, textSize.height);
		cv::putText(src_color_clone,text, textOrg, fontFace, fontScale, Scalar(0, 0, 255));
	}
	else
	{
		for (auto &r:ocrRects)
		{
			Mat roi = src_color_clone(r);
			//imgEnhance(roi);
			//tess.SetImage((uchar*)roi.data, roi.size().width, roi.size().height, 
			tess->SetImage((uchar*)roi.data, roi.size().width, roi.size().height, 
			roi.channels(), roi.step1());
			//tess.Recognize(0);
			tess->Recognize(0);
			//text = std::unique_ptr<char[]>(tess.GetUTF8Text()).get();
			text = std::unique_ptr<char[]>(tess->GetUTF8Text()).get();
			cout << text << endl;
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
			cout << text << endl;
			textSize = getTextSize(text, fontFace, fontScale,1, &baseline);
			cv::Point textOrg(r.tl().x, r.br().y+textSize.height);
			cv::putText(src_color_clone,text, textOrg, fontFace, fontScale, Scalar(255, 0, 0));
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
	subtract(grad_x, grad_y, gradient);
	//subtract(abs_grad_x, abs_grad_y, gradient);
	convertScaleAbs(gradient, gradient);
	Mat blurred,gblurred;
	//GaussianBlur(gradient, gblurred, Size(15, 9),0,0);
	blur(abs_grad_x, blurred, Size(15, 9));
	Mat th;
	threshold(blurred, th, 210, 255, THRESH_BINARY);
	Mat kernal = getStructuringElement(MORPH_RECT, Size(31,11));
	//Mat kernalo = getStructuringElement(MORPH_RECT, Size(71,11));
	Mat kernalo = getStructuringElement(MORPH_RECT, Size(31,9));
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
		if (contourArea(contours[i]) > 6000)
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
			cv::HoughLinesP(roi, lines, 1, CV_PI / 720, 100, size.width / 2.f, 30);
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
	subtract(grad_x, grad_y, gradient);
	//subtract(abs_grad_x, abs_grad_y, gradient);
	convertScaleAbs(gradient, gradient);
	//if (gradient.empty())
	//{
	//	cerr << "cannot produce gradient" << endl;
	//	cin.get();
	//	return - 2;
	//}
	Mat blurred,gblurred;
	//GaussianBlur(gradient, gblurred, Size(15, 9),0,0);
	blur(abs_grad_x, blurred, Size(15, 9));
	Mat th;
	threshold(blurred, th, 210, 255, THRESH_BINARY);
	Mat kernal = getStructuringElement(MORPH_RECT, Size(31,11));
	//Mat kernalo = getStructuringElement(MORPH_RECT, Size(71,11));
	Mat kernalo = getStructuringElement(MORPH_RECT, Size(31,9));
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
		if (contourArea(contours[i]) > 6000)
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
		Rect r(Point(rects[0].tl().x, rects[0].br().y+20), Size(rects[0].width, 40));
		ocrRects.push_back(r);
	}
	else{
		for (int i = 0; i < 2; i++)
		{
			Rect r(Point(rects[i].tl().x, rects[i].br().y+5), Size(rects[i].width, 70));
			//Mat roi = src_gray(r).clone();
			Mat roi = src_gray(r).clone();
			//bitwise_not(roi,roi);
			threshold(roi, roi, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
			//roi = preprocess2(roi);

			//vertical projection
			//std::cout << roi.size() << endl;
			std::vector<int> vp(roi.rows);
			for (int k = 0; k < vp.size(); k++)
			{
				vp[k] = 0;
				for (int j = 0; j < roi.cols; j++)
					vp[k] += roi.ptr<uchar>(k)[j] > 128 ? 1 : 0;
				//cout << vp[k]<<" ";
			}
			//cout << endl;

			//next,find the top and bottom of the character
			int top, bottom;
			std::vector<int>::reverse_iterator it,itt;
			it = find_if(vp.rbegin(), vp.rend(), [=](int x) {return x > 0.5*roi.cols; });
			it = find_if(it, vp.rend(), [=](int x) {return x < 0.05*roi.cols; });
			bottom = it.base() - vp.begin();
			//bottom -= 2;
			it = find_if(it, vp.rend(), [=](int x) {return x > 0.15*roi.cols; });
			int count = 0;
			it = find_if(it, vp.rend(), [=](int x) {return x < 0.05*roi.cols; });
			while (count<5&&it!=vp.rend())
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

			//horizon projection
			std::vector<int> hp(roi.cols);
			for (int k = 0; k < hp.size(); k++)
			{
				hp[k] = 0;
				for (int j = 0; j < roi.rows; j++)
					hp[k] += roi.ptr<uchar>(j)[k] > 128 ? 1 : 0;
				//cout << hp[k] << " ";
			}
			//cout << endl;
			//find the border of left and rigtht
			int left, right;
			it = find_if(hp.rbegin(), hp.rend(), [=](int x) {return x > 0.1*roi.rows; });
			right = it.base() - hp.begin();
			count = 0;
			it = find_if(it, hp.rend(), [=](int x) {return x < 2; });
			while (count<19&&it!=hp.rend())
			{
				itt = it;
				it = find_if(++it, hp.rend(), [=](int x) {return x < 2; });
				if (it == itt + 1) count++;
				else count = 0;
			}
			//cout << "end -beging:: " << hp.rend().base() - hp.begin() <<hp.rbegin().base()-hp.end()<< endl;
			left = it.base() - hp.begin();
			//cout << "left: " << left << "right: " << right << endl;
			roi = roi(Rect(left, 0, roi.cols - left, roi.rows));
			r = Rect(cv::Point(r.tl().x+left,r.tl().y+top),cv::Size(r.width-left,bottom-top));
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

