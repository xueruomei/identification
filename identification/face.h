#pragma once

/*
author:薛若梅
starttime：2019-1-1
content：人脸训练，侦测，增量训练，识别
*/

#include<vector>
#include <string>
#include "io.h"
#include <iostream>
#include <map>
#include "public.h"
#include "filesystem.h"

using namespace std;
using namespace cv;

class face_common
{
public:
	//1.人脸侦测
	int func_Face_detect(cv::Mat images, std::vector<detect_rect> &detectRect) {
		cv::Mat new_image;
		//haar人脸检测分类器 此算法擅长检测特定的视角的刚性物体（脸，汽车，自行车，人体） 目前最优的检测算法。
		//6.使用模型库去检测人脸
		std::string face_cascade = "face_reco/haarcascade_frontalface_alt.xml";
		std::string eye_cascade = "face_reco/haarcascade_eye_tree_eyeglasses.xml";
		cv::CascadeClassifier faces_cascade_class, eyes_cascade_class;
		faces_cascade_class.load(face_cascade);
		eyes_cascade_class.load(eye_cascade);
		std::vector<cv::Rect> faces;
		faces_cascade_class.detectMultiScale(images, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(15, 15));
		//检测到人脸
		if (faces.size() > 0)
		{
			//6.1框出人脸
			for (int i = 0; i < faces.size(); i++) {
				detect_rect det_obj;
				det_obj.rect = cv::Rect(faces[i].x, faces[i].y, faces[i].width, faces[i].height);
				detectRect.push_back(det_obj);
			}
			return 0;
		}
		else {
			return -1;
		}

	};
	
};

class face_train:public face_common
{
private:
	cv::Ptr<cv::face::FaceRecognizer> _rec;
	std::vector<cv::Mat> _images;
	std::vector<int> _label;
	map<int, string> _label_Name; //一一对应
	int __label = 0;
	string _ori_folder;
	_folder_list _list_main;
public:
	//1.set rec
	void set_rec(cv::Ptr<cv::face::FaceRecognizer> rec) {
		_rec = rec;
	};
	//2.获取所有文件和构造训练数据
	void func_get_files(string folder)
	{
		_list_main.clear();;
		_ori_folder = folder;
		listfolder(folder, _list_main);
		list<_folder>::iterator iter;
		iter = _list_main.folders.begin();
		int ret = 0;
		while (iter != _list_main.folders.end())//找子文件夹- 文件名
		{
		/*	cout << iter->Folder_Name << endl;*/
			list<string>::iterator iter1;
			iter1 = iter->files.begin();
			_label_Name[ret] = iter->Folder_Name;//赋值 名称标签一一对应
			//cout << " ret ::" << ret << " _label_Name[ret] ::" << _label_Name[ret] << endl;
			while (iter1 != iter->files.end())//找文件夹对应的文件
			{
				//cout << ret << "  ---" << (*iter1) << endl;
				//构造训练集
				cv::Mat image = imread((*iter1), 0);
				_images.push_back(image);
				_label.push_back(ret);//每人对应的label
				iter1++;
			}
			_rec->setLabelInfo(ret, iter->Folder_Name);

			iter++;
			ret++;
		}
		__label = (int)_label_Name.size();
		//cout << "__labels" << __label << endl;
		//_list_main.clear();
	};
	//3.训练
	void train()
	{
		if (!_images.empty()) {
			_rec->train(_images, _label);
		}
	};
	//4.增量更新
	int update(string path ,string name,string folder) {
		_ori_folder = folder;//face_reco/FACE
		//总：截取侦测到的人脸
		//1.调用侦测函数
		cv::Mat mat = imread(path, 0);
		std::vector<detect_rect> detectrect;
		int ret = func_Face_detect(mat, detectrect);
		if (ret == 0)//侦测到人脸
		{
			cv::Mat mat1 = mat(detectrect[0].rect);
			resize(mat1, mat1, cv::Size(256, 256));
			string foldname = _ori_folder +"/"+ name ;
			if (_list_main.search(name) != 0)
			{
				CreateDirectoryA(foldname.c_str(),NULL);
				std::vector<cv::Mat> vec;
				vec.push_back(mat1);
				std::vector<int> la;
				la.push_back(__label);
				_rec->update(vec, la);
				_rec->setLabelInfo(__label, name);//set labelinfo
				_label_Name[__label++] = name;
			}
			else
			{
				map<int, string>::iterator iter;
				iter = _label_Name.begin();
				int label = -1;
				while (iter != _label_Name.end())
				{
					if (iter->second == name)
						label = iter->first;
					iter++;
				}
				if (label != -1)
				{
					std::vector<cv::Mat> vec;
					vec.push_back(mat1);
					std::vector<int> la;
					__label = label;//将找到的名称对应的label赋值给__label
					la.push_back(__label);
					_rec->update(vec, la);
					//set label info
					_rec->setLabelInfo(__label, name);
				}
			}
			char buffer[256];
			time_t _time = time(NULL);
			time_t local_time = time(&_time); //获取时间戳
			sprintf_s(buffer, "%s/%s%d.jpg",foldname.c_str(),name.c_str(),(int)local_time);
			cv::imwrite(buffer, mat1);//写侦测到的人脸
			return 0;
		}
		else
		{
			return -1;
		}
	};
	//5.写xml
	void write(std::string dir) 
	{
		_rec->write(dir+"_TRAIN.xml");
	};
	//6.加载xml
	void read(std::string dirfile) {	
		_rec->read(dirfile);
	};
	//7.从label 找到name
	string name_From_Label(int label) {
		return _rec->getLabelInfo(label);
#if 0
		cout << "name_From_Label,label::" << label << endl;
		//return _label_Name[label];
		map<int, string>::iterator iter;
		iter = _label_Name.find(label);
		
		if (iter != _label_Name.end()) {
			return iter->second;
		}
#endif
		return "";
	}

	//8.分割字符串函数
	vector<string> split(const string &s, const string &seperator) {
		vector<string> result;
		typedef string::size_type string_size;
		string_size i = 0;
		while (i != s.size()) {
			//找到字符串中首个不等于分隔符的字母；
			int flag = 0;
			while (i != s.size() && flag == 0) {
				flag = 1;
				for (string_size x = 0; x < seperator.size(); ++x)
					if (s[i] == seperator[x]) {
						++i;
						flag = 0;
						break;
					}
			}

			//找到又一个分隔符，将两个分隔符之间的字符串取出；
			flag = 0;
			string_size j = i;
			while (j != s.size() && flag == 0) {
				for (string_size x = 0; x < seperator.size(); ++x)
					if (s[j] == seperator[x]) {
						flag = 1;
						break;
					}
				if (flag == 0)
					++j;
			}
			if (i != j) {
				result.push_back(s.substr(i, j - i));
				i = j;
			}
		}
		return result;
	}

};

//类的定义
class face_recognition :public face_common
{
private:
	cv::Ptr<cv::face::FaceRecognizer> _rec;
public:
	void set_rec(cv::Ptr<cv::face::FaceRecognizer> rec) {
		_rec = rec;
	};
	
	//3.预测识别
	void predict(cv::Mat image, predict_data & predict_obj) {
		int predictedLabel = -1;
		double confidence = 0.0;
		_rec->predict(image, predictedLabel,confidence);
		predict_obj.confidence = confidence;
		predict_obj.predictedLabel = predictedLabel;		
		return ;
	};
	
	//6.设置阈值
	void setthreshold(float &value) {
		_rec->setThreshold(value);
	};
	//7.获取阈值
	void getthreshold() {
		_rec->getThreshold();
	};
};

//人脸对比类
class face_comparison :public face_common
{

private:
	cv::Ptr<cv::face::FaceRecognizer> _rec;
	
public:
	//1.set rec
	void set_rec(cv::Ptr<cv::face::FaceRecognizer> rec) {
		_rec = rec;
	};
	//分割字符串函数
	void split_str(const string &path, const string &seperator, vector<string> &result) {
		typedef string::size_type string_size;
		string_size i = 0;
		while (i != path.size()) {
			//找到字符串中首个不等于分隔符的字母；
			int flag = 0;
			while (i != path.size() && flag == 0) {
				flag = 1;
				for (string_size x = 0; x < seperator.size(); ++x)
					if (path[i] == seperator[x]) {
						++i;
						flag = 0;
						break;
					}
			}

			//找到又一个分隔符，将两个分隔符之间的字符串取出；
			flag = 0;
			string_size j = i;
			while (j != path.size() && flag == 0) {
				for (string_size x = 0; x < seperator.size(); ++x)
					if (path[j] == seperator[x]) {
						flag = 1;
						break;
					}
				if (flag == 0)
					++j;
			}
			if (i != j) {
				result.push_back(path.substr(i, j - i));
				i = j;
			}
		}
		//return result;
	}
	//人脸对比
	void comparison(cv::Mat train_image,cv::Mat predict_image,  pre_ret &face_obj) {
		//3.侦测训练数据的人脸
		std::vector<detect_rect> detectRect_train;
		//9.侦测人脸识别数据的人脸
		std::vector<detect_rect> detectRect_predict;
		int status_t = func_Face_detect(train_image, detectRect_train);
		int status_p = func_Face_detect(predict_image, detectRect_predict);
		//4.是否有人脸
		//5.如果有人脸 没有人脸return
		if (status_t == -1 || status_p  == -1) {//无人脸
			face_obj.ret = -1;
			face_obj.label = -1;
			face_obj.name = "image is no face";
		}
		else {//图片有人脸
			  //3.1裁剪训练人脸
			cv::Mat rect_train_image = train_image(cv::Rect(detectRect_train[0].rect.x, detectRect_train[0].rect.y, detectRect_train[0].rect.width, detectRect_train[0].rect.height));
			resize(rect_train_image, train_image, Size(256, 256), 0, 0, INTER_LINEAR);
			//裁剪识别人脸
			cv::Mat rect_predict_image = predict_image(cv::Rect(detectRect_predict[0].rect.x, detectRect_predict[0].rect.y, detectRect_predict[0].rect.width, detectRect_predict[0].rect.height));
			resize(rect_predict_image, predict_image, Size(256, 256), 0, 0, INTER_LINEAR);
			cv::Mat train_img = imread("face_reco/21.jpg", 0);
			//imshow("train_img", train_img);
			//6.构造人脸训练集
			std::vector<cv::Mat> _images;
			std::vector<int> _label;
			_images.push_back(train_image);
			_label.push_back(1);
			_images.push_back(train_img);
			_label.push_back(0);
			//6.1参数调优省
			//设置阈值
			//7.训练
			_rec->train(_images, _label);
			//10.构造识别数据
			int predictlabel = _rec->predict(predict_image);
			//cout << "predictlabel::" << predictlabel << endl;
			//11.识别预测
			//12.将识别结果 插入结构体 face_obj
			face_obj.ret = 0;
			face_obj.label = predictlabel;
			//cv::waitKey(10);
		}
	};
};

//接口层的class
class face_interface
{
private:
	cv::Ptr<cv::face::FaceRecognizer> rec = cv::face::LBPHFaceRecognizer::create(3, // radius of LBP pattern 
		8,      // the number of neighboring pixels to consider 相邻像素的数量
		8, 8,   // grid size
		200.);  // minimum distance to nearest neighbor
				// vectors of referencxe image and their labels
	face_train _train; //人脸训练
	face_recognition _recognition; //人脸识别
	face_comparison _comparison;//人脸对比
	bool _init = false;
	string _trainedxmlfile;
public:
	/*std::string face_result[4] = {
		"涂勤瑶","薛若梅","钱波","马绍"
	};*/
	//初始化
	void init(string trainedxmlfile) {
		if (_init == true) {
			return;
		}
		_trainedxmlfile = trainedxmlfile;
		_train.set_rec(rec);
		_recognition.set_rec(rec);
		_comparison.set_rec(rec);
		_init = true;
	};
	//训练
	void train(string folder) {
		
		_train.func_get_files(folder);
		_train.train();
	};
	//增量更新
	int update(const char *path, string name,string folder) {
		if (!_init) {
			return 1;
		}
		if (_train.update(path, name, folder) == 0) {
			return 0;
		}
		else {
			return -1;
		}
	}
	//侦测人脸
	int detect(cv::Mat images, std::vector<detect_rect> &detectRect) {
		if (_recognition.func_Face_detect(images, detectRect) == 0) {
			return 0;
		}
		else {
			return -1;
		}
	};
	//4.加载xml
	void read() 
	{
		//cout << "_trainedxmlfile::" << _trainedxmlfile << endl;
		_train.read(_trainedxmlfile);
		//cout << "read finished" << endl;
	};
	//5.写xml
	void write(std::string dir)
	{
		_train.write(dir);
	};
	//6.设置阈值
	void setthreshold(float &value) 
	{
		_recognition.setthreshold(value);
	};
	//7.获取阈值
	void getthreshold() 
	{
		_recognition.getthreshold();
	};
	//预测
	void predict(cv::Mat image, predict_data & predict_obj) 
	{
		if (!_init) {
			return ;//未初始化
			//init();
		}
		_recognition.predict(image,predict_obj);
	};
	//找label对应的名称
	string name_Label(int predictedLabel) 
	{
		return _train.name_From_Label(predictedLabel);
	}
	//分割路径
	void split_str(const string path, const string &seperator, vector<string> &result) {
	 _comparison.split_str(path, seperator, result);
	}
	//人脸对比
	void comparison(cv::Mat train_image,cv::Mat predict_image, pre_ret &face_obj) {
		_comparison.comparison(train_image, predict_image,face_obj);
	}
};