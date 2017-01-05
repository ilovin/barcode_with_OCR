#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include "bar.h"

//#define STR(s) #s

using namespace std;
using namespace cv;

int main()
{
	string fileName = "img/ws/10degree.jpg";
	Mat src_color = imread(fileName, 1);
	Mat src;
	if (src_color.empty())
	{
		cerr << "cannot open the IMG" << endl;
		cin.get();
		return -1;
	}
	WorkSheet ws(src_color);
	ws.ocrEngineInit();
	ws.drewCompute();
	ws.getBarCodePositon();
	ws.findOcrRects();
	//ws.drawRotated();
	ws.drawRects();
	ws.putOcrText();
	//ws.dftAllRect();
	
	//imshow("class", ws.getSrc_color());
	//cvtColor(src_color, src, CV_BGR2GRAY);
	//normalize(src,src,0,255,NORM_MINMAX);  

	//while (src.rows>900||src.cols>1600)
	//{
	//	resize(src, src, Size(src.cols * 2 / 3, src.rows * 2 / 3));
	//}
	//vector<Rect> possibleRect;

	//decode(src, possibleRect);
	//draw RotateBox
	//imshow("gray", ws.getGrayImg());
	imshow("color_res", ws.getProcessedImg());
	waitKey(0);
    return 0;
}