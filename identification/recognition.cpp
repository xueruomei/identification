// videocapture.cpp : 定义控制台应用程序的入口点。
/*
author:薛若梅
starttime：2019-3-28
content：读取视频文件，侦测每帧人脸（车牌） 识别每帧人脸（车牌）
*/
#include "stdafx.h"
#include <string>
#include <vector>
#include <iostream>
#include <math.h>
#include <algorithm>
#include "time.h"
#include "jsonutil.h"
#include "public.h"
#include "recognition.h" //所有识别的入口
#include "common.h"
using namespace std;
using namespace cv;
using namespace cv::ml;


/*
params: [1] = 1 
params：[2] =薛若梅   识别谁
params：[3] 路径
*/

/*
	1.人相关
*/
static string buf() {
	char buffer[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, &buffer[0]);//获取当前磁盘路径
	for (int i = 0; i < MAX_PATH; i++) {
		if (buffer[i] == '\\') {
			buffer[i] = '/';
		}
	}
	return buffer;
}

//处理磁盘路径字符
static string handle_disk_path(string handle_buf) {
	for (int i = 0; i < handle_buf.size(); i++) {
		if (handle_buf[i] == '\\') {
			handle_buf[i] = '/';
		}
	}
	return handle_buf;
 }

//返回协议路径
//string handle_http_path(string file_name) {
//	string http_path = "http://192.168.1.239:5200/" + file_name;
//
//	return http_path;
//}

//1.1人脸公共识别
void wrapper::func_Face_common_Predict(cv::Mat origin, pre_ret &face_obj)
{
	face_obj = {};//清空
	cv::Mat gray;//灰度
	cvtColor(origin, gray, cv::COLOR_BGR2GRAY);
	//1.侦测人脸
	std::vector<detect_rect> detectRect; //侦测人脸的向量
	int detect_status = face_api.detect(gray, detectRect);
	if (detect_status == 0) {
		//侦测到人脸
		//1.裁剪人脸去识别
		//灰度图裁剪
		cv::Mat rect_image = gray(cv::Rect(detectRect[0].rect.x, detectRect[0].rect.y, detectRect[0].rect.width, detectRect[0].rect.height));
		//原图裁剪
		cv::Mat rect_cnl3 = origin(cv::Rect(detectRect[0].rect.x, detectRect[0].rect.y, detectRect[0].rect.width, detectRect[0].rect.height));
		//2.识别
		predict_data predict_obj;
		face_api.predict(rect_image, predict_obj);
		if (predict_obj.confidence < 90) {
			if (predict_obj.confidence <= 100) {
				predict_obj.confidence = 100 - predict_obj.confidence;
			}
			string names = face_api.name_Label(predict_obj.predictedLabel);
			face_obj.ret = 0;
			face_obj.name = names;
			face_obj.label = predict_obj.predictedLabel;
			face_obj.confidence = predict_obj.confidence;
			face_obj.rect_image = rect_cnl3;
		}
	}
	else {//未侦测到人脸
		face_obj.ret = -1;
		face_obj.name="";
		face_obj.label = -1;
		face_obj.confidence = 0.0;
	}
}

//1.2人脸侦测
void wrapper::func_Face_Picture_Dectet(const char*path,const char *number, string &_str_client) {
	cv::Mat gray = cv::imread(path,0);//灰度Mat
	cv::Mat origin = cv::imread(path);//原图Mat
	if (gray.empty()) {
		std::cout << "please input file" << endl;
		return ;
	};
	std::vector<detect_rect> detectRect; //侦测人脸的向量
	char buffer[256];
	int status = face_api.detect(gray, detectRect);
	int sum = (int)detectRect.size();//人脸个数
	string json = "{";
	json += yy("sum", sum);//人脸个数
	json += ",";
	if (status == 0) {//有人脸
		for (int i = 0; i < detectRect.size(); i++) {
			cv::rectangle(origin, cv::Rect(cv::Rect(detectRect[i].rect.x, detectRect[i].rect.y, detectRect[i].rect.width, detectRect[i].rect.height)), cv::Scalar(0, 0, 255), 2, 8);
		}
		time_t _time = time(NULL);
		time_t local_time = time(&_time); //获取时间戳
		sprintf_s(buffer, "face_reco/%d.jpg", (int)local_time);
		cv::imwrite(buffer, origin);//写侦测到的人脸
		//cv::imshow("侦测人脸", origin);
		string path_arr[] = {""};
		//输出json
		json += yy("status", 0);
		json += ",";
		json += yy("path",buffer);//画框后的路径
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
		//std::cout << json << endl;
		//cv::waitKey(10);
	}
	else {//无人脸
		//输出json
		json += yy("status", -1);
		json += ",";
		json += yy("path", "");//画框后的路径
		json += ",";
		json += yy("number", number);
		json += "}";
		//std::cout << json << endl;
		_str_client = json;
	}
}

//1.3人脸图片识别
void wrapper::func_Face_Picture_Predict(const char * path, const char *number, pre_ret &face_obj,string &_str_client)
{
	cv::Mat origin = cv::imread(path);//原图
	if (origin.empty()) {
		std::cout << "please input file" << endl;
		return ;
	}
	char buffer[256];//头像路径
	func_Face_common_Predict(origin, face_obj);//调用识别
	if (face_obj.ret == 0) {//有人
		if (face_obj.name != "") {
			time_t _time = time(NULL);
			time_t local_time = time(&_time); //获取时间戳
			sprintf_s(buffer, "face_reco/%d.jpg", (int)local_time);
			//cv::imshow("人脸图片识别", face_obj.rect_image);
			cv::imwrite(buffer, face_obj.rect_image);//写头像
			
			string json = "{";
			json += yy("status", 0);
			json += ",";
			json += yy("name", face_obj.name.c_str());
			//json += ",";
			//json += yy("confidence", face_obj.confidence );
			json += ",";
			json += yy("path", buffer);
			json += ",";
			json += yy("number", number);
			json += "}";
			_str_client = json;
			//std::cout << _str_client << endl;
			//cv::waitKey(10);
		}
		else {
			string json = "{";
			json += yy("status", 0);
			json += ",";
			json += yy("name", face_obj.name.c_str());
			//json += ",";
			//json += yy("confidence", face_obj.confidence );
			json += ",";
			json += yy("path", "");
			json += ",";
			json += yy("number", number);
			json += "}";
			_str_client = json;
		}
	}
	else {//没人
		string json = "{";
		json += yy("status", -1);
		json += ",";
		json += yy("name", "no face");
		json += ",";
		json += yy("path", "");
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
		//std::cout << _str_client << endl;
	}
	cv::waitKey(2);
}

//1.4人脸视频识别
void wrapper::func_Face_Video_Predict(const char * param, const char *number, pre_ret &face_obj, string &_str_client)
{
	//11.实例化视频对象
	cv::VideoCapture capture;
	//6.读取人脸视频
	//8.读取视频每一帧图像去侦测人脸
	capture.open("face_reco/face.mp4");
	//capture.open(path);
	if (!capture.isOpened())
	{
		std::cout << "video open error" << std::endl;
		return ;
	}
	//cout << "总帧数::" << capture.get(CV_CAP_PROP_FRAME_COUNT) << endl;
	//7.读取视频每一帧图像去侦测
	int G_freeframe = 0;
	double starttime = 0.0f;
	double endtime = 0.0f;
	//每秒的帧速率
	char buffer[256];//头像路径
	char buffers[256];//视频路径
	while (1)
	{
		//Mat 矩阵
		cv::Mat frame, face_image;
		capture >> frame;
		if (frame.empty()) {
			break;
		}
		else {
			//cv::imshow("读取视频", frame);
			func_Face_common_Predict(frame, face_obj);//调用识别
			if (face_obj.ret == 0) {//有人
				if (face_obj.name.compare(param) == 0) {//识别结果比对
					G_freeframe++; //该人出现的次数
					if (starttime == 0.0f) {
						starttime = capture.get(0) / 1000.0f;
						//写下裁剪的头像
						time_t _time = time(NULL);
						time_t local_time = time(&_time); //获取时间戳
						sprintf_s(buffer, "face_reco/%d.jpg", (int)local_time);
						cv::imwrite(buffer, face_obj.rect_image);//写头像
						//cv::imshow("读取视频进行人脸识别", face_obj.rect_image);
						cv::waitKey(10);
					};
					endtime = capture.get(0) / 1000.0f;
				}
			}
			else {//没人
				string json = "{";
				json += yy("name", "no face");
				json += ",";
				json += yy("status", -1);
				json += ",";
				json += yy("path","");
				json += ",";
				json += yy("number", number);
				json += "}";
				_str_client = json;
				//std::cout << _str_client << endl;
			}
		}
		//cv::waitKey(2);
	}
	sprintf_s(buffers, "face_reco/face.mp4");
	string s_buf = buffer ;
	s_buf += ",";
	s_buf += buffers;
	//json字符串
	string json = "{";
	json += yy("status", 0);
	json += ",";
	json += yy("sum", G_freeframe);
	json += ",";
	json += yy("name", param);
	json += ",";
	json += yy("starttime", starttime);
	json += ",";
	json += yy("endtime", endtime);
	json += ",";
	json += yy("path", s_buf.c_str());
	json += ",";
	json += yy("number", number);
	json += "}";
	_str_client = json;
	//std::cout << _str_client << endl;
	capture.release();
}

//人脸图片对比
void wrapper::func_Face_Picture_comparison(const char * path, const char *number, pre_ret &face_obj, string &_str_client) {
	//1.分割获取cv::Mat
	vector<string> result;
	face_api.split_str(path,",", result);
	//2.读取第一个和第二个图片
	cv::Mat train_image = imread(result[0], 0);
	
	cv::Mat predict_image = imread(result[1], 0);
	
	/*imshow("train_image", train_image);
	imshow("predict_image", predict_image);
	cvWaitKey(10);*/
	//3.去掉比较函数
	face_api.comparison(train_image, predict_image, face_obj);
	//收到 face_obj 的识别结果 
	//处理json返回给客户端
	if (face_obj.ret == 0 ) {
		if (face_obj.label >= 1) {
			string json = "{";
			json += yy("status", 0);
			json += ",";
			json += yy("name", "是同一个人");
			json += ",";
			json += yy("label", face_obj.label);
			//json += ",";
			//json += yy("confidence", face_obj.confidence);
			json += ",";
			json += yy("path", "");
			json += ",";
			json += yy("number", number);
			json += "}";
			_str_client = json;
			//cout << "_str_client:" << _str_client << endl;
		}
		else if(face_obj.label <=0 ) {
			string json = "{";
			json += yy("status", 0);
			json += ",";
			json += yy("name", "不是同一个人");
			json += ",";
			json += yy("label", face_obj.label);
			//json += ",";
			//json += yy("confidence", 0);
			json += ",";
			json += yy("path", "");
			json += ",";
			json += yy("number", number);
			json += "}";
			_str_client = json;
			//std::cout << "_str_client:" << _str_client << endl;
		}
		//std::cout << "_str_client::" << _str_client << endl;
	}
	else {
		string json = "{";
		json += yy("status", -1);
		json += ",";
		json += yy("name", "image is no face");
		json += ",";
		json += yy("label", 0);
		//json += ",";
		//json += yy("confidence", 0);
		json += ",";
		json += yy("path", "");
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
	}
}

//1.5人脸训练
void wrapper::func_Face_Train(const char* folder, const char *number, string &_str_client) {
	//string folder = "face_reco/FACE";
	face_api.train(folder);
	face_api.write("face_reco/FACE");
	std::cout << "train finished" << endl;
	string json = "{";
	json += yy("status", 0);
	json += ",";
	json += yy("path", "");
	json += ",";
	json += yy("number", number);
	json += "}";
	_str_client = json;
	std::cout << "train::" << _str_client << endl;
}

//1.6人脸更新
void wrapper::func_Face_Update(const char * path, const char * param, const char *number, string &_str_client) {
	int status = face_api.update(path, param, _ori_folder);
	if (status == 0) {
		//更新成功
		string dir = "face_reco/FACE";
		face_api.write(dir);
		std::cout << "train_update finished" << endl;
		string json = "{";
		json += yy("status", 0);
		json += ",";
		json += yy("path", "");
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
		//std::cout << "train::" << _str_client << endl;
	}
	else if(status == -1) {
		//更新失败
		string json = "{";
		json += yy("status", -1);
		json += ",";
		json += yy("path", "");
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
		//std::cout << json << endl;
	}
}


/*
	2.图像相关
*/

void wrapper::func_Image_Contrast(const char * path, const char *number, pre_ret &image_obj, string &_str_client) {
	//1.读取图片
	cv::Mat image = imread(path);
	double gamma = 2.000; //高灰度
	//调用图像对比度增强
	image_contrast_api.contrast_enhancement(image, gamma,image_obj);
	char buffer[256];
	char buffers[256];
	vector<string> result;
	face_api.split_str(path,"/",result);
	//处理json 返回给客户端
	if (image_obj.ret == 0) {//对比增强成功
		//将原图及处理之后的图像写入磁盘
		time_t _time = time(NULL);
		time_t local_time = time(&_time); //获取时间戳
		sprintf_s(buffer, "image/%d.jpg", (int)local_time);

		cv::imwrite(buffer, image_obj.rect_image);//写处理后的图像
		int split_one = result.size()-1;
		int split_two = result.size() - 2;
		string bufs = result[split_two] + "/" + result[split_one];
		string s_buf = buffer;
		s_buf += ",";
		s_buf += bufs;
		//cv::imshow("图像增强", image_obj.rect_image);
		//json字符串
		string json = "{";
		json += yy("status", 0);
		json += ",";
		json += yy("path", s_buf.c_str());//拼上原图路径
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
		//cout << "_str_client:" << _str_client << endl;
	}
	else {//对比增强失败

		string json = "{";
		json += yy("status", -1);
		json += ",";
		json += yy("path", "");
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
	}
}


/*
	3.车相关
*/

//2.4车牌内容训练
void wrapper::func_Plate_Picture_Content_train(const char *number, string &_str_client) {

	plate_content_api.train_content();
	plate_content_api.save();
	string json = "{";
	json += yy("status", 0);
	json += ",";
	json += yy("path", "");
	json += ",";
	json += yy("number", number);
	json += "}";
	_str_client = json;
}

//2.5车牌正负样本训练
void wrapper::func_Plate_Picture_Is_train(const char *number, string &_str_client) {
	plate_lience_api.train_is();
	plate_lience_api.save();
	string json = "{";
	json += yy("status", 0);
	json += ",";
	json += yy("path", "");
	json += ",";
	json += yy("number", number);
	json += "}";
	_str_client = json;
}

//车牌识别公共函数
void wrapper::func_Plate_Common_Predict(cv::Mat image, Ptr<SVM> svm, HOGDescriptor hog, pre_ret &plate_obj) {
	//2.0candy 检测图像（车牌）轮廓
	//检测全图的边缘轮廓
	cv::Mat gray;//灰度图像
	cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	cv::Mat contour;//检测轮廓
	cv::Canny(gray, contour, 125, 350);
	//3.0形态学运算变换图像
	//形态学处理
	//图片膨胀处理
	cv::Mat dilate_image, erode_image;
	//自定义 核进行 x 方向的膨胀腐蚀
	cv::Mat elementX = getStructuringElement(MORPH_RECT, Size(30, 1));
	cv::Mat elementY = getStructuringElement(MORPH_RECT, Size(1, 25));
	Point point(-1, -1);
	cv::dilate(contour, dilate_image, elementX, point, 2);
	cv::erode(dilate_image, erode_image, elementX, point, 4);
	cv::dilate(erode_image, dilate_image, elementX, point, 2);
	//自定义 核进行 Y 方向的膨胀腐蚀
	cv::erode(dilate_image, erode_image, elementY, point, 1);
	cv::dilate(erode_image, dilate_image, elementY, point, 2);
	//4.噪声处理
	//平滑处理 中值滤波
	cv::Mat blurr_image;
	cv::medianBlur(dilate_image, blurr_image, 5);
	vector<vector<Point>> contours;
	//从二值图像检索轮廓
	cv::findContours(blurr_image, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//列出包含水平，垂直或对角线轮廓的端点
	//6.轮廓表示为一个矩形车牌提取
	cv::Mat  roi_image,origin_image;
	for (int i = 0; i < contours.size(); i++) {
		cv::Rect r = boundingRect(cv::Mat(contours[i]));
		if ((float)r.width / r.height >= 2.2 && (float)r.width / r.height <= 3.6) {
			//计算点 画框
			roi_image = gray(r);
			origin_image = image(r);//原图裁剪返回给客户端
			resize(roi_image, roi_image, Size(128, 64), 0, 0, INTER_LINEAR);

			//识别是否是车牌
			float res = plate_lience_api.predict_license(roi_image);
			//去识别
			if (res >= 1.0f) {//是车牌
				cv::Mat image = plate_common::adjust(roi_image);//旋转
			
				std::vector<mat_rect> sormatrect; //存排序的rect
				cv::Mat return_data = plate_content_api.OCR_split(image, sormatrect);//切割
				cv::Mat copy_return_data;
				return_data.copyTo(copy_return_data);//copy 为了裁剪没有边框
				string lensnce_plate = "";
				//修改尺寸
				for (int i = 0; i < sormatrect.size(); i++) {
					rectangle(return_data, sormatrect[i].rect, Scalar(0, 255, 0));
					cv::Mat outs;
					cv::Mat ROI = copy_return_data(Rect(sormatrect[i].rect));
					resize(ROI, outs, Size(32, 40), 0, 0, INTER_LINEAR);
					//开始识别
					vector<float> descriptors;//HOG描述子向量
					hog.compute(outs, descriptors, Size(8, 8));//计算HOG描述子，检测窗口移动步长(8,8)
					float lable = svm->predict(descriptors);//对7个图像进行预测
					lensnce_plate += plate_content_api.label_name((int)lable);
				};
				plate_obj.ret = 0;
				plate_obj.name = lensnce_plate;
				plate_obj.rect_image = origin_image;
			}
			else if (res == -1) {//不是车牌
				plate_obj.ret = -1;
				plate_obj.name = "";
			} //是否是车牌 结束
		}
		
	};
}

//车牌图片识别
void wrapper::func_Plate_Picture_Predict(const char * path, const char * param, const char *number, pre_ret &plate_obj, string &_str_client) {
	Ptr<SVM> svm = SVM::create();
	svm = SVM::load("plate_reco/PLATE_CONTENT.xml");//load train file
	//检测窗口(32, 40),块尺寸(16,16),块步长(8,8),cell尺寸(8,8),直方图bin个数9
	HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
	//cv::Mat origin = cv::imread("plate_reco/1.jpg");//1.0原图像
	cv::Mat origin = cv::imread(path);//1.0原图像
	if (origin.empty()) {
		std::cout << "image.empty()" << endl;
		return ;
	}
	//调用车牌公共函数
	wrapper::func_Plate_Common_Predict(origin, svm, hog, plate_obj);
	char buffer[256];
	if (plate_obj.ret == 0) {//有车
		//标记识别结果大于4
		//int sign_reco = 0;
		//for (int g = 0; g < strlen(plate_obj.name.c_str()); g++) {
		//	//2.循环输入参数
		//	if (plate_obj.name[g] == param[g]) {
		//		sign_reco++;
		//	}
		//}; //循环车牌识别 结束
		//if (sign_reco >= 4) {//识别相对准确
		//		//写下裁剪的车
		//		time_t _time = time(NULL);
		//		time_t local_time = time(&_time); //获取时间戳
		//		sprintf_s(buffer, "plate_reco/%d.jpg", (int)local_time);
		//		cv::imwrite(buffer, plate_obj.rect_image);//写车牌
		//}
		time_t _time = time(NULL);
		time_t local_time = time(&_time); //获取时间戳
		sprintf_s(buffer, "plate_reco/%d.jpg", (int)local_time);
		cv::imwrite(buffer, plate_obj.rect_image);//写车牌
		//cv::imshow("车牌图片识别", plate_obj.rect_image);
		string json = "{";
		json += yy("name", plate_obj.name.c_str());
		json += ",";
		json += yy("path", buffer);
		json += ",";
		json += yy("status", 0);
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
		
	}
	else {//没车
		string json = "{";
		json += yy("name", "no plate");
		json += ",";
		json += yy("status", -1);
		json += ",";
		json += yy("path", "");
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
	}
	//cv::waitKey(10);
}

//车牌视频识别
void wrapper::func_Plate_Video_Predict(const char *path, const char * param, const char *number, pre_ret &plate_obj, string &_str_client) {
	Ptr<SVM> svm = SVM::create();
	//load train file
	svm = SVM::load("plate_reco/PLATE_CONTENT.xml");
	//检测窗口(32, 40),块尺寸(16,16),块步长(8,8),cell尺寸(8,8),直方图bin个数9
	HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
	//11.实例化视频对象
	cv::VideoCapture capture;
	capture.open("plate_reco/plate.mp4");
	//capture.open(path);
	if (!capture.isOpened())
	{
		std::cout << "video open error" << std::endl;
		return ;
	}
	int plate_num = 0;//记录该车牌出现的次数
	double endtime = 0.0f;//结束时间
	double starttime = 0.0f;//开始时间
	char buffer[256];
	char buffers[256];
	string plate_name = "";
	int G_freeframe = 0;//识别到结果的次数
	while (1)
	{
		cv::Mat origin, gray;
		capture >> origin;
		if (origin.empty()) {
			break;
		}
		else {
			//cv::imshow("读取视频", origin);
			//调用车牌公共函数
			wrapper::func_Plate_Common_Predict(origin, svm, hog, plate_obj);
			if (plate_obj.ret == 0) {//有车
				//循环识别结果 循环输入要识别的内容
				//将以上两种结果对比，四个及四个以上 内容相符时 就视为识别出来了
				//1.循环车牌识别结果
				//标记识别结果大于4
				int sign_reco = 0;
				for (int g = 0; g < strlen(plate_obj.name.c_str()); g++) {
					//2.循环输入参数
					if (plate_obj.name[g] == param[g]) {
						sign_reco++;
					}
				}; //循环车牌识别 结束
				if (sign_reco >= 5) {//识别相对准确
					//将识别结果返回给全局
					plate_name = plate_obj.name;
					//记录开始时间：记录帧 此时识别到的第一帧
					if (starttime == 0.0f) {
						starttime = capture.get(0) / 1000.0f;
						//写下裁剪的车
						time_t _time = time(NULL);
						time_t local_time = time(&_time); //获取时间戳
						sprintf_s(buffer, "plate_reco/%d.jpg", (int)local_time);
						cv::imwrite(buffer, plate_obj.rect_image);//写车牌
						//imshow("视频中要搜索的车牌图像识别", plate_obj.rect_image);
						cv::waitKey(10);
					}
					endtime = capture.get(0) / 1000.0f;
					//记录此车牌出现的次数
					plate_num++;
				}
			}
			else {//没车
				string json = "{";
				json += yy("name", "no plate");
				json += ",";
				json += yy("status", -1);
				json += ",";
				json += yy("path", "");
				json += ",";
				json += yy("number", number);
				json += "}";
				_str_client = json;
			}
		}
	}
	//cv::waitKey(10);
	sprintf_s(buffers, "plate_reco/plate.mp4");
	string s_buf = buffer;
	s_buf += ",";
	s_buf += buffers;
	//json字符串
	string json = "{";
	json += yy("sum", plate_num);
	json += ",";
	json += yy("status", 0);
	json += ",";
	json += yy("name", plate_name.c_str());
	json += ",";
	json += yy("starttime", starttime);
	json += ",";
	json += yy("endtime", endtime);
	json += ",";
	json += yy("path", s_buf.c_str());
	json += ",";
	json += yy("number", number);
	json += "}";
	_str_client = json;
	capture.release();
}

//httpCommand
void http_receive::httpCommand(request_param & rp)
{
	string cmd = rp.getParam("cmd");
	string name = rp.getParam("name");
	string path = rp.getParam("path");
	string number  = rp.getParam("number");
	/*cout << cmd << " " << name << " " << path << " " << number << endl;*/
	int ncmd = -1;
	try
	{
		ncmd = atoi(cmd.c_str());
	}
	catch (...)
	{
		std::cout << "error cmd" << endl;
		return ;
	}

	if (ncmd == Face_Picture_Update)
	{
		string strpath = path;
		string strname = Utf82Gb2312(name.c_str());
		string strnumber = number;
		string _post_client;
		int status = Train_Update(strpath, strname, strnumber, _post_client);
		if (status == 0) {
			//返回给客户端
			http_connect http_client;
			string str = GB23122Utf8(_post_client.c_str());
			http_client.postData("localhost", 5200, "/api/ai", str.c_str());
		}
	}
	else
	{
		http_recv_param * param = new http_recv_param();
		param->cmd = ncmd;
		param->name = Utf82Gb2312(name.c_str());// "薛若梅"; //name;陕AYE360
		param->path = path ;// path;
		param->number = number;
		InsertParam(param);
	}
	return ;
}