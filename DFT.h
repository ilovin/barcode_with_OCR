#include <opencv2\opencv.hpp>

using namespace cv;

void DFT(Mat &src, Mat &dst);
Mat show_spectrum_magnitude(Mat &complexImg, bool shift);
void shiftImg(Mat &src);
