//入口
#include "stdafx.h"
#include "public.h"
#include "recognition.h"
#include "filesystem.h"
#include "face.h"
#include "restful_server.h"
#include "httpclient.h"

#ifdef DEBUG
#pragma comment(lib,"opencv_world343d.lib")
#endif // RELEASE
#pragma comment(lib,"opencv_world343.lib")

//接口层
int main(int argc, char * argv[])
{

	//http_connect hc;
	//hc.postData("192.168.1.239", 5200, "/api/ai", "{\"ret\":0}");
	//return 0;
	
	Service_HTTP runhttp;
	http_receive hr;//接收吗，命令
	hr.Train_Init(&runhttp);//初始化 开始训练
	hr.StartAllThread(); //启动线程
	runhttp.Start();
	runhttp.Join(); //等待下一个
	hr.StopAllThread();//关闭线程
};
