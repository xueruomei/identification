#include "stdafx.h"
#include <string>
#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace cv;

#ifdef DEBUG
#pragma comment(lib,"opencv_world343d.lib")
#endif // RELEASE
#pragma comment(lib,"opencv_world343.lib")

#define V_PROJECT 1
#define H_PROJECT 2

#define CV_8UC1 CV_MAKETYPE(CV_8U,1)
typedef struct
{
	int begin;
	int end;

}char_range_t;

void draw_projection(vector<int>& pos, int mode)
{
	vector<int>::iterator max = std::max_element(std::begin(pos), std::end(pos)); //求最大值
	if (mode == H_PROJECT)
	{
		int height = pos.size();
		int width = *max;
		Mat project = Mat::zeros(height, width, CV_8UC1);
		for (int i = 0; i < project.rows; i++)
		{
			for (int j = 0; j < pos[i]; j++)
			{
				project.at<uchar>(i, j) = 255;
			}
		}
		cvNamedWindow("2horizational projection", 0);
		imshow("2horizational projection", project);

	}
	else if (mode == V_PROJECT)
	{
		int height = *max;
		int width = pos.size();
		Mat project = Mat::zeros(height, width, CV_8UC1);
		for (int i = 0; i < project.cols; i++)
		{
			for (int j = project.rows - 1; j >= project.rows - pos[i]; j--)
			{
				//std::cout << "j:" << j << "i:" << i << std::endl;
				project.at<uchar>(j, i) = 255;
			}
		}
		imshow("5vertical projection", project);
	}

	//waitKey();
}

//获取文本的投影用于分割字符(垂直，水平)
int GetTextProjection(Mat &src, vector<int>& pos, int mode)
{
	if (mode == V_PROJECT)
	{
		for (int i = 0; i < src.rows; i++)
		{
			uchar* p = src.ptr<uchar>(i);
			for (int j = 0; j < src.cols; j++)
			{
				if (p[j] == 0)
				{
					pos[j]++;
				}
			}
		}

		draw_projection(pos, V_PROJECT);
	}
	else if (mode == H_PROJECT)
	{
		for (int i = 0; i < src.cols; i++)
		{

			for (int j = 0; j < src.rows; j++)
			{
				if (src.at<uchar>(j, i) == 0)
				{
					pos[j]++;
				}
			}
		}
		draw_projection(pos, H_PROJECT);

	}

	return 0;
}

//获取每个分割字符的范围，min_thresh：波峰的最小幅度，min_range：两个波峰的最小间隔
int GetPeekRange(vector<int> &vertical_pos, vector<char_range_t> &peek_range, int min_thresh = 1, int min_range = 4)
{
	int begin = 0;
	int end = 0;
	for (int i = 0; i < vertical_pos.size(); i++)
	{

		if (vertical_pos[i] > min_thresh && begin == 0)
		{
			begin = i;
		}
		else if (vertical_pos[i] > min_thresh && begin != 0)
		{
			continue;
		}
		else if (vertical_pos[i] < min_thresh && begin != 0)
		{
			end = i;
			if (end - begin >= min_range)
			{
				char_range_t tmp;
				tmp.begin = begin;
				tmp.end = end;
				peek_range.push_back(tmp);
				begin = 0;
				end = 0;
			}

		}
		else if (vertical_pos[i] < min_thresh || begin == 0)
		{
			continue;
		}
		else
		{
			//printf("raise error!\n");
		}
	}

	return 0;
}




inline void save_cut(const Mat& img, int id)
{
	char name[128] = { 0 };
	sprintf_s(name, "%d.jpg", id);
	imwrite(name, img);
}

//切割字符
int CutChar(Mat &img, const vector<char_range_t>& v_peek_range, const vector<char_range_t>& h_peek_range, vector<Mat>& chars_set)
{
	static int count = 0;
	int norm_width = img.rows;  //因为汉字都是类正方形的，所以我们定了norm_width就是汉字的高度
	Mat show_img = img.clone();
	cvtColor(show_img, show_img, CV_GRAY2BGR);
	for (int i = 0; i < v_peek_range.size(); i++)
	{
		int char_gap = v_peek_range[i].end - v_peek_range[i].begin;
		//if (char_gap <= (int)(norm_width*1.2) && char_gap >= (int)(norm_width*0.8))
		{
			int x = v_peek_range[i].begin - 2>0 ? v_peek_range[i].begin - 2 : 0;
			int width = char_gap + 4 <= img.rows ? char_gap : img.rows;
			Rect r(x, 0, width, img.rows);
			rectangle(show_img, r, Scalar(255, 0, 0), 1);
			Mat single_char = img(r).clone();
			chars_set.push_back(single_char);
			save_cut(single_char, count);
			count++;
		}
	}

	imshow("6cut", show_img);
	waitKey();

	return 0;
}

Mat cut_one_line(const Mat& src, int begin, int end)
{
	Mat line = src(Rect(0, begin, src.cols, end - begin)).clone();
	return line;
}


vector<Mat> CutSingleChar(Mat& img)
{
	Mat show = img.clone();
	cvtColor(show, show, CV_GRAY2BGR);
	threshold(img, img, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	imshow("3binary", img);
	vector<int> horizion_pos(img.rows, 0);
	vector<char_range_t> h_peek_range;
	GetTextProjection(img, horizion_pos, H_PROJECT);
	GetPeekRange(horizion_pos, h_peek_range, 2, 10);

#if 1

	/*将每一文本行切割*/
	vector<Mat> lines_set;
	//vector<Mat> lines_set_show;
	for (int i = 0; i < h_peek_range.size(); i++)
	{
		Mat line = cut_one_line(img, h_peek_range[i].begin, h_peek_range[i].end);
		lines_set.push_back(line);
		//Mat line_show = show(Rect(0, h_peek_range[i].begin, show.cols, h_peek_range[i].end - h_peek_range[i].begin));
		//lines_set_show.push_back(line_show);
	}
	//cv::imshow("lines_set", lines_set);
	vector<Mat> chars_set;
	for (int i = 0; i < lines_set.size(); i++)
	{
		Mat line = lines_set[i];
		//Mat line2 = lines_set_show[i];
		imshow("4raw line", line);
		vector<int> vertical_pos(line.cols, 0);
		vector<char_range_t> v_peek_range;
		GetTextProjection(line, vertical_pos, V_PROJECT);
		GetPeekRange(vertical_pos, v_peek_range);
		CutChar(line, v_peek_range, h_peek_range, chars_set);
		//CutChar(line2, v_peek_range, h_peek_range, chars_set);
	}
#endif

	//imshow("line_show", show);
	//imwrite("show.png", show);
	return chars_set;
}


int mainpcr()
{
	Mat img = imread("roi_image.jpg", 0);
	imshow("1src", img);
	resize(img, img, Size(), 2, 2, INTER_LANCZOS4);
	vector<Mat> chars_set = CutSingleChar(img);

	for (int i = 0; i < chars_set.size(); i++)
	{
		/*字符识别*/
	}

	waitKey();
	return 0;
}


struct mat_rect
{
	//cv::Mat mat;
	cv::Rect rect;
};
bool funCmpByValue(const mat_rect& a, const mat_rect& b)
{
	return a.rect.x < b.rect.x;
}

cv::Mat OCR_splitnew(cv::Mat image, std::vector<mat_rect> &sormatrect) {
	std::vector<cv::Mat> _Images;
	Mat img_threshold;//阈值化
	threshold(image, img_threshold, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	vector<vector<Point>> cons;
	//从二值图像检索轮廓
	findContours(img_threshold, cons, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	Mat result;
	img_threshold.copyTo(result);
	cvtColor(result, result, CV_GRAY2RGB);
	vector<vector<Point>>::iterator itc = cons.begin();
	while (itc != cons.end())
	{
		Rect mr = boundingRect(Mat(*(itc)));
		if (mr.width / mr.height < 1 && mr.width > 5 && mr.width <= 25 && mr.width * mr.height >15) {
			//cout << "mr1::" << mr << endl;
			mat_rect mat_obj;
			mat_obj.rect = mr;
			sormatrect.push_back(mat_obj);
		}
		itc++;
	}
	std::sort(sormatrect.begin(), sormatrect.end(), funCmpByValue);

	return result;
};

cout << "sormatrect.size():" << sormatrect.size() << endl;
string lensnce_plate = "";
for (int i = 0; i < sormatrect.size(); i++) {
	if (i == 0) {
		float width = sormatrect[0].rect.width;
		float height = sormatrect[0].rect.height;
		if (width / height < 0.8) {

			int x = -0.8 * height + sormatrect[0].rect.x + width;

			sormatrect[0].rect = cv::Rect(x, sormatrect[0].rect.y, 0.8*height, height);
		}
	}
	cout << sormatrect[i].rect << endl;
	rectangle(ous, sormatrect[i].rect, Scalar(0, 255, 0));
	cv::Mat outs;
	cv::Mat ROI = ous(Rect(sormatrect[i].rect));
	resize(ROI, outs, Size(32, 40), 0, 0, INTER_LINEAR);
	char buffer[256];

	sprintf_s(buffer, "%d.jpg", i);
	//imwrite(buffer, sormatrect[i].mat);

	vector<float> descriptors;//HOG描述子向量
	hog.compute(outs, descriptors, Size(8, 8));//计算HOG描述子，检测窗口移动步长(8,8)
	int lable = svm->predict(descriptors);//对7个图像进行预测
	lensnce_plate += plate_result[lable];
}
cout << "lensnce_plate::" << lensnce_plate << endl;
cv::imshow("sss", ous);




//int main()
//	{
//		Mat src;
//		src = imread("train/test/roi_image.jpg");
//		if (src.empty()) {
//
//			printf("imread imgae error");
//			return -1;
//		}
//		namedWindow("input", CV_WINDOW_AUTOSIZE);
//		imshow("input", src);
//
//
//		//cvtColor(src, src, CV_BGR2GRAY);//把src转为灰度图像
//
//		int height = src.rows;
//		int width = src.cols;
//		int channels = src.channels();
//		double alpha = 1.2;
//		double bata = 30;
//
//		Mat dst;
//		dst = Mat::zeros(src.size(), src.type());
//		//src.convertTo(dst, CV_32F);
//		printf("height=%d width=%d channels=%d", height, width, channels);
//		for (int row = 0; row < height; row++) {
//
//			for (int col = 0; col < width; col++) {
//
//				if (channels == 1) {
//
//					dst.at<uchar>(row, col) = saturate_cast<uchar>(alpha * src.at<uchar>(row, col) + bata);
//				}
//				if (channels == 3) {
//
//					dst.at<Vec3b>(row, col)[0] = saturate_cast<uchar>(alpha * src.at<Vec3b>(row, col)[0] + bata);
//					dst.at<Vec3b>(row, col)[1] = saturate_cast<uchar>(alpha * src.at<Vec3b>(row, col)[1] + bata);
//					dst.at<Vec3b>(row, col)[2] = saturate_cast<uchar>(alpha * src.at<Vec3b>(row, col)[2] + bata);
//				}
//			}
//		}
//		namedWindow("output", CV_WINDOW_AUTOSIZE);
//		imshow("output", dst);
//		imwrite("train/test/new.jpg", dst);
//		waitKey(0);
//		return 0;
//}

/*
2019 4.30
封装
*/
//vector<pair<Mat, Rect>> split(cv::Mat input){
//	vector<pair<Mat, Rect>> output;
//	//Mat input = imread("train/test/roi_image.jpg", 0);
//	Mat img_threshold;
//	threshold(input, img_threshold, 60, 255, CV_THRESH_BINARY_INV);
//	Mat img_contours;
//	img_threshold.copyTo(img_contours);
//	vector<vector<Point>> contours;
//	findContours(img_contours, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
//	Mat result;
//	img_threshold.copyTo(result);
//	cvtColor(result, result, CV_GRAY2RGB);
//	drawContours(result, contours, 6, Scalar(255, 0, 0), 1);
//	vector<vector<Point>>::iterator itc = contours.begin();
//
//	while (itc != contours.end())
//	{
//		Rect mr = boundingRect(Mat(*(itc)));
//		rectangle(result, mr, Scalar(0, 255, 0));
//		Mat auxRoi = img_threshold(mr);
//		if (verifySizes(auxRoi))
//		{
//			auxRoi = preprocessChar(auxRoi);
//			output.push_back(make_pair(auxRoi, mr));
//			rectangle(result, mr, Scalar(0, 125, 255));
//		}
//		itc++;
//	}
//	return output;
//}


std::vector<cv::Mat> OCR_split(cv::Mat image) {
	std::vector<cv::Mat> _Images;
	Mat img_threshold;//阈值化
	threshold(image, img_threshold, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	vector<vector<Point>> cons;
	//从二值图像检索轮廓
	findContours(img_threshold, cons, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	Mat result;
	img_threshold.copyTo(result);
	cvtColor(result, result, CV_GRAY2RGB);
	vector<vector<Point>>::iterator itc = cons.begin();
	while (itc != cons.end())
	{
		Rect mr = boundingRect(Mat(*(itc)));
		if (mr.width / mr.height < 1 && mr.width > 5 && mr.width <= 25 && mr.width * mr.height >15) {
			cout << "mr::" << mr << endl;
			rectangle(result, mr, Scalar(0, 255, 0));
			cv::Mat ROI = img_threshold(Rect(mr));
			cv::Mat newimages;
			resize(ROI, newimages, Size(32, 40), 0, 0, INTER_LINEAR);
			_Images.push_back(newimages);
			/*char buffcon[256];
			sprintf_s(buffcon, "plate_reco/plate_%d.jpg", ret);
			cv::imwrite(buffcon, newimages);*/

		}
		itc++;
	}
	//}
	cv::imshow("split", result);
	return _Images;
};

//std::vector<cv::Mat> split = OCR_split(image);//切割

//cout << split.size() << endl;
////cv::imshow("split", split);
//string lensnce_plate = "";
//for (int i = split.size() - 1; i >= 0; i--) {
//	vector<float> descriptors;//HOG描述子向量
//	hog.compute(split[i], descriptors, Size(8, 8));//计算HOG描述子，检测窗口移动步长(8,8)
//	int lable = svm->predict(descriptors);//对7个图像进行预测
//	lensnce_plate += plate_result[lable];
//}
//cout << "lensnce_plate::" << lensnce_plate << endl;