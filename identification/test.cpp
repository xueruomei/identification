#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include "colordetector.h"
using namespace std;
using namespace cv;

int mains222(int, char *argv[])
{
	//11.实例化视频对象
	cv::VideoCapture capture;
	//6.读取人脸视频
	//8.读取视频每一帧图像去侦测人脸
	capture.open("plate_reco/plate.mp4");
	if (!capture.isOpened())
	{
		std::cout << "video open error" << std::endl;
		return -1;
	}
	while (true)
	{
		cv::Mat frame, gray;
		capture >> frame;
		if (frame.empty()) {
			break;
		}
		else {
			cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
			cv::imshow("读取视频", frame);
			//灰度变换
			//cv::Mat dst;
			//////图像增强
			//dst.create(frame.size(), frame.type());

			//for (int row = 0; row < frame.rows; row++)
			//{
			//	for (int col = 0; col < frame.cols; col++)
			//	{
			//		int b = frame.at<Vec3b>(row, col)[0];
			//		int g = frame.at<Vec3b>(row, col)[1];
			//		int r = frame.at<Vec3b>(row, col)[2];
			//		dst.at<Vec3b>(row, col)[0] = min(r, min(g, b));
			//		dst.at<Vec3b>(row, col)[1] = min(r, min(g, b));
			//		dst.at<Vec3b>(row, col)[2] = min(r, min(g, b));
			//	}
			//}
			//伽马变换
			//Mat imageGamma(frame.size(), CV_32FC3);
			//for (int i = 0; i < frame.rows; i++)
			//{
			//	for (int j = 0; j < frame.cols; j++)
			//	{
			//		imageGamma.at<Vec3f>(i, j)[0] = (frame.at<Vec3b>(i, j)[0])*(frame.at<Vec3b>(i, j)[0])*(frame.at<Vec3b>(i, j)[0]);
			//		imageGamma.at<Vec3f>(i, j)[1] = (frame.at<Vec3b>(i, j)[1])*(frame.at<Vec3b>(i, j)[1])*(frame.at<Vec3b>(i, j)[1]);
			//		imageGamma.at<Vec3f>(i, j)[2] = (frame.at<Vec3b>(i, j)[2])*(frame.at<Vec3b>(i, j)[2])*(frame.at<Vec3b>(i, j)[2]);
			//	}
			//}
			////归一化到0~255    
			//normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
			////转换成8bit图像显示    
			//convertScaleAbs(imageGamma, imageGamma);
			//imshow("原图", frame);
			//imshow("伽马变换图像增强效果", imageGamma);

			//Log变换
			cv::Mat image = imread("Test.jpg");
			cv::Mat imageLog(frame.size(), CV_32FC3);
			for (int i = 0; i < frame.rows; i++)
			{
				for (int j = 0; j < frame.cols; j++)
				{
					imageLog.at<Vec3f>(i, j)[0] = log(1 + frame.at<Vec3b>(i, j)[0]);
					imageLog.at<Vec3f>(i, j)[1] = log(1 + frame.at<Vec3b>(i, j)[1]);
					imageLog.at<Vec3f>(i, j)[2] = log(1 + frame.at<Vec3b>(i, j)[2]);
				}
			}
			//归一化到0~255    
			normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
			//转换成8bit图像显示    
			convertScaleAbs(imageLog, imageLog);
			/*
			imshow("Soure", frame);
			imshow("after", imageLog);
			*/
			//拉普拉斯算子
			/*Mat imageEnhance;
			Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
			filter2D(frame, imageEnhance, CV_8UC3, kernel);
			imshow("拉普拉斯算子图像增强效果", imageEnhance);*/
			//将图像转为hsv
			cv::Mat newimage;
			cv::cvtColor(frame, newimage, CV_BGR2HSV);
			//cv::imshow("newimage", newimage);
			ColorDetector cdetect;
			cdetect.setTargetColor(120, 255, 46); // here blue sky

			// 4. Process the image and display the result
			//cv::namedWindow("result");
			cv::Mat result = cdetect.process(newimage);
			//cv::imshow("result", result);
			//高斯模糊
			GaussianBlur(result, result, Size(5, 5), 11); //高斯模糊
			namedWindow("gaussianblur img", CV_WINDOW_AUTOSIZE);
			imshow("gaussianblur img", result);
			//阈值化
			cv::Mat roi_threadhold_image;
			threshold(result, roi_threadhold_image, 100, 255, CV_THRESH_BINARY);
			imshow("threshold", result);
			//预测
			//2.0candy 检测图像（车牌）轮廓
			//检测全图的边缘轮廓
			cv::Mat contour;
			cv::Canny(result, contour, 60, 200);
			//cv::imshow("contour", contour);

			//3.0形态学变换图像
			Point point(-1, -1);
			cv::Mat dilate_image, erode_image;
			//自定义 核进行 x 方向的膨胀腐蚀
			cv::Mat elementX = getStructuringElement(MORPH_RECT, Size(30, 1));
			cv::Mat elementY = getStructuringElement(MORPH_RECT, Size(1, 20));
			cv::dilate(contour, dilate_image, elementX, point, 2);
			cv::erode(dilate_image, erode_image, elementX, point, 4);
			cv::dilate(erode_image, dilate_image, elementX, point, 2);
			//cv::imshow("xxx", dilate_image);
			//自定义 核进行 Y 方向的膨胀腐蚀
			cv::erode(dilate_image, erode_image, elementY, point, 1);
			cv::dilate(erode_image, dilate_image, elementY, point, 2);
			//4.噪声处理
			//平滑处理 中值滤波
			cv::Mat blurr_image;
			cv::medianBlur(dilate_image, blurr_image, 5);
			//cv::imshow("blurr_image", blurr_image);

			
			//矩形轮廓查找与筛选:
			cv::Mat contour_image;
			//深拷贝  查找轮廓会改变源图像信息，需要重新 拷贝 图像
			contour_image = blurr_image.clone();
			vector<vector<Point>> contours;
			findContours(contour_image, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//列出包含水平，垂直或对角线轮廓的端点
			cv::Mat  roi_image;
			for (int i = 0; i < contours.size(); i++) {
				cv::Rect r = boundingRect(cv::Mat(contours[i]));
				//cout << "contours " << i << "  height = " << r.height << "  width = " << r.width << "rate = " << ((float)r.width / r.height) << endl;
				//cout << "(float)r.width / r.height::" << (float)r.width / r.height << endl;
				if ((float)r.width / r.height > 2.2 && (float)r.width / r.height < 3.6) {
					cout << "r.width = " << r.width << "  r.height  = " << r.height << endl;	
					cout << "(float)r.width / r.height::" << (float)r.width / r.height << endl;
					rectangle(contour_image, r, Scalar(0, 0, 255), 2);
					//计算点画框
					rectangle(imageLog, r, cv::Scalar(0, 0, 255), 2);
					cv::imwrite("plate_reco/new_plate.jpg", imageLog);
					roi_image = imageLog(r);
					cv::imshow("侦测车牌", imageLog);
				}
			}
			if (roi_image.empty()) {
				continue;
			}
			/*Mat imageEnhance;
			Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
			filter2D(frame, imageEnhance, CV_8UC3, kernel);
			imshow("拉普拉斯算子图像增强效果", imageEnhance);
			cv::waitKey(10);*/
			//Mat image = imread("Test.jpg");
			cv::waitKey(10);
		}
	}
	capture.release();
	return 0;
}