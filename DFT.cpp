// DFT.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include "DFT.h"

using namespace std;
using namespace cv;

void DFT(Mat &src, Mat &dst) {
	copyMakeBorder(src, dst, 0, getOptimalDFTSize(src.rows) - src.rows,
		0, getOptimalDFTSize(src.cols) - src.cols, BORDER_CONSTANT, Scalar::all(0));
	Mat planes[] = { Mat_<float>(dst),Mat::zeros(dst.size(),CV_32F) };
	merge(planes, 2, dst);
	dft(dst, dst);
}
Mat show_spectrum_magnitude(Mat &complexImg, bool shift) {
	Mat planes[2], dst;
	split(complexImg, planes);
	magnitude(planes[0], planes[1], dst);
	dst += Scalar::all(1);
	log(dst, dst);
	if (shift) shiftImg(dst);
	normalize(dst, dst, CV_MINMAX);
	return dst;
}
void shiftImg(Mat &src) {
	int cx = src.cols / 2;
	int cy = src.rows / 2;
	Mat tmp, q0, q1, q2, q3;
	q0 = src(Rect(0, 0, cx, cy));
	q1 = src(Rect(cx, 0, cx, cy));
	q2 = src(Rect(cx, cy, cx, cy));
	q3 = src(Rect(0, cy, cx, cy));
	q0.copyTo(tmp);
	q2.copyTo(q0);
	tmp.copyTo(q2);
	q1.copyTo(tmp);
	q3.copyTo(q1);
	tmp.copyTo(q3);
}