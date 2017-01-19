#include "stdafx.h"
#include "func.h"
#include <baseapi.h>
#include "page.h"

namespace fs = std::experimental::filesystem;
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
//inputImg,rotate_degree
cv::Mat rotateImg(cv::Mat &src, double &angle) {
	if (src.empty()) return src;
	cv::Point2f center(src.cols / 2, src.rows / 2);
	Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	cv::Rect bbox = cv::RotatedRect(center, src.size(), angle).boundingRect();
	//cout << bbox << endl;
	rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;
	Mat dst;
	cv::warpAffine(src, src, rot, bbox.size());
	return src;
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

void help() {
	cout << "Three parameter need to be defined" << endl;
}
void parameterDefine(string &inDir,string &outDir,int &type) {
	string str;
	stringstream ss;
	cout << "Input the file type:" << endl
		<< "Work Sheet(0)*,Application Form(1),Work Sheet Demo(3)" << endl;
	getline(cin,str);
	if (str.empty()) type = 3;
	else istringstream(str) >> type;
	//cout << type << endl;
	if (type>2)
	{
		type = WORKSHEET;
		inDir = DEFAULT_IN_WS_DIR;
		outDir = DEFAULT_OUT_DIR;
		return;
	}
	cout<<"Input the output directory:"<<endl
		<<"default is "<<DEFAULT_OUT_DIR<<endl;
	getline(cin, str);
	//cin >> str;
	if (str.empty()) outDir = DEFAULT_OUT_DIR;
	else outDir = str;
	cout << outDir;

	if (fs::exists(outDir))
		std::experimental::filesystem::remove_all(outDir.c_str());

	cout << "Input the Input directory:" << endl;
	switch (type)
	{
	case WORKSHEET:
		cout << "default is "<<DEFAULT_IN_WS_DIR;
		getline(cin, str);
		//cin >> str;
		if (str.empty()) inDir = DEFAULT_IN_WS_DIR;
		else inDir = str;
		break;
	case FORM:
		cout << "default is "<<DEFAULT_IN_FM_DIR<<endl;
		getline(cin, str);
		//cin >> str;
		if (str.empty()) inDir = DEFAULT_IN_FM_DIR;
		else inDir = str;
		break;
	default:
		break;
	}

	if (!fs::exists(outDir)) {
		try
		{
			fs::create_directory(outDir);
		}
		catch (const std::exception&e)
		{
			cerr << e.what()<< endl;
			return;
		}
	}

}

void systemInit(string &inDir,string &outDir,int &type)
{
	help();
	parameterDefine(inDir,outDir,type);
	roiOcrEngineInit();
}

void systemEnd()
{
	tessr.End();
}

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
