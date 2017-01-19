#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <memory>//unique_ptr
#include "page.h"
#include "func.h"

//#define TESSDATADIR "D:\\Document\\code\\tesseract\\tessdata"
using namespace std;
using namespace cv;

//tesseract::TessBaseAPI tessr; 

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
