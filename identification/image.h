#pragma once
#include "stdafx.h"
#include "public.h"

//公共
class image_common 
{
private:

public:

	//归一化图像
	cv::Mat norm_0_255(const Mat& src) {
		// Create and return normalized image:
		cv::Mat dst;
		switch (src.channels()) {
		case 1:
			cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
			break;
		case 3:
			cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
			break;
		default:
			src.copyTo(dst);
			break;
		}
		return dst;
	};
};

//图像对比类
class image_contrast:public image_common
{
private:

public:
	//图像对比度增强
	void contrast_enhancement(cv::Mat image, double gamma, pre_ret &image_obj) {
		//2.转换图像类型为浮点数
		cv::Mat I, X;
		image.convertTo(X, CV_32FC1);
		pow(X, gamma, I);
		cv::Mat result_image = norm_0_255(I);	
		//将结果写入
		if (!result_image.empty()) {
			image_obj.ret = 0;
			image_obj.rect_image = result_image;
		}
		else {
			image_obj.ret = -1;
		}
	}

};