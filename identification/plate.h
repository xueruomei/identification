#pragma once
#include "stdafx.h"
#include "linefinder.h"//画线
#include "edgedetector.h"
#include "public.h"
#include "filesystem.h"

//比较大小
static bool funCmpByValue(const mat_rect& a, const mat_rect& b)
{
	return a.rect.x < b.rect.x;
}

//公共
class plate_common 
{
private:
	
	
 protected:
	 string _trained_file;
	 string _strfolder;
	 Ptr<SVM> svm = SVM::create();//SVM分类器
	 _folder_list _list_main;
	/*string _plate_xml_content = "plate_reco/PLATE_CONTENT.xml";
	string _plate_xml_sample = "plate_reco/PLATE_SAMPLE.xml";
	string _plate_content_train = "plate_reco/train_content";
	string _plate_is_train = "plate_reco/train_is";*/
public:
	void init(string trained_file,string folder) {
		_trained_file = trained_file;
		_strfolder = folder;
	};
	
	//字间距等算法
	/*
	in:CV::Mat
	*/
	bool verifySizes(cv::Mat r)
	{
		//Char sizes 45x77
		float aspect = 45.0f / 77.0f;
		float charAspect = (float)r.cols / (float)r.rows;
		float error = 0.35f;
		float minHeight = 15.0f;
		float maxHeight = 28.0f;
		//We have a different aspect ratio for number 1, and it can be ~0.2
		float minAspect = 0.2f;
		float maxAspect = aspect + aspect*error;
		//area of pixels
		float area = (float)countNonZero(r);
		//bb area
		float bbArea = (float)r.cols*r.rows;
		//% of pixel in area
		float percPixels = area / bbArea;

		if (percPixels < 0.8 && charAspect > minAspect && charAspect < maxAspect && r.rows >= minHeight && r.rows < maxHeight)
			return true;
		else
			return false;
	};

	//预处理程序
	/*
	in:CV::Mat
	*/
	cv::Mat preprocessChar(cv::Mat in)
	{
		//Remap image
		int h = in.rows;
		int w = in.cols;
		Mat transformMat = Mat::eye(2, 3, CV_32F);
		float m = max((float)w, (float)h);
		transformMat.at<float>(0, 2) = m / 2 - w / 2;
		transformMat.at<float>(1, 2) = m / 2 - h / 2;
		Mat warpImage((int)m, (int)m, in.type());
		warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0));
		Mat out;
		resize(warpImage, out, Size(20, 20));
		return out;
	};

	//调整函数
	static cv::Mat adjust(cv::Mat image) {
		return rotate(image);
	};

	//车牌变换 并计算角度 、旋转
	/*
	in:CV::Mat
	*/
	static cv::Mat rotate(cv::Mat image) {
		cv::Mat clone_image;
		image.copyTo(clone_image);
		if (image.data) {
			// Compute Sobel
			EdgeDetector ed;
			ed.computeSobel(image);
			// Apply Canny algorithm
			cv::Mat contours;
			cv::Canny(image, contours, 125, 350);
			//// Create LineFinder instance
			LineFinder ld;
			// Set probabilistic Hough parameters
			ld.setLineLengthAndGap(100, 20);
			ld.setMinVote(60);
			// Detect lines
			std::vector<cv::Vec4i> li = ld.findLines(contours);
			ld.drawDetectedLines(image);
			std::vector<cv::Vec4i>::const_iterator it2 = li.begin();
			//theta
			double theta = 0.0;
			while (it2 != li.end()) {
				int x1 = (*it2)[0];
				int x2 = (*it2)[2];
				int y1 = (*it2)[1];
				int y2 = (*it2)[3];

				//计算角度
				//1.0计算边长 对边 斜边
				// 计算斜边长
				double bevel_edge = sqrt((x2 - x1) *(x2 - x1) + (y2 - y1) * (y2 - y1));
				//画邻边
				cv::line(image, cv::Point(x1, y1), cv::Point(x2, y1), cv::Scalar(255, 255, 255), 1);
				//计算直线的终点
				int x3 = x2;
				int y3 = y1;
				//计算邻边长
				double neck_edge = sqrt((x2 - x1) *(x2 - x1) + (y3 - y1) * (y3 - y1));
				//计算cos角度
				double cos = neck_edge / bevel_edge;
				//theta = -1.0 * acos(cos) / 3.14 * 180.0f;
				if (y2 < y1) {
					//cout << "yes" << endl;
					theta = -1.0 * acos(cos) / 3.14 * 180.0f;
					//cout << "theta::" << theta << endl;
				}
				else {
					//cout << "no" << endl;
					theta = acos(cos) / 3.14 * 180.0f;
					//cout << "theta::" << theta << endl;
				}
				++it2;
			}
			if (theta != 0) {
				//std::cout << "theta::" << theta << std::endl;
				cv::Size src_sz = image.size();
				cv::Size dst_sz(src_sz.width + 20, src_sz.height);
				int len = std::max(image.cols, image.rows);
				//指定旋转中心
				cv::Point2f center((float)len / 2.0f, (float)len / 2.0f);
				//获取旋转矩阵（2x3矩阵）
				cv::Mat rot_mat = cv::getRotationMatrix2D(center, theta, 1.0);
				//根据旋转矩阵进行仿射变换
				cv::warpAffine(clone_image, image, rot_mat, dst_sz);
			}
		}
		return image;
	};
	
	//7.加载xml文件
	void load() {
		if(_access(_trained_file.c_str(),0)!=-1)
			svm->load(_trained_file);
		
	};
	//8.生成xml文件
	void save() {
		svm->save(_trained_file);
	};

	//设置参数
	void setparams() {
		svm->setType(SVM::C_SVC);
		svm->setC(0.01);
		svm->setKernel(SVM::LINEAR);
		svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 3000, 1e-6));
	};

};
//训练是否是车牌类
class plate_lience:public plate_common
{
private:
	//正样本
	std::vector<cv::Mat> positives;
	//负样本
	std::vector<cv::Mat> negatives;
	int load_train_is = 0;//判断是否加载过xml
public:
	plate_lience()
	{
		_strfolder = "plate_reco/train_is";
		_trained_file = "plate_reco/PLATE_SAMPLE.xml";
	}
	
	//是否是车牌
	float predict_license(cv::Mat file) {
		if (load_train_is == 0) {
			svm = Algorithm::load<ml::SVM>(_trained_file);
			load_train_is = 1;
		}
		
		//创建
		cv::HOGDescriptor hogDesc(file.size(), // size of the window
			cv::Size(8, 8),    // block size
			cv::Size(4, 4),    // block stride
			cv::Size(4, 4),    // cell size
			9);                // number of bins
							   //第一个正样本图的描述子
		std::vector<float> desc;
		hogDesc.compute(file, desc, Size(8, 8));
		//预测
		return svm->predict(desc);
	};
	//11.读文件夹下的文件构造训练数据（车牌训练）
	void func_get_is_files()
	{
		listfolder(_strfolder, _list_main);
		list<_folder>::iterator iter;
		iter = _list_main.folders.begin();
		int ret = 0;
		while (iter != _list_main.folders.end())
		{
			//cout << iter->Folder_Name << endl;
			list<string>::iterator iter1;
			iter1 = iter->files.begin();
			while (iter1 != iter->files.end())
			{
				//cout << ret - 1 << "  ---" << (*iter1) << endl;
				//构造训练集
				if (iter->Folder_Name == "positive") {
					positives.push_back(cv::imread(*iter1, cv::IMREAD_GRAYSCALE));
				}
				else if(iter->Folder_Name == "negative") {
					negatives.push_back(cv::imread(*iter1, cv::IMREAD_GRAYSCALE));
				}
				iter1++;
			};
			iter++;
			ret++;
		}
		_list_main.clear();
	};
	//9.车牌框训练
	void train_is() {
		setparams();
		func_get_is_files();
		cv::HOGDescriptor hogDesc(positives[0].size(), // size of the window
			cv::Size(8, 8),    // block size
			cv::Size(4, 4),    // block stride
			cv::Size(4, 4),    // cell size
			9);
		std::vector<float> desc;
		
		//计算
		hogDesc.compute(positives[0], desc);
		//样本描述子矩阵
		int featureSize = desc.size();//HOG描述子个数
		int numberOfSamples = positives.size() + negatives.size();//正负样本的个数
		//创建存储样本HOG的矩阵
		cv::Mat samples(numberOfSamples, featureSize, CV_32FC1);
		//用第一个描述子填第一行
		for (int i = 0; i < featureSize; i++) {
			samples.ptr<float>(0)[i] = desc[i];
		}
		//计算正样本的描述子
		for (int j = 1; j < positives.size(); j++) {
			hogDesc.compute(positives[j], desc);
			//用当前描述子填下一行
			for (int i = 0; i < featureSize; i++)
				samples.ptr<float>(j)[i] = desc[i];
		}
		//计算负样本描述子
		for (int j = 0; j < negatives.size(); j++) {
			hogDesc.compute(negatives[j], desc);
			//用当前描述子填下一行
			for (int i = 0; i < featureSize; i++)
				samples.ptr<float>(j + positives.size())[i] = desc[i];
		}
		//创建标签
		cv::Mat labels(numberOfSamples, 1, CV_32SC1);
		//正样本的标签
		labels.rowRange(0, positives.size()) = 1.0;
		//负样本的标签
		labels.rowRange(positives.size(), numberOfSamples) = -1.0;
		svm->train(samples, ROW_SAMPLE, labels);
	}
};

//训练车牌内容类
class plate_content:public plate_common
{

private:

	//HOG检测器，用来计算HOG描述子的
	int DescriptorDim;//HOG描述子的维数，由图片大小、检测窗口大小、块大小、细胞单元中直方图bin个数决定
	cv::Mat _image;//所有训练样本的特征向量组成的矩阵，行数等于所有样本的个数，列数等于HOG描述子维数
	cv::Mat _label;//训练样本的类别向量，行数等于所有样本的个数，列数等于1；1表示有人，-1表示无人
	/*	1.定义对应训练模型的 label name*/
	string plate_result[41] = {
		"0","1","2","3","4","5","6","7","8","9",
		"A","B","C","D","E","F","G","H","J","K","L","M","N","P",
		"Q","R","S","T","U","V","W","X","Y","Z",
		"京","沪","浙","粤","苏","闽","陕"
	};
	int load_plate_content = 0;//判断是否加载过xml
public:

	plate_content()
	{
		_strfolder = "plate_reco/train_content";
		_trained_file = "plate_reco/PLATE_CONTENT.xml";
		if (load_plate_content == 0) {
			load();
			load_plate_content = 1;
		};
	};

	//找label对应的名称
	string label_name(int label) {
		return plate_result[label];
	}
	
	/*
	content:车牌OCR 分割
	in:CV::Mat
	out:CV::Mat
	*/
	cv::Mat OCR_split(cv::Mat image, std::vector<mat_rect> &sormatrect) {

		std::vector<cv::Mat> _Images;
		//阈值化
		vector<pair<Mat, Rect>> output;
		Mat img_threshold;
		cv::threshold(image, img_threshold, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		vector<vector<Point>> cons;
		//从二值图像检索轮廓
		cv::findContours(img_threshold, cons, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		Mat result;
		img_threshold.copyTo(result);
		cvtColor(result, result, CV_GRAY2RGB);
		vector<vector<Point>>::iterator itc = cons.begin();
		while (itc != cons.end())
		{
			Rect mr = boundingRect(Mat(*(itc)));
			if (mr.width / mr.height < 1 && mr.width > 5 && mr.width <= 25 && mr.width * mr.height > 15) {
				mat_rect mat_obj;
				mat_obj.rect = mr;
				sormatrect.push_back(mat_obj);
			}
			itc++;
		}
		//排序
		std::sort(sormatrect.begin(), sormatrect.end(), funCmpByValue);
		return result;
	};
	//11.读文件夹下的文件构造训练数据（车牌内容训练）
	void func_get_content_files()
	{
		listfolder(_strfolder, _list_main);
		list<_folder>::iterator iter;
		iter = _list_main.folders.begin();
		int ret = 0;
		int num = 0;//记录图片个数
		while (iter != _list_main.folders.end())
		{
			list<string>::iterator iter1;
			iter1 = iter->files.begin();
			while (iter1 != iter->files.end())
			{
				//cout << ret << "  ---" << (*iter1) << endl;
				//构造训练集
				cv::Mat image = imread((*iter1), 0);
				vector<float> descriptors;//HOG描述子向量
				//hog 返回描述子
				HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
				hog.compute(image, descriptors, Size(8, 8));
				if (num == 0) {//第一张图
					DescriptorDim = descriptors.size();
					//初始化所有训练样本的特征向量组成的矩阵，行数等于所有样本的个数，列数等于HOG描述子维数sampleFeatureMat
					_image = Mat::zeros(7000, DescriptorDim, CV_32FC1);
					//初始化训练样本的类别向量，行数等于所有样本的个数，列数等于1；1表示有人，0表示无人
					_label = Mat::zeros(7000, 1, CV_32SC1);//sampleLabelMat的数据类型必须为有符号整数型
				}
				//将计算好的HOG描述子复制到样本特征矩阵sampleFeatureMat
				for (int i = 0; i < DescriptorDim; i++) {
					//cout << "descriptors[i]" << descriptors[i] << endl;
					_image.at<float>(num, i) = descriptors[i];//第num个样本的特征向量中的第i个元素
					_label.at<int>(num, 0) = ret;//正样本类别为1，有人
				}
				iter1++;
				num++;
			}
			iter++;
			ret++;
		}
		_list_main.clear();
	};
	//9.车牌内容训练
	void train_content() {
		setparams();
		func_get_content_files();
		svm->train(_image, ROW_SAMPLE, _label);
	};
	
	//10.识别车牌内容
	float predict(cv::Mat &image, int width, int height) {
		
		vector<float> descriptors;//HOG描述子向量
		//hog 返回描述子
		HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
		hog.compute(image, descriptors, Size(8, 8));

		return svm->predict(descriptors);
	};

};
