#pragma once
#include <string>
#include<opencv2\opencv.hpp> 
using namespace std;
using namespace cv;
//所有的结构体

//预测直接返回结果
struct predict_data
{
	string name;//识别结果对比后的真实名称
	int predictedLabel;//识别结果的标签
	double confidence;//置信度
};

//返回识别结果
struct pre_ret
{
	string name;//识别结果对比后的真实名称
	int label;//识别结果的标签
	double confidence;//置信度
	cv::Mat rect_image;//裁剪后的人脸
	cv::Mat face_image;//框出的人脸
	int ret;//是否有名称 0 有 1没有
};

//侦测人脸结构体
struct detect_rect
{
	cv::Rect rect;
};

//排序车牌内容结构体
struct mat_rect
{
	//cv::Mat mat;
	cv::Rect rect;
};

//枚举
typedef enum FLAG_CASE
{
	//人相关
	Face_Picture_Train = 1, //人脸图片训练
	Face_Picture_Update = 2,//人脸图片增量
	Face_Picture_Detect = 3, //人脸图片侦测
	Face_Picture_Predict = 4, //人脸图片识别
	Face_Video_Predict = 5,//人脸视频识别
	Face_Picture_comparison = 6,//人脸图片对比
	//车相关
	Plate_Picture_Content_train = 11,//车牌内容训练
	Plate_Picture_Is_train = 12,//辨别是否是车牌训练
	Plate_Picture_Predict = 13,//车牌图片识别
	Plate_Video_Predict = 14, //车牌视频识别
	//图像相关
	Image_Picture_Contrast = 21,//图像对比度增强

}FLAG_CASE;

//一个文件夹的数据
struct _folder
{
	string Folder_Name;//文件夹的名称
	list<string> files;//文件路径
	_folder::~_folder() {
		files.clear();
	}
};
//多个文件夹的数据
struct _folder_list
{
	list<_folder> folders;//所有文件夹的信息（文件夹名称，文件夹中文件）
	
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
















//1.一个人 onePerson
struct one_person
{
	std::string name;//人员名称
	std::vector <string> images;//人员图像
	int lable;//人员标签
};

//2.多个人 group_db_person
struct group_db_person
{
	map<int, one_person> db;//map结构
							//map 表的个数
	int size() {
		return (int)db.size();
	};
	//训练push
	void push(int lable, string namefolder, string images) {
		//cout << "lable:::" << lable << "->namefolder:::" << namefolder << "->images:::" << images << endl;	
		map<int, one_person>::iterator iter;//迭代器
		iter = db.find(lable);//二叉树查找
		if (iter != db.end()) {//找到了
							   //cout << "iter->first:::" << iter->first << endl;
			iter->second.images.push_back(images);
			//	db[iter->first].images.push_back(images);
		}
		else  if (iter == db.end()) {//未找到
			one_person op;//一个人
			op.name = namefolder;
			op.images.push_back(images);
			db.insert(pair<int, one_person>(lable, op));
			//cout << "op.lable::" << lable << "->" << "op.name" << op.name << endl;
		}
	};
	//清除
	void clear() {};
	//更新存入
	void set(int lable, string namefolder, string images) {
		one_person op;//一个人
		op.name = namefolder;
		op.lable = lable;//标签
		op.images.push_back(images);
	}
};