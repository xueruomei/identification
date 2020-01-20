#pragma once
#include "face.h"
#include "plate.h"
#include "image.h"
#include "TThreadRunable.h"
#include <queue>
#include "httpclient.h"
#include "common.h"
#include "restful_server.h"
//接口调用层的函数
//处理磁盘路径字符
class wrapper
{
	face_interface face_api;
	plate_content plate_content_api;
	plate_lience plate_lience_api;
	image_contrast image_contrast_api;
	//人脸识别公共函数
	void func_Face_common_Predict(cv::Mat origin, pre_ret &face_obj);
	//车牌识别公共函数
	void func_Plate_Common_Predict(cv::Mat image, Ptr<SVM> svm, HOGDescriptor hog, pre_ret &plate_obj);
	string _ori_folder;
public:

	void init_Interface(string trainedxmlfile,string foldername)
	{
		_ori_folder = foldername;
		face_api.init(trainedxmlfile);
		face_api.read();
	}
	//人
	void func_Face_Picture_Dectet(const char*path, const char *number, string &_str_client);
	void func_Face_Picture_Predict(const char *path, const char *number, pre_ret &face_obj, string &_str_client);
	void func_Face_Train(const char* folder_name, const char *number, string &_str_client);
	void func_Face_Update(const char *  path,const char * param, const char *number,string &_str_client);
	void func_Face_Video_Predict(const char * param, const char *number, pre_ret &face_obj, string &_str_client);
	void func_Face_Picture_comparison(const char * param, const char *number, pre_ret &face_obj, string &_str_client);
	//车
	void func_Plate_Picture_Predict(const char * path, const char * param, const char *number ,pre_ret &plate_obj, string &_str_client);
	void func_Plate_Video_Predict(const char *path, const char * param, const char *number, pre_ret &plate_obj, string &_str_client);
	void func_Plate_Picture_Content_train(const char *number, string &_str_client);
	void func_Plate_Picture_Is_train(const char *number, string &_str_client);
	//图像
	void func_Image_Contrast(const char * path, const char *number, pre_ret &image_obj, string &_str_client);
};

typedef struct http_receive_param
{
	int cmd;
	string name;
	string path;
	string number;
}http_recv_param;

class http_run_wrapper :public TThreadRunable
{
	string folder_name = "face_reco/FACE";
	string folder_train = "FACE";
	string trainedfile = "face_reco/FACE_TRAIN.xml";
private:
	std::mutex _lock;
	queue<http_recv_param*> _queue_params;
public:
	void insert(http_recv_param * param)
	{
		_lock.lock();
		_queue_params.push(param);
		_lock.unlock();
		Notify();
	}
	http_recv_param  *pop()
	{
		http_recv_param * pp = NULL;
		_lock.lock();
		if (!_queue_params.empty())
		{
			pp = _queue_params.front();
			_queue_params.pop();
		}
		_lock.unlock();
		return pp;
	}

public:
	http_run_wrapper()
	{

	}
	~http_run_wrapper()
	{

	}
	
	void Clear()
	{
		http_recv_param * param = pop();
		while (param != NULL)
		{
			delete param;
			param = pop();
		}
	}
	void DoWork(http_recv_param *param,string &_str_client)
	{
		int &_cmd = param->cmd;
		string &_path = param->path;
		string &_name = param->name;
		string &_number = param->number;
		wrapper wrap; //实例化接口层对象
		pre_ret face_obj;//实例化人脸返回值
		pre_ret plate_obj;//实例化车牌返回值
		pre_ret image_obj;//实例化车牌返回值
		wrap.init_Interface(trainedfile, folder_name);//初始化
		switch (_cmd)
		{
		case Face_Picture_Train:
			wrap.func_Face_Train(folder_name.c_str(), _number.c_str(),_str_client);
			break;
		case Face_Picture_Detect:
			wrap.func_Face_Picture_Dectet(_path.c_str(), _number.c_str(), _str_client);
			break;
		case Face_Picture_Predict:
			wrap.func_Face_Picture_Predict(_path.c_str(),_number.c_str(), face_obj, _str_client);
			break;
		case Face_Video_Predict:
			wrap.func_Face_Video_Predict(_name.c_str(), _number.c_str(), face_obj, _str_client);
			break;
		case Face_Picture_comparison:
			wrap.func_Face_Picture_comparison(_path.c_str(), _number.c_str(), face_obj, _str_client);
			break;
		case Plate_Picture_Content_train:
			wrap.func_Plate_Picture_Content_train(_number.c_str(), _str_client);
			break;
		case Plate_Picture_Is_train:
			wrap.func_Plate_Picture_Is_train(_number.c_str(), _str_client);
			break;
		case Plate_Picture_Predict:
			wrap.func_Plate_Picture_Predict(_path.c_str(),  _name.c_str(), _number.c_str(), plate_obj, _str_client);
			break;
		case Plate_Video_Predict:
			wrap.func_Plate_Video_Predict(_path.c_str(), _name.c_str(),_number.c_str(), plate_obj, _str_client);
			break;
		case Image_Picture_Contrast:
			wrap.func_Image_Contrast(_path.c_str(), _number.c_str(), image_obj, _str_client);
			break;
		};
	}
	void Run()
	{
		while (1)
		{
			if (IsStop())
				break;
			WaitForSignal();
			http_recv_param *param = pop();
			while (param != NULL)
			{
				if (IsStop()) {
					break;
				}
				string _str_client;
				DoWork(param, _str_client);
				//返回给客户端
				http_connect http_client;
				string str = GB23122Utf8(_str_client.c_str());
				//param->ip.c_str(), param->port
				http_client.postData("localhost", 5200, "/api/ai", str.c_str());
				delete param;
				param = pop();
			}
		}
		cout << "this is the end of thread" << endl;
	}
	void Stop()
	{
		TThreadRunable::Stop();
		Notify();
		Join();
		Clear();
	}
};

class http_receive
{
#define THREAD_NUM 4
	string folder_name = "face_reco/FACE";
	string trainedfile = "face_reco/FACE_TRAIN.xml";
	wrapper _wrap;
	string _str_client;
	int64_t _lastThread = -1;
	Service_HTTP * _shttpserver;
	//线程池概念
	http_run_wrapper _run_wrapper[THREAD_NUM];
public:
	void Train_Init(Service_HTTP * hs)
	{
		_shttpserver = hs;
		route_request rr;
		rr._param.insertParam("cmd");
		rr._param.insertParam("name");
		rr._param.insertParam("path");
		rr._param.insertParam("number");
		handler2 hd = std::bind(&http_receive::httpCommand, this, std::placeholders::_1);
		rr.func = hd;
		_shttpserver->RegisterHandler("/start",rr);
		_wrap.init_Interface(trainedfile, folder_name);
		_wrap.func_Face_Train(folder_name.c_str(),"-1", _str_client);
	}

	int Train_Update(string path, string name, string number, string &_post_client)
	{
		_wrap.func_Face_Update(path.c_str(), name.c_str(), number.c_str(), _post_client);
		//_post_client = _str_client;
		return 0;
	}
	//, const char * ip, const char * port
	void httpCommand(request_param& requestparam);
	
	void StartAllThread()
	{
		for (int i = 0; i < THREAD_NUM; i++)
		{
			_run_wrapper[i].Start();
		}
	}
	void StopAllThread()
	{
		for (int i = 0; i < THREAD_NUM; i++)
		{
			_run_wrapper[i].Stop();
		}
	}
	void InsertParam(http_recv_param *param)
	{
		int now = ++_lastThread % THREAD_NUM;
		_run_wrapper[now].insert(param);
	}


};