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
		"��","��","��","��","��","��"
	};
	Ptr<SVM> svm = SVM::create();
	//load train file
	svm = SVM::load("plate_reco/PLATE_TRAIN.xml");
	//��ⴰ��(32, 40),��ߴ�(16,16),�鲽��(8,8),cell�ߴ�(8,8),ֱ��ͼbin����9
	HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);

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
			//cv::imshow("��ȡ��Ƶ", frame);
			cv::Mat dst;
			////ͼ����ǿ
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
			//Ԥ��
			//2.0candy ���ͼ�񣨳��ƣ�����
			//���ȫͼ�ı�Ե����
			cv::Mat contour;
			cv::Canny(dst, contour, 125, 350);
			cv::imshow("contour", contour);

			//3.0��̬ѧ�任ͼ��
			Point point(-1, -1);
			cv::Mat dilate_image, erode_image;
			//�Զ��� �˽��� x ��������͸�ʴ
			cv::Mat elementX = getStructuringElement(MORPH_RECT, Size(20, 2));
			cv::Mat elementY = getStructuringElement(MORPH_RECT, Size(2, 15));

			cv::dilate(contour, dilate_image, elementX, point, 2);
			cv::erode(dilate_image, erode_image, elementX, point, 4);
			cv::dilate(erode_image, dilate_image, elementX, point, 2);
			//cv::imshow("xxx", dilate_image);
			//�Զ��� �˽��� Y ��������͸�ʴ
			cv::erode(dilate_image, erode_image, elementY, point, 1);
			cv::dilate(erode_image, dilate_image, elementY, point, 2);
			cv::imshow("dilate_image", dilate_image);
			//ƽ������ ��ֵ�˲�
			cv::Mat blurr_image;
			cv::medianBlur(dilate_image, blurr_image, 3);
			//��������������ɸѡ:
			cv::Mat contour_image;
			//���  ����������ı�Դͼ����Ϣ����Ҫ���� ���� ͼ��
			contour_image = blurr_image.clone();
			vector<vector<Point>> contours;
			findContours(contour_image, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//�г�����ˮƽ����ֱ��Խ��������Ķ˵�
			//6.������ʾΪһ�����γ�����ȡ
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
					//����� ����
					rectangle(dst, r, cv::Scalar(0, 0, 255), 2);
					cv::imwrite("plate_reco/new_plate.jpg", dst);
					roi_image = dst(r);
					cv::imshow("��⳵��", dst);
				}
			};
			if (roi_image.empty()) {
				continue;
			}
			//7.��⳵�Ʋ��ֵı�Ե
			//8.��ֵ�����Ʋ��� ��ֵ��
			cv::Mat roi_threadhold_image;
			threshold(roi_image, roi_threadhold_image, 128, 230, CV_THRESH_BINARY);

			//9.��ȡ���Ƶ�����
			//10.�ָ�
			if (!roi_threadhold_image.empty()) {
				//д����ֵ���ĳ���ͼ��
				cv::imwrite("plate_reco/threadhold_plate.jpg", roi_threadhold_image);
				//һ�е���������
				int sum_px = roi_threadhold_image.cols;
				if (roi_threadhold_image.rows > 20) {
					//����7�ȷֵ�����ֵ
					int per_px = sum_px / 7 - 1.5;
					string lensnce_plate = "";
					for (int i = 0; i < 7; i++) {
						int d = i*per_px;
						//������ //���Ͻǵ㣬���½ǵ�
						cv::Mat ROI;
						rectangle(roi_threadhold_image, Rect(d, 2, per_px, 1.95*per_px), Scalar(128, 128, 128), 1, 8, 0);
						ROI = roi_threadhold_image(Rect(d, 2, per_px, 1.95*per_px));
						char buffer[256];//ת����string
						sprintf_s(buffer, "plate_reco/plate_%d.jpg", i);
						cv::imwrite(buffer, ROI);
						cv::Mat images = imread(buffer);
						cv::Mat newimages;
						cv::imshow("ROI", roi_threadhold_image);
						resize(images, newimages, Size(32, 40), 0, 0, INTER_LINEAR);
						cv::imwrite(buffer, newimages);
						vector<float> descriptors;//HOG����������
						hog.compute(newimages, descriptors, Size(8, 8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
						int lable = svm->predict(descriptors);   //��7��ͼ�����Ԥ��
						lensnce_plate += plate_result[lable];
					}
					//�ڼ������˭���ܴ���
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