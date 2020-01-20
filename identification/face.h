#pragma once

/*
author:Ѧ��÷
starttime��2019-1-1
content������ѵ������⣬����ѵ����ʶ��
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
	//1.�������
	int func_Face_detect(cv::Mat images, std::vector<detect_rect> &detectRect) {
		cv::Mat new_image;
		//haar������������ ���㷨�ó�����ض����ӽǵĸ������壨�������������г������壩 Ŀǰ���ŵļ���㷨��
		//6.ʹ��ģ�Ϳ�ȥ�������
		std::string face_cascade = "face_reco/haarcascade_frontalface_alt.xml";
		std::string eye_cascade = "face_reco/haarcascade_eye_tree_eyeglasses.xml";
		cv::CascadeClassifier faces_cascade_class, eyes_cascade_class;
		faces_cascade_class.load(face_cascade);
		eyes_cascade_class.load(eye_cascade);
		std::vector<cv::Rect> faces;
		faces_cascade_class.detectMultiScale(images, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(15, 15));
		//��⵽����
		if (faces.size() > 0)
		{
			//6.1�������
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
	map<int, string> _label_Name; //һһ��Ӧ
	int __label = 0;
	string _ori_folder;
	_folder_list _list_main;
public:
	//1.set rec
	void set_rec(cv::Ptr<cv::face::FaceRecognizer> rec) {
		_rec = rec;
	};
	//2.��ȡ�����ļ��͹���ѵ������
	void func_get_files(string folder)
	{
		_list_main.clear();;
		_ori_folder = folder;
		listfolder(folder, _list_main);
		list<_folder>::iterator iter;
		iter = _list_main.folders.begin();
		int ret = 0;
		while (iter != _list_main.folders.end())//�����ļ���- �ļ���
		{
		/*	cout << iter->Folder_Name << endl;*/
			list<string>::iterator iter1;
			iter1 = iter->files.begin();
			_label_Name[ret] = iter->Folder_Name;//��ֵ ���Ʊ�ǩһһ��Ӧ
			//cout << " ret ::" << ret << " _label_Name[ret] ::" << _label_Name[ret] << endl;
			while (iter1 != iter->files.end())//���ļ��ж�Ӧ���ļ�
			{
				//cout << ret << "  ---" << (*iter1) << endl;
				//����ѵ����
				cv::Mat image = imread((*iter1), 0);
				_images.push_back(image);
				_label.push_back(ret);//ÿ�˶�Ӧ��label
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
	//3.ѵ��
	void train()
	{
		if (!_images.empty()) {
			_rec->train(_images, _label);
		}
	};
	//4.��������
	int update(string path ,string name,string folder) {
		_ori_folder = folder;//face_reco/FACE
		//�ܣ���ȡ��⵽������
		//1.������⺯��
		cv::Mat mat = imread(path, 0);
		std::vector<detect_rect> detectrect;
		int ret = func_Face_detect(mat, detectrect);
		if (ret == 0)//��⵽����
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
					__label = label;//���ҵ������ƶ�Ӧ��label��ֵ��__label
					la.push_back(__label);
					_rec->update(vec, la);
					//set label info
					_rec->setLabelInfo(__label, name);
				}
			}
			char buffer[256];
			time_t _time = time(NULL);
			time_t local_time = time(&_time); //��ȡʱ���
			sprintf_s(buffer, "%s/%s%d.jpg",foldname.c_str(),name.c_str(),(int)local_time);
			cv::imwrite(buffer, mat1);//д��⵽������
			return 0;
		}
		else
		{
			return -1;
		}
	};
	//5.дxml
	void write(std::string dir) 
	{
		_rec->write(dir+"_TRAIN.xml");
	};
	//6.����xml
	void read(std::string dirfile) {	
		_rec->read(dirfile);
	};
	//7.��label �ҵ�name
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

	//8.�ָ��ַ�������
	vector<string> split(const string &s, const string &seperator) {
		vector<string> result;
		typedef string::size_type string_size;
		string_size i = 0;
		while (i != s.size()) {
			//�ҵ��ַ������׸������ڷָ�������ĸ��
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

			//�ҵ���һ���ָ������������ָ���֮����ַ���ȡ����
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

//��Ķ���
class face_recognition :public face_common
{
private:
	cv::Ptr<cv::face::FaceRecognizer> _rec;
public:
	void set_rec(cv::Ptr<cv::face::FaceRecognizer> rec) {
		_rec = rec;
	};
	
	//3.Ԥ��ʶ��
	void predict(cv::Mat image, predict_data & predict_obj) {
		int predictedLabel = -1;
		double confidence = 0.0;
		_rec->predict(image, predictedLabel,confidence);
		predict_obj.confidence = confidence;
		predict_obj.predictedLabel = predictedLabel;		
		return ;
	};
	
	//6.������ֵ
	void setthreshold(float &value) {
		_rec->setThreshold(value);
	};
	//7.��ȡ��ֵ
	void getthreshold() {
		_rec->getThreshold();
	};
};

//�����Ա���
class face_comparison :public face_common
{

private:
	cv::Ptr<cv::face::FaceRecognizer> _rec;
	
public:
	//1.set rec
	void set_rec(cv::Ptr<cv::face::FaceRecognizer> rec) {
		_rec = rec;
	};
	//�ָ��ַ�������
	void split_str(const string &path, const string &seperator, vector<string> &result) {
		typedef string::size_type string_size;
		string_size i = 0;
		while (i != path.size()) {
			//�ҵ��ַ������׸������ڷָ�������ĸ��
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

			//�ҵ���һ���ָ������������ָ���֮����ַ���ȡ����
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
	//�����Ա�
	void comparison(cv::Mat train_image,cv::Mat predict_image,  pre_ret &face_obj) {
		//3.���ѵ�����ݵ�����
		std::vector<detect_rect> detectRect_train;
		//9.�������ʶ�����ݵ�����
		std::vector<detect_rect> detectRect_predict;
		int status_t = func_Face_detect(train_image, detectRect_train);
		int status_p = func_Face_detect(predict_image, detectRect_predict);
		//4.�Ƿ�������
		//5.��������� û������return
		if (status_t == -1 || status_p  == -1) {//������
			face_obj.ret = -1;
			face_obj.label = -1;
			face_obj.name = "image is no face";
		}
		else {//ͼƬ������
			  //3.1�ü�ѵ������
			cv::Mat rect_train_image = train_image(cv::Rect(detectRect_train[0].rect.x, detectRect_train[0].rect.y, detectRect_train[0].rect.width, detectRect_train[0].rect.height));
			resize(rect_train_image, train_image, Size(256, 256), 0, 0, INTER_LINEAR);
			//�ü�ʶ������
			cv::Mat rect_predict_image = predict_image(cv::Rect(detectRect_predict[0].rect.x, detectRect_predict[0].rect.y, detectRect_predict[0].rect.width, detectRect_predict[0].rect.height));
			resize(rect_predict_image, predict_image, Size(256, 256), 0, 0, INTER_LINEAR);
			cv::Mat train_img = imread("face_reco/21.jpg", 0);
			//imshow("train_img", train_img);
			//6.��������ѵ����
			std::vector<cv::Mat> _images;
			std::vector<int> _label;
			_images.push_back(train_image);
			_label.push_back(1);
			_images.push_back(train_img);
			_label.push_back(0);
			//6.1��������ʡ
			//������ֵ
			//7.ѵ��
			_rec->train(_images, _label);
			//10.����ʶ������
			int predictlabel = _rec->predict(predict_image);
			//cout << "predictlabel::" << predictlabel << endl;
			//11.ʶ��Ԥ��
			//12.��ʶ���� ����ṹ�� face_obj
			face_obj.ret = 0;
			face_obj.label = predictlabel;
			//cv::waitKey(10);
		}
	};
};

//�ӿڲ��class
class face_interface
{
private:
	cv::Ptr<cv::face::FaceRecognizer> rec = cv::face::LBPHFaceRecognizer::create(3, // radius of LBP pattern 
		8,      // the number of neighboring pixels to consider �������ص�����
		8, 8,   // grid size
		200.);  // minimum distance to nearest neighbor
				// vectors of referencxe image and their labels
	face_train _train; //����ѵ��
	face_recognition _recognition; //����ʶ��
	face_comparison _comparison;//�����Ա�
	bool _init = false;
	string _trainedxmlfile;
public:
	/*std::string face_result[4] = {
		"Ϳ����","Ѧ��÷","Ǯ��","����"
	};*/
	//��ʼ��
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
	//ѵ��
	void train(string folder) {
		
		_train.func_get_files(folder);
		_train.train();
	};
	//��������
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
	//�������
	int detect(cv::Mat images, std::vector<detect_rect> &detectRect) {
		if (_recognition.func_Face_detect(images, detectRect) == 0) {
			return 0;
		}
		else {
			return -1;
		}
	};
	//4.����xml
	void read() 
	{
		//cout << "_trainedxmlfile::" << _trainedxmlfile << endl;
		_train.read(_trainedxmlfile);
		//cout << "read finished" << endl;
	};
	//5.дxml
	void write(std::string dir)
	{
		_train.write(dir);
	};
	//6.������ֵ
	void setthreshold(float &value) 
	{
		_recognition.setthreshold(value);
	};
	//7.��ȡ��ֵ
	void getthreshold() 
	{
		_recognition.getthreshold();
	};
	//Ԥ��
	void predict(cv::Mat image, predict_data & predict_obj) 
	{
		if (!_init) {
			return ;//δ��ʼ��
			//init();
		}
		_recognition.predict(image,predict_obj);
	};
	//��label��Ӧ������
	string name_Label(int predictedLabel) 
	{
		return _train.name_From_Label(predictedLabel);
	}
	//�ָ�·��
	void split_str(const string path, const string &seperator, vector<string> &result) {
	 _comparison.split_str(path, seperator, result);
	}
	//�����Ա�
	void comparison(cv::Mat train_image,cv::Mat predict_image, pre_ret &face_obj) {
		_comparison.comparison(train_image, predict_image,face_obj);
	}
};