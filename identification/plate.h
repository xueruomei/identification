#pragma once
#include "stdafx.h"
#include "linefinder.h"//����
#include "edgedetector.h"
#include "public.h"
#include "filesystem.h"

//�Ƚϴ�С
static bool funCmpByValue(const mat_rect& a, const mat_rect& b)
{
	return a.rect.x < b.rect.x;
}

//����
class plate_common 
{
private:
	
	
 protected:
	 string _trained_file;
	 string _strfolder;
	 Ptr<SVM> svm = SVM::create();//SVM������
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
	
	//�ּ����㷨
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

	//Ԥ�������
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

	//��������
	static cv::Mat adjust(cv::Mat image) {
		return rotate(image);
	};

	//���Ʊ任 ������Ƕ� ����ת
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

				//����Ƕ�
				//1.0����߳� �Ա� б��
				// ����б�߳�
				double bevel_edge = sqrt((x2 - x1) *(x2 - x1) + (y2 - y1) * (y2 - y1));
				//���ڱ�
				cv::line(image, cv::Point(x1, y1), cv::Point(x2, y1), cv::Scalar(255, 255, 255), 1);
				//����ֱ�ߵ��յ�
				int x3 = x2;
				int y3 = y1;
				//�����ڱ߳�
				double neck_edge = sqrt((x2 - x1) *(x2 - x1) + (y3 - y1) * (y3 - y1));
				//����cos�Ƕ�
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
				//ָ����ת����
				cv::Point2f center((float)len / 2.0f, (float)len / 2.0f);
				//��ȡ��ת����2x3����
				cv::Mat rot_mat = cv::getRotationMatrix2D(center, theta, 1.0);
				//������ת������з���任
				cv::warpAffine(clone_image, image, rot_mat, dst_sz);
			}
		}
		return image;
	};
	
	//7.����xml�ļ�
	void load() {
		if(_access(_trained_file.c_str(),0)!=-1)
			svm->load(_trained_file);
		
	};
	//8.����xml�ļ�
	void save() {
		svm->save(_trained_file);
	};

	//���ò���
	void setparams() {
		svm->setType(SVM::C_SVC);
		svm->setC(0.01);
		svm->setKernel(SVM::LINEAR);
		svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 3000, 1e-6));
	};

};
//ѵ���Ƿ��ǳ�����
class plate_lience:public plate_common
{
private:
	//������
	std::vector<cv::Mat> positives;
	//������
	std::vector<cv::Mat> negatives;
	int load_train_is = 0;//�ж��Ƿ���ع�xml
public:
	plate_lience()
	{
		_strfolder = "plate_reco/train_is";
		_trained_file = "plate_reco/PLATE_SAMPLE.xml";
	}
	
	//�Ƿ��ǳ���
	float predict_license(cv::Mat file) {
		if (load_train_is == 0) {
			svm = Algorithm::load<ml::SVM>(_trained_file);
			load_train_is = 1;
		}
		
		//����
		cv::HOGDescriptor hogDesc(file.size(), // size of the window
			cv::Size(8, 8),    // block size
			cv::Size(4, 4),    // block stride
			cv::Size(4, 4),    // cell size
			9);                // number of bins
							   //��һ��������ͼ��������
		std::vector<float> desc;
		hogDesc.compute(file, desc, Size(8, 8));
		//Ԥ��
		return svm->predict(desc);
	};
	//11.���ļ����µ��ļ�����ѵ�����ݣ�����ѵ����
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
				//����ѵ����
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
	//9.���ƿ�ѵ��
	void train_is() {
		setparams();
		func_get_is_files();
		cv::HOGDescriptor hogDesc(positives[0].size(), // size of the window
			cv::Size(8, 8),    // block size
			cv::Size(4, 4),    // block stride
			cv::Size(4, 4),    // cell size
			9);
		std::vector<float> desc;
		
		//����
		hogDesc.compute(positives[0], desc);
		//���������Ӿ���
		int featureSize = desc.size();//HOG�����Ӹ���
		int numberOfSamples = positives.size() + negatives.size();//���������ĸ���
		//�����洢����HOG�ľ���
		cv::Mat samples(numberOfSamples, featureSize, CV_32FC1);
		//�õ�һ�����������һ��
		for (int i = 0; i < featureSize; i++) {
			samples.ptr<float>(0)[i] = desc[i];
		}
		//������������������
		for (int j = 1; j < positives.size(); j++) {
			hogDesc.compute(positives[j], desc);
			//�õ�ǰ����������һ��
			for (int i = 0; i < featureSize; i++)
				samples.ptr<float>(j)[i] = desc[i];
		}
		//���㸺����������
		for (int j = 0; j < negatives.size(); j++) {
			hogDesc.compute(negatives[j], desc);
			//�õ�ǰ����������һ��
			for (int i = 0; i < featureSize; i++)
				samples.ptr<float>(j + positives.size())[i] = desc[i];
		}
		//������ǩ
		cv::Mat labels(numberOfSamples, 1, CV_32SC1);
		//�������ı�ǩ
		labels.rowRange(0, positives.size()) = 1.0;
		//�������ı�ǩ
		labels.rowRange(positives.size(), numberOfSamples) = -1.0;
		svm->train(samples, ROW_SAMPLE, labels);
	}
};

//ѵ������������
class plate_content:public plate_common
{

private:

	//HOG���������������HOG�����ӵ�
	int DescriptorDim;//HOG�����ӵ�ά������ͼƬ��С����ⴰ�ڴ�С�����С��ϸ����Ԫ��ֱ��ͼbin��������
	cv::Mat _image;//����ѵ������������������ɵľ��������������������ĸ�������������HOG������ά��
	cv::Mat _label;//ѵ����������������������������������ĸ�������������1��1��ʾ���ˣ�-1��ʾ����
	/*	1.�����Ӧѵ��ģ�͵� label name*/
	string plate_result[41] = {
		"0","1","2","3","4","5","6","7","8","9",
		"A","B","C","D","E","F","G","H","J","K","L","M","N","P",
		"Q","R","S","T","U","V","W","X","Y","Z",
		"��","��","��","��","��","��","��"
	};
	int load_plate_content = 0;//�ж��Ƿ���ع�xml
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

	//��label��Ӧ������
	string label_name(int label) {
		return plate_result[label];
	}
	
	/*
	content:����OCR �ָ�
	in:CV::Mat
	out:CV::Mat
	*/
	cv::Mat OCR_split(cv::Mat image, std::vector<mat_rect> &sormatrect) {

		std::vector<cv::Mat> _Images;
		//��ֵ��
		vector<pair<Mat, Rect>> output;
		Mat img_threshold;
		cv::threshold(image, img_threshold, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		vector<vector<Point>> cons;
		//�Ӷ�ֵͼ���������
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
		//����
		std::sort(sormatrect.begin(), sormatrect.end(), funCmpByValue);
		return result;
	};
	//11.���ļ����µ��ļ�����ѵ�����ݣ���������ѵ����
	void func_get_content_files()
	{
		listfolder(_strfolder, _list_main);
		list<_folder>::iterator iter;
		iter = _list_main.folders.begin();
		int ret = 0;
		int num = 0;//��¼ͼƬ����
		while (iter != _list_main.folders.end())
		{
			list<string>::iterator iter1;
			iter1 = iter->files.begin();
			while (iter1 != iter->files.end())
			{
				//cout << ret << "  ---" << (*iter1) << endl;
				//����ѵ����
				cv::Mat image = imread((*iter1), 0);
				vector<float> descriptors;//HOG����������
				//hog ����������
				HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
				hog.compute(image, descriptors, Size(8, 8));
				if (num == 0) {//��һ��ͼ
					DescriptorDim = descriptors.size();
					//��ʼ������ѵ������������������ɵľ��������������������ĸ�������������HOG������ά��sampleFeatureMat
					_image = Mat::zeros(7000, DescriptorDim, CV_32FC1);
					//��ʼ��ѵ����������������������������������ĸ�������������1��1��ʾ���ˣ�0��ʾ����
					_label = Mat::zeros(7000, 1, CV_32SC1);//sampleLabelMat���������ͱ���Ϊ�з���������
				}
				//������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
				for (int i = 0; i < DescriptorDim; i++) {
					//cout << "descriptors[i]" << descriptors[i] << endl;
					_image.at<float>(num, i) = descriptors[i];//��num�����������������еĵ�i��Ԫ��
					_label.at<int>(num, 0) = ret;//���������Ϊ1������
				}
				iter1++;
				num++;
			}
			iter++;
			ret++;
		}
		_list_main.clear();
	};
	//9.��������ѵ��
	void train_content() {
		setparams();
		func_get_content_files();
		svm->train(_image, ROW_SAMPLE, _label);
	};
	
	//10.ʶ��������
	float predict(cv::Mat &image, int width, int height) {
		
		vector<float> descriptors;//HOG����������
		//hog ����������
		HOGDescriptor hog(Size(32, 40), Size(16, 16), Size(8, 8), Size(8, 8), 9);
		hog.compute(image, descriptors, Size(8, 8));

		return svm->predict(descriptors);
	};

};
