// videocapture.cpp : �������̨Ӧ�ó������ڵ㡣
/*
author:Ѧ��÷
starttime��2019-3-28
content����ȡ��Ƶ�ļ������ÿ֡���������ƣ� ʶ��ÿ֡���������ƣ�
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
#include "recognition.h" //����ʶ������
#include "common.h"
using namespace std;
using namespace cv;
using namespace cv::ml;


/*
params: [1] = 1 
params��[2] =Ѧ��÷   ʶ��˭
params��[3] ·��
*/

/*
	1.�����
*/
static string buf() {
	char buffer[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, &buffer[0]);//��ȡ��ǰ����·��
	for (int i = 0; i < MAX_PATH; i++) {
		if (buffer[i] == '\\') {
			buffer[i] = '/';
		}
	}
	return buffer;
}

//�������·���ַ�
static string handle_disk_path(string handle_buf) {
	for (int i = 0; i < handle_buf.size(); i++) {
		if (handle_buf[i] == '\\') {
			handle_buf[i] = '/';
		}
	}
	return handle_buf;
 }

//����Э��·��
//string handle_http_path(string file_name) {
//	string http_path = "http://192.168.1.239:5200/" + file_name;
//
//	return http_path;
//}

//1.1��������ʶ��
void wrapper::func_Face_common_Predict(cv::Mat origin, pre_ret &face_obj)
{
	face_obj = {};//���
	cv::Mat gray;//�Ҷ�
	cvtColor(origin, gray, cv::COLOR_BGR2GRAY);
	//1.�������
	std::vector<detect_rect> detectRect; //�������������
	int detect_status = face_api.detect(gray, detectRect);
	if (detect_status == 0) {
		//��⵽����
		//1.�ü�����ȥʶ��
		//�Ҷ�ͼ�ü�
		cv::Mat rect_image = gray(cv::Rect(detectRect[0].rect.x, detectRect[0].rect.y, detectRect[0].rect.width, detectRect[0].rect.height));
		//ԭͼ�ü�
		cv::Mat rect_cnl3 = origin(cv::Rect(detectRect[0].rect.x, detectRect[0].rect.y, detectRect[0].rect.width, detectRect[0].rect.height));
		//2.ʶ��
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
	else {//δ��⵽����
		face_obj.ret = -1;
		face_obj.name="";
		face_obj.label = -1;
		face_obj.confidence = 0.0;
	}
}

//1.2�������
void wrapper::func_Face_Picture_Dectet(const char*path,const char *number, string &_str_client) {
	cv::Mat gray = cv::imread(path,0);//�Ҷ�Mat
	cv::Mat origin = cv::imread(path);//ԭͼMat
	if (gray.empty()) {
		std::cout << "please input file" << endl;
		return ;
	};
	std::vector<detect_rect> detectRect; //�������������
	char buffer[256];
	int status = face_api.detect(gray, detectRect);
	int sum = (int)detectRect.size();//��������
	string json = "{";
	json += yy("sum", sum);//��������
	json += ",";
	if (status == 0) {//������
		for (int i = 0; i < detectRect.size(); i++) {
			cv::rectangle(origin, cv::Rect(cv::Rect(detectRect[i].rect.x, detectRect[i].rect.y, detectRect[i].rect.width, detectRect[i].rect.height)), cv::Scalar(0, 0, 255), 2, 8);
		}
		time_t _time = time(NULL);
		time_t local_time = time(&_time); //��ȡʱ���
		sprintf_s(buffer, "face_reco/%d.jpg", (int)local_time);
		cv::imwrite(buffer, origin);//д��⵽������
		//cv::imshow("�������", origin);
		string path_arr[] = {""};
		//���json
		json += yy("status", 0);
		json += ",";
		json += yy("path",buffer);//������·��
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
		//std::cout << json << endl;
		//cv::waitKey(10);
	}
	else {//������
		//���json
		json += yy("status", -1);
		json += ",";
		json += yy("path", "");//������·��
		json += ",";
		json += yy("number", number);
		json += "}";
		//std::cout << json << endl;
		_str_client = json;
	}
}

//1.3����ͼƬʶ��
void wrapper::func_Face_Picture_Predict(const char * path, const char *number, pre_ret &face_obj,string &_str_client)
{
	cv::Mat origin = cv::imread(path);//ԭͼ
	if (origin.empty()) {
		std::cout << "please input file" << endl;
		return ;
	}
	char buffer[256];//ͷ��·��
	func_Face_common_Predict(origin, face_obj);//����ʶ��
	if (face_obj.ret == 0) {//����
		if (face_obj.name != "") {
			time_t _time = time(NULL);
			time_t local_time = time(&_time); //��ȡʱ���
			sprintf_s(buffer, "face_reco/%d.jpg", (int)local_time);
			//cv::imshow("����ͼƬʶ��", face_obj.rect_image);
			cv::imwrite(buffer, face_obj.rect_image);//дͷ��
			
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
	else {//û��
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

//1.4������Ƶʶ��
void wrapper::func_Face_Video_Predict(const char * param, const char *number, pre_ret &face_obj, string &_str_client)
{
	//11.ʵ������Ƶ����
	cv::VideoCapture capture;
	//6.��ȡ������Ƶ
	//8.��ȡ��Ƶÿһ֡ͼ��ȥ�������
	capture.open("face_reco/face.mp4");
	//capture.open(path);
	if (!capture.isOpened())
	{
		std::cout << "video open error" << std::endl;
		return ;
	}
	//cout << "��֡��::" << capture.get(CV_CAP_PROP_FRAME_COUNT) << endl;
	//7.��ȡ��Ƶÿһ֡ͼ��ȥ���
	int G_freeframe = 0;
	double starttime = 0.0f;
	double endtime = 0.0f;
	//ÿ���֡����
	char buffer[256];//ͷ��·��
	char buffers[256];//��Ƶ·��
	while (1)
	{
		//Mat ����
		cv::Mat frame, face_image;
		capture >> frame;
		if (frame.empty()) {
			break;
		}
		else {
			//cv::imshow("��ȡ��Ƶ", frame);
			func_Face_common_Predict(frame, face_obj);//����ʶ��
			if (face_obj.ret == 0) {//����
				if (face_obj.name.compare(param) == 0) {//ʶ�����ȶ�
					G_freeframe++; //���˳��ֵĴ���
					if (starttime == 0.0f) {
						starttime = capture.get(0) / 1000.0f;
						//д�²ü���ͷ��
						time_t _time = time(NULL);
						time_t local_time = time(&_time); //��ȡʱ���
						sprintf_s(buffer, "face_reco/%d.jpg", (int)local_time);
						cv::imwrite(buffer, face_obj.rect_image);//дͷ��
						//cv::imshow("��ȡ��Ƶ��������ʶ��", face_obj.rect_image);
						cv::waitKey(10);
					};
					endtime = capture.get(0) / 1000.0f;
				}
			}
			else {//û��
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
	//json�ַ���
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

//����ͼƬ�Ա�
void wrapper::func_Face_Picture_comparison(const char * path, const char *number, pre_ret &face_obj, string &_str_client) {
	//1.�ָ��ȡcv::Mat
	vector<string> result;
	face_api.split_str(path,",", result);
	//2.��ȡ��һ���͵ڶ���ͼƬ
	cv::Mat train_image = imread(result[0], 0);
	
	cv::Mat predict_image = imread(result[1], 0);
	
	/*imshow("train_image", train_image);
	imshow("predict_image", predict_image);
	cvWaitKey(10);*/
	//3.ȥ���ȽϺ���
	face_api.comparison(train_image, predict_image, face_obj);
	//�յ� face_obj ��ʶ���� 
	//����json���ظ��ͻ���
	if (face_obj.ret == 0 ) {
		if (face_obj.label >= 1) {
			string json = "{";
			json += yy("status", 0);
			json += ",";
			json += yy("name", "��ͬһ����");
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
			json += yy("name", "����ͬһ����");
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

//1.5����ѵ��
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

//1.6��������
void wrapper::func_Face_Update(const char * path, const char * param, const char *number, string &_str_client) {
	int status = face_api.update(path, param, _ori_folder);
	if (status == 0) {
		//���³ɹ�
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
		//����ʧ��
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
	2.ͼ�����
*/

void wrapper::func_Image_Contrast(const char * path, const char *number, pre_ret &image_obj, string &_str_client) {
	//1.��ȡͼƬ
	cv::Mat image = imread(path);
	double gamma = 2.000; //�߻Ҷ�
	//����ͼ��Աȶ���ǿ
	image_contrast_api.contrast_enhancement(image, gamma,image_obj);
	char buffer[256];
	char buffers[256];
	vector<string> result;
	face_api.split_str(path,"/",result);
	//����json ���ظ��ͻ���
	if (image_obj.ret == 0) {//�Ա���ǿ�ɹ�
		//��ԭͼ������֮���ͼ��д�����
		time_t _time = time(NULL);
		time_t local_time = time(&_time); //��ȡʱ���
		sprintf_s(buffer, "image/%d.jpg", (int)local_time);

		cv::imwrite(buffer, image_obj.rect_image);//д������ͼ��
		int split_one = result.size()-1;
		int split_two = result.size() - 2;
		string bufs = result[split_two] + "/" + result[split_one];
		string s_buf = buffer;
		s_buf += ",";
		s_buf += bufs;
		//cv::imshow("ͼ����ǿ", image_obj.rect_image);
		//json�ַ���
		string json = "{";
		json += yy("status", 0);
		json += ",";
		json += yy("path", s_buf.c_str());//ƴ��ԭͼ·��
		json += ",";
		json += yy("number", number);
		json += "}";
		_str_client = json;
		//cout << "_str_client:" << _str_client << endl;
	}
	else {//�Ա���ǿʧ��

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
	3.�����
*/

//2.4��������ѵ��
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

//2.5������������ѵ��
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

//����ʶ�𹫹�����
void wrapper::func_Plate_Common_Predict(cv::Mat image, Ptr<SVM> svm, HOGDescriptor hog, pre_ret &plate_obj) {
	//2.0candy ���ͼ�񣨳��ƣ�����
	//���ȫͼ�ı�Ե����
	cv::Mat gray;//�Ҷ�ͼ��
	cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	cv::Mat contour;//�������
	cv::Canny(gray, contour, 125, 350);
	//3.0��̬ѧ����任ͼ��
	//��̬ѧ����
	//ͼƬ���ʹ���
	cv::Mat dilate_image, erode_image;
	//�Զ��� �˽��� x ��������͸�ʴ
	cv::Mat elementX = getStructuringElement(MORPH_RECT, Size(30, 1));
	cv::Mat elementY = getStructuringElement(MORPH_RECT, Size(1, 25));
	Point point(-1, -1);
	cv::dilate(contour, dilate_image, elementX, point, 2);
	cv::erode(dilate_image, erode_image, elementX, point, 4);
	cv::dilate(erode_image, dilate_image, elementX, point, 2);
	//�Զ��� �˽��� Y ��������͸�ʴ
	cv::erode(dilate_image, erode_image, elementY, point, 1);
	cv::dilate(erode_image, dilate_image, elementY, point, 2);
	//4.��������
	//ƽ������ ��ֵ�˲�
	cv::Mat blurr_image;
	cv::medianBlur(dilate_image, blurr_image, 5);
	vector<vector<Point>> contours;
	//�Ӷ�ֵͼ���������
	cv::findContours(blurr_image, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//�г�����ˮƽ����ֱ��Խ��������Ķ˵�
	//6.������ʾΪһ�����γ�����ȡ
	cv::Mat  roi_image,origin_image;
	for (int i = 0; i < contours.size(); i++) {
		cv::Rect r = boundingRect(cv::Mat(contours[i]));
		if ((float)r.width / r.height >= 2.2 && (float)r.width / r.height <= 3.6) {
			//����� ����
			roi_image = gray(r);
			origin_image = image(r);//ԭͼ�ü����ظ��ͻ���
			resize(roi_image, roi_image, Size(128, 64), 0, 0, INTER_LINEAR);

			//ʶ���Ƿ��ǳ���
			float res = plate_lience_api.predict_license(roi_image);
			//ȥʶ��
			if (res >= 1.0f) {//�ǳ���
				cv::Mat image = plate_common::adjust(roi_image);//��ת
			
				std::vector<mat_rect> sormatrect; //�������rect
				cv::Mat return_data = plate_content_api.OCR_split(image, sormatrect);//�и�
				cv::Mat copy_return_data;
				return_data.copyTo(copy_return_data);//copy Ϊ�˲ü�û�б߿�
				string lensnce_plate = "";
				//�޸ĳߴ�
				for (int i = 0; i < sormatrect.size(); i++) {
					rectangle(return_data, sormatrect[i].rect, Scalar(0, 255, 0));
					cv::Mat outs;
					cv::Mat ROI = copy_return_data(Rect(sormatrect[i].rect));
					resize(ROI, outs, Size(32, 40), 0, 0, INTER_LINEAR);
					//��ʼʶ��
					vector<float> descriptors;//HOG����������
					hog.compute(outs, descriptors, Size(8, 8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
					float lable = svm->predict(descriptors);//��7��ͼ�����Ԥ��
					lensnce_plate += plate_content_api.label_name((int)lable);
				};
				plate_obj.ret = 0;
				plate_obj.name = lensnce_plate;
				plate_obj.rect_image = origin_image;
			}
			else if (res == -1) {//���ǳ���
				plate_obj.ret = -1;
				plate_obj.name = "";
			} //�Ƿ��ǳ��� ����
		}
		
	};
}

//����ͼƬʶ��
void wrapper::func_Plate_Picture_Predict(const char * path, const char * param, const char *number, pre_ret &plate_obj, string &_str_client) {
	Ptr<SVM> svm = SVM::create();
	svm = SVM::load("plate_reco/PLATE_CONTENT.xml");//load train file
	//��ⴰ��(32, 40),��ߴ�(16,16),�鲽��(8,8),cell�ߴ�(8,8),ֱ��ͼbin����9
	HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
	//cv::Mat origin = cv::imread("plate_reco/1.jpg");//1.0ԭͼ��
	cv::Mat origin = cv::imread(path);//1.0ԭͼ��
	if (origin.empty()) {
		std::cout << "image.empty()" << endl;
		return ;
	}
	//���ó��ƹ�������
	wrapper::func_Plate_Common_Predict(origin, svm, hog, plate_obj);
	char buffer[256];
	if (plate_obj.ret == 0) {//�г�
		//���ʶ��������4
		//int sign_reco = 0;
		//for (int g = 0; g < strlen(plate_obj.name.c_str()); g++) {
		//	//2.ѭ���������
		//	if (plate_obj.name[g] == param[g]) {
		//		sign_reco++;
		//	}
		//}; //ѭ������ʶ�� ����
		//if (sign_reco >= 4) {//ʶ�����׼ȷ
		//		//д�²ü��ĳ�
		//		time_t _time = time(NULL);
		//		time_t local_time = time(&_time); //��ȡʱ���
		//		sprintf_s(buffer, "plate_reco/%d.jpg", (int)local_time);
		//		cv::imwrite(buffer, plate_obj.rect_image);//д����
		//}
		time_t _time = time(NULL);
		time_t local_time = time(&_time); //��ȡʱ���
		sprintf_s(buffer, "plate_reco/%d.jpg", (int)local_time);
		cv::imwrite(buffer, plate_obj.rect_image);//д����
		//cv::imshow("����ͼƬʶ��", plate_obj.rect_image);
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
	else {//û��
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

//������Ƶʶ��
void wrapper::func_Plate_Video_Predict(const char *path, const char * param, const char *number, pre_ret &plate_obj, string &_str_client) {
	Ptr<SVM> svm = SVM::create();
	//load train file
	svm = SVM::load("plate_reco/PLATE_CONTENT.xml");
	//��ⴰ��(32, 40),��ߴ�(16,16),�鲽��(8,8),cell�ߴ�(8,8),ֱ��ͼbin����9
	HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
	//11.ʵ������Ƶ����
	cv::VideoCapture capture;
	capture.open("plate_reco/plate.mp4");
	//capture.open(path);
	if (!capture.isOpened())
	{
		std::cout << "video open error" << std::endl;
		return ;
	}
	int plate_num = 0;//��¼�ó��Ƴ��ֵĴ���
	double endtime = 0.0f;//����ʱ��
	double starttime = 0.0f;//��ʼʱ��
	char buffer[256];
	char buffers[256];
	string plate_name = "";
	int G_freeframe = 0;//ʶ�𵽽���Ĵ���
	while (1)
	{
		cv::Mat origin, gray;
		capture >> origin;
		if (origin.empty()) {
			break;
		}
		else {
			//cv::imshow("��ȡ��Ƶ", origin);
			//���ó��ƹ�������
			wrapper::func_Plate_Common_Predict(origin, svm, hog, plate_obj);
			if (plate_obj.ret == 0) {//�г�
				//ѭ��ʶ���� ѭ������Ҫʶ�������
				//���������ֽ���Աȣ��ĸ����ĸ����� �������ʱ ����Ϊʶ�������
				//1.ѭ������ʶ����
				//���ʶ��������4
				int sign_reco = 0;
				for (int g = 0; g < strlen(plate_obj.name.c_str()); g++) {
					//2.ѭ���������
					if (plate_obj.name[g] == param[g]) {
						sign_reco++;
					}
				}; //ѭ������ʶ�� ����
				if (sign_reco >= 5) {//ʶ�����׼ȷ
					//��ʶ�������ظ�ȫ��
					plate_name = plate_obj.name;
					//��¼��ʼʱ�䣺��¼֡ ��ʱʶ�𵽵ĵ�һ֡
					if (starttime == 0.0f) {
						starttime = capture.get(0) / 1000.0f;
						//д�²ü��ĳ�
						time_t _time = time(NULL);
						time_t local_time = time(&_time); //��ȡʱ���
						sprintf_s(buffer, "plate_reco/%d.jpg", (int)local_time);
						cv::imwrite(buffer, plate_obj.rect_image);//д����
						//imshow("��Ƶ��Ҫ�����ĳ���ͼ��ʶ��", plate_obj.rect_image);
						cv::waitKey(10);
					}
					endtime = capture.get(0) / 1000.0f;
					//��¼�˳��Ƴ��ֵĴ���
					plate_num++;
				}
			}
			else {//û��
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
	//json�ַ���
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
			//���ظ��ͻ���
			http_connect http_client;
			string str = GB23122Utf8(_post_client.c_str());
			http_client.postData("localhost", 5200, "/api/ai", str.c_str());
		}
	}
	else
	{
		http_recv_param * param = new http_recv_param();
		param->cmd = ncmd;
		param->name = Utf82Gb2312(name.c_str());// "Ѧ��÷"; //name;��AYE360
		param->path = path ;// path;
		param->number = number;
		InsertParam(param);
	}
	return ;
}