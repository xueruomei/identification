#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include "colordetector.h"
using namespace std;
using namespace cv;

int mains222(int, char *argv[])
{
	//11.ʵ������Ƶ����
	cv::VideoCapture capture;
	//6.��ȡ������Ƶ
	//8.��ȡ��Ƶÿһ֡ͼ��ȥ�������
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
			cv::imshow("��ȡ��Ƶ", frame);
			//�Ҷȱ任
			//cv::Mat dst;
			//////ͼ����ǿ
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
			//٤��任
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
			////��һ����0~255    
			//normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
			////ת����8bitͼ����ʾ    
			//convertScaleAbs(imageGamma, imageGamma);
			//imshow("ԭͼ", frame);
			//imshow("٤��任ͼ����ǿЧ��", imageGamma);

			//Log�任
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
			//��һ����0~255    
			normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
			//ת����8bitͼ����ʾ    
			convertScaleAbs(imageLog, imageLog);
			/*
			imshow("Soure", frame);
			imshow("after", imageLog);
			*/
			//������˹����
			/*Mat imageEnhance;
			Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
			filter2D(frame, imageEnhance, CV_8UC3, kernel);
			imshow("������˹����ͼ����ǿЧ��", imageEnhance);*/
			//��ͼ��תΪhsv
			cv::Mat newimage;
			cv::cvtColor(frame, newimage, CV_BGR2HSV);
			//cv::imshow("newimage", newimage);
			ColorDetector cdetect;
			cdetect.setTargetColor(120, 255, 46); // here blue sky

			// 4. Process the image and display the result
			//cv::namedWindow("result");
			cv::Mat result = cdetect.process(newimage);
			//cv::imshow("result", result);
			//��˹ģ��
			GaussianBlur(result, result, Size(5, 5), 11); //��˹ģ��
			namedWindow("gaussianblur img", CV_WINDOW_AUTOSIZE);
			imshow("gaussianblur img", result);
			//��ֵ��
			cv::Mat roi_threadhold_image;
			threshold(result, roi_threadhold_image, 100, 255, CV_THRESH_BINARY);
			imshow("threshold", result);
			//Ԥ��
			//2.0candy ���ͼ�񣨳��ƣ�����
			//���ȫͼ�ı�Ե����
			cv::Mat contour;
			cv::Canny(result, contour, 60, 200);
			//cv::imshow("contour", contour);

			//3.0��̬ѧ�任ͼ��
			Point point(-1, -1);
			cv::Mat dilate_image, erode_image;
			//�Զ��� �˽��� x ��������͸�ʴ
			cv::Mat elementX = getStructuringElement(MORPH_RECT, Size(30, 1));
			cv::Mat elementY = getStructuringElement(MORPH_RECT, Size(1, 20));
			cv::dilate(contour, dilate_image, elementX, point, 2);
			cv::erode(dilate_image, erode_image, elementX, point, 4);
			cv::dilate(erode_image, dilate_image, elementX, point, 2);
			//cv::imshow("xxx", dilate_image);
			//�Զ��� �˽��� Y ��������͸�ʴ
			cv::erode(dilate_image, erode_image, elementY, point, 1);
			cv::dilate(erode_image, dilate_image, elementY, point, 2);
			//4.��������
			//ƽ������ ��ֵ�˲�
			cv::Mat blurr_image;
			cv::medianBlur(dilate_image, blurr_image, 5);
			//cv::imshow("blurr_image", blurr_image);

			
			//��������������ɸѡ:
			cv::Mat contour_image;
			//���  ����������ı�Դͼ����Ϣ����Ҫ���� ���� ͼ��
			contour_image = blurr_image.clone();
			vector<vector<Point>> contours;
			findContours(contour_image, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//�г�����ˮƽ����ֱ��Խ��������Ķ˵�
			cv::Mat  roi_image;
			for (int i = 0; i < contours.size(); i++) {
				cv::Rect r = boundingRect(cv::Mat(contours[i]));
				//cout << "contours " << i << "  height = " << r.height << "  width = " << r.width << "rate = " << ((float)r.width / r.height) << endl;
				//cout << "(float)r.width / r.height::" << (float)r.width / r.height << endl;
				if ((float)r.width / r.height > 2.2 && (float)r.width / r.height < 3.6) {
					cout << "r.width = " << r.width << "  r.height  = " << r.height << endl;	
					cout << "(float)r.width / r.height::" << (float)r.width / r.height << endl;
					rectangle(contour_image, r, Scalar(0, 0, 255), 2);
					//����㻭��
					rectangle(imageLog, r, cv::Scalar(0, 0, 255), 2);
					cv::imwrite("plate_reco/new_plate.jpg", imageLog);
					roi_image = imageLog(r);
					cv::imshow("��⳵��", imageLog);
				}
			}
			if (roi_image.empty()) {
				continue;
			}
			/*Mat imageEnhance;
			Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
			filter2D(frame, imageEnhance, CV_8UC3, kernel);
			imshow("������˹����ͼ����ǿЧ��", imageEnhance);
			cv::waitKey(10);*/
			//Mat image = imread("Test.jpg");
			cv::waitKey(10);
		}
	}
	capture.release();
	return 0;
}