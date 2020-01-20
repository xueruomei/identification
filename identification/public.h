#pragma once
#include <string>
#include<opencv2\opencv.hpp> 
using namespace std;
using namespace cv;
//���еĽṹ��

//Ԥ��ֱ�ӷ��ؽ��
struct predict_data
{
	string name;//ʶ�����ԱȺ����ʵ����
	int predictedLabel;//ʶ�����ı�ǩ
	double confidence;//���Ŷ�
};

//����ʶ����
struct pre_ret
{
	string name;//ʶ�����ԱȺ����ʵ����
	int label;//ʶ�����ı�ǩ
	double confidence;//���Ŷ�
	cv::Mat rect_image;//�ü��������
	cv::Mat face_image;//���������
	int ret;//�Ƿ������� 0 �� 1û��
};

//��������ṹ��
struct detect_rect
{
	cv::Rect rect;
};

//���������ݽṹ��
struct mat_rect
{
	//cv::Mat mat;
	cv::Rect rect;
};

//ö��
typedef enum FLAG_CASE
{
	//�����
	Face_Picture_Train = 1, //����ͼƬѵ��
	Face_Picture_Update = 2,//����ͼƬ����
	Face_Picture_Detect = 3, //����ͼƬ���
	Face_Picture_Predict = 4, //����ͼƬʶ��
	Face_Video_Predict = 5,//������Ƶʶ��
	Face_Picture_comparison = 6,//����ͼƬ�Ա�
	//�����
	Plate_Picture_Content_train = 11,//��������ѵ��
	Plate_Picture_Is_train = 12,//����Ƿ��ǳ���ѵ��
	Plate_Picture_Predict = 13,//����ͼƬʶ��
	Plate_Video_Predict = 14, //������Ƶʶ��
	//ͼ�����
	Image_Picture_Contrast = 21,//ͼ��Աȶ���ǿ

}FLAG_CASE;

//һ���ļ��е�����
struct _folder
{
	string Folder_Name;//�ļ��е�����
	list<string> files;//�ļ�·��
	_folder::~_folder() {
		files.clear();
	}
};
//����ļ��е�����
struct _folder_list
{
	list<_folder> folders;//�����ļ��е���Ϣ���ļ������ƣ��ļ������ļ���
	
	int search(string name)
	{
		list<_folder>::iterator iter;
		iter = folders.begin();
		while (iter != folders.end())
		{
			if (iter->Folder_Name == name)
			{
				return 0;
			}
			iter++;
		}
		return -1;
	}
	//clear
	void clear() {
		folders.clear();
	 }

};
















//1.һ���� onePerson
struct one_person
{
	std::string name;//��Ա����
	std::vector <string> images;//��Աͼ��
	int lable;//��Ա��ǩ
};

//2.����� group_db_person
struct group_db_person
{
	map<int, one_person> db;//map�ṹ
							//map ��ĸ���
	int size() {
		return (int)db.size();
	};
	//ѵ��push
	void push(int lable, string namefolder, string images) {
		//cout << "lable:::" << lable << "->namefolder:::" << namefolder << "->images:::" << images << endl;	
		map<int, one_person>::iterator iter;//������
		iter = db.find(lable);//����������
		if (iter != db.end()) {//�ҵ���
							   //cout << "iter->first:::" << iter->first << endl;
			iter->second.images.push_back(images);
			//	db[iter->first].images.push_back(images);
		}
		else  if (iter == db.end()) {//δ�ҵ�
			one_person op;//һ����
			op.name = namefolder;
			op.images.push_back(images);
			db.insert(pair<int, one_person>(lable, op));
			//cout << "op.lable::" << lable << "->" << "op.name" << op.name << endl;
		}
	};
	//���
	void clear() {};
	//���´���
	void set(int lable, string namefolder, string images) {
		one_person op;//һ����
		op.name = namefolder;
		op.lable = lable;//��ǩ
		op.images.push_back(images);
	}
};