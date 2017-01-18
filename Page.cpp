#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <memory>//unique_ptr
#include "page.h"

#define TESSDATADIR "e:\\resources\\tesseract\\tessdata"
//#define TESSDATADIR "D:\\Document\\code\\tesseract\\tessdata"
using namespace std;
using namespace cv;

int horizonProjection(cv::Mat &gray, std::vector<int> &hp) {
	for (int k = 0; k < hp.size(); k++)
	{
		hp[k] = 0;
		for (int j = 0; j < gray.rows; j++)
			hp[k] += gray.ptr<uchar>(j)[k] > 128 ? 1 : 0;
		//cout << hp[k] << " ";
	}

	return 0;
}

int verticalProjection(cv::Mat &gray, std::vector<int> &vp) {
	for (int k = 0; k < vp.size(); k++)
	{
		vp[k] = 0;
		for (int j = 0; j < gray.cols; j++)
			vp[k] += gray.ptr<uchar>(k)[j] > 128 ? 1 : 0;
		//cout << vp[k]<<" ";
	}
	return 0;
}


//tesseract::TessBaseAPI tessr; 
tesseract::TessBaseAPI tessr; 
const char* workArea = "select ROI";
bool clicked = false;
cv::Point pt1, pt2;
static void on_mouse(int event, int x, int y, int flags, void *param) {
	Mat *img = (Mat *)param;
	Mat src = *img;
	static cv::Mat dst;
	auto ocr = [&](cv::Rect &roi) {
		if (!roi.area()) return;
		cv::Mat roiPic = src(roi).clone();
		//imgEnhance(roiPic);
		tessr.SetImage((uchar*)roiPic.data, roiPic.size().width, roiPic.size().height, 
			roiPic.channels(), roiPic.step1());
		tessr.Recognize(0);
		string res = std::unique_ptr<char[]>(tessr.GetUTF8Text()).get() ;
		res.erase(res.find_last_not_of("\t\r\n")+1);
		cout << res;
	};
	auto func = [&](cv::Scalar &s)->void {
		cv::Rect roi;
		roi = cv::Rect(pt1, pt2);
		if (roi.area())
		{
			dst = src.clone();
			rectangle(dst, roi, s, 1, 8, 0); 
			imshow(workArea, dst);
			//if (!clicked) cout << roi << endl;
		}
	};
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		clicked = true;
		pt1 = cv::Point(x, y);
		//pt2 = cv::Point(x, y);
		break;
	case CV_EVENT_LBUTTONUP:
		clicked = false;
		pt2 = cv::Point(x, y);
		if (pt2.x < 0) pt2.x = 0;
		if (pt2.y < 0) pt2.y = 0;
		if (pt2.x > src.cols-1) pt2.x = src.cols-1;
		if (pt2.y > src.rows-1) pt2.y = src.rows-1;
		func(cv::Scalar(255, 0, 128));
		ocr(cv::Rect(pt1, pt2));
		break;
	case CV_EVENT_MOUSEMOVE:
		if (clicked) {
			pt2 = cv::Point(x, y);
			func(cv::Scalar(0, 255, 0));
		}
	break;
	default:
		break;
	}
}

void roiOcr(cv::Mat &src_color)
{
	cv::namedWindow(workArea, cv::WINDOW_AUTOSIZE);
	cv::setMouseCallback(workArea, on_mouse, (void*)&src_color);
	imshow(workArea, src_color);
	int key = 0;
	while (key!=27)
	{
		key = cv::waitKey(0);
	}
}

void roiOcrEngineInit() {
	if (tessr.Init(TESSDATADIR,"eng",tesseract::OEM_TESSERACT_CUBE_COMBINED))
	{
		cerr << "OCRTess:could not initialize teseract" << endl;
		return;
	}
	tessr.SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_CHAR);
	tessr.SetVariable("tessedit_char_whitelist","0123456789ABCDEFGHIGKLMNOPQRSTUVWXYZ-_");
}

void systemInit()
{
	roiOcrEngineInit();
}

void systemEnd()
{
	tessr.End();
}

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

Page::~Page()
{
	tess->End();
}

int Page::ocrEngineInit()
{
	//cout << blur_w;
	if (tess->Init(TESSDATADIR,"eng_arial",tesseract::OEM_TESSERACT_CUBE_COMBINED))//OEM_CUBE_ONLY;OEM_TESSERACT_LSTM_COMBINED;OEM_LSTM_ONLY;OEM_TESSERACT_CUBE_COMBINED
	{
		cerr << "OCRTess:could not initialize teseract" << endl;
		return -1;
	}
	tess->SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_WORD);
	//cout<<"white "<<tess->SetVariable("tessedit_char_whitelist","0123456789");
	tess->SetVariable("tessedit_char_whitelist","0123456789ABCDEFHIGKLMNOPQRSTUVWXYZ-_");
	//if (tess.Init("e:\\resources\\tesseract\\tessdata","eng",tesseract::OEM_LSTM_ONLY))
	//{
	//	cerr << "OCRTess:could not initialize teseract" << endl;
	//	return -1;
	//}
	//tess.SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_LINE);
	//cout<<"white "<<tess.SetVariable("tessedit_char_whitelist",
	//	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	//cout<<" black "<<tess.SetVariable("tessedit_char_blacklist","!?@#$%&*()<>_-+=/:;'\"");
	return 0;
}
