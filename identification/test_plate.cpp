#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include <opencv2/ml/ml.hpp>
using namespace std;
using namespace cv;
using namespace cv::ml;

int mainaaa(int, char *argv[])
{
	string plate_result[40] = {
		"0","1","2","3","4","5","6","7","8","9",
		"A","B","C","D","E","F","G","H","J","K","L","M","N","P",
		"Q","R","S","T","U","V","W","X","Y","Z",
		"京","沪","浙","粤","苏","闽"
	};
	Ptr<SVM> svm = SVM::create();
	//load train file
	svm = SVM::load("plate_reco/PLATE_TRAIN.xml");
	//检测窗口(32, 40),块尺寸(16,16),块步长(8,8),cell尺寸(8,8),直方图bin个数9
	HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);

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
			//cv::imshow("读取视频", frame);
			cv::Mat dst;
			////图像增强
			dst.create(frame.size(), frame.type());

			for (int row = 0; row < frame.rows; row++)
			{
				for (int col = 0; col < frame.cols; col++)
				{
					int b = frame.at<Vec3b>(row, col)[0];
					int g = frame.at<Vec3b>(row, col)[1];
					int r = frame.at<Vec3b>(row, col)[2];
					dst.at<Vec3b>(row, col)[0] = min(r, min(g, b));
					dst.at<Vec3b>(row, col)[1] = min(r, min(g, b));
					dst.at<Vec3b>(row, col)[2] = min(r, min(g, b));
				}
			}
			//预测
			//2.0candy 检测图像（车牌）轮廓
			//检测全图的边缘轮廓
			cv::Mat contour;
			cv::Canny(dst, contour, 125, 350);
			cv::imshow("contour", contour);

			//3.0形态学变换图像
			Point point(-1, -1);
			cv::Mat dilate_image, erode_image;
			//自定义 核进行 x 方向的膨胀腐蚀
			cv::Mat elementX = getStructuringElement(MORPH_RECT, Size(20, 2));
			cv::Mat elementY = getStructuringElement(MORPH_RECT, Size(2, 15));

			cv::dilate(contour, dilate_image, elementX, point, 2);
			cv::erode(dilate_image, erode_image, elementX, point, 4);
			cv::dilate(erode_image, dilate_image, elementX, point, 2);
			//cv::imshow("xxx", dilate_image);
			//自定义 核进行 Y 方向的膨胀腐蚀
			cv::erode(dilate_image, erode_image, elementY, point, 1);
			cv::dilate(erode_image, dilate_image, elementY, point, 2);
			cv::imshow("dilate_image", dilate_image);
			//平滑处理 中值滤波
			cv::Mat blurr_image;
			cv::medianBlur(dilate_image, blurr_image, 3);
			//矩形轮廓查找与筛选:
			cv::Mat contour_image;
			//深拷贝  查找轮廓会改变源图像信息，需要重新 拷贝 图像
			contour_image = blurr_image.clone();
			vector<vector<Point>> contours;
			findContours(contour_image, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//列出包含水平，垂直或对角线轮廓的端点
			//6.轮廓表示为一个矩形车牌提取
			cv::Mat  roi_image;
			for (int i = 0; i < contours.size(); i++) {
				cv::Rect r = boundingRect(cv::Mat(contours[i]));
				//cout << "contours " << i << "  height = " << r.height << "  width = " << r.width << "rate = " << ((float)r.width / r.height) << endl;
				if ((float)r.width / r.height >= 2.2 && (float)r.width / r.height <= 3.6) {
					//if ((float)r.width / r.height >= 3.1 && (float)r.width / r.height <= 3.3) {
					cout << "r.width = " << r.width << "  r.height  = " << r.height << endl;
					//cout << "r.x = " << r.x << "  r.y  = " << r.y << endl;
					cout << "(float)r.width / r.height::" << (float)r.width / r.height << endl;
					rectangle(contour_image, r, Scalar(0, 0, 255), 2);
					//计算点 画框
					rectangle(dst, r, cv::Scalar(0, 0, 255), 2);
					cv::imwrite("plate_reco/new_plate.jpg", dst);
					roi_image = dst(r);
					cv::imshow("侦测车牌", dst);
				}
			};
			if (roi_image.empty()) {
				continue;
			}
			//7.检测车牌部分的边缘
			//8.阈值化车牌部分 二值化
			cv::Mat roi_threadhold_image;
			threshold(roi_image, roi_threadhold_image, 128, 230, CV_THRESH_BINARY);

			//9.提取车牌的轮廓
			//10.分割
			if (!roi_threadhold_image.empty()) {
				//写入阈值化的车牌图像
				cv::imwrite("plate_reco/threadhold_plate.jpg", roi_threadhold_image);
				//一行的总像素数
				int sum_px = roi_threadhold_image.cols;
				if (roi_threadhold_image.rows > 20) {
					//均分7等分的像素值
					int per_px = sum_px / 7 - 1.5;
					string lensnce_plate = "";
					for (int i = 0; i < 7; i++) {
						int d = i*per_px;
						//画矩形 //左上角点，右下角点
						cv::Mat ROI;
						rectangle(roi_threadhold_image, Rect(d, 2, per_px, 1.95*per_px), Scalar(128, 128, 128), 1, 8, 0);
						ROI = roi_threadhold_image(Rect(d, 2, per_px, 1.95*per_px));
						char buffer[256];//转化成string
						sprintf_s(buffer, "plate_reco/plate_%d.jpg", i);
						cv::imwrite(buffer, ROI);
						cv::Mat images = imread(buffer);
						cv::Mat newimages;
						cv::imshow("ROI", roi_threadhold_image);
						resize(images, newimages, Size(32, 40), 0, 0, INTER_LINEAR);
						cv::imwrite(buffer, newimages);
						vector<float> descriptors;//HOG描述子向量
						hog.compute(newimages, descriptors, Size(8, 8));//计算HOG描述子，检测窗口移动步长(8,8)
						int lable = svm->predict(descriptors);   //对7个图像进行预测
						lensnce_plate += plate_result[lable];
					}
					//第几秒出现谁及总次数
					cout << " Image name=" << lensnce_plate << endl;
					cv::waitKey(20);
				}
			}
			cv::waitKey(10);
		}
	}
	capture.release();
	return 0;

}