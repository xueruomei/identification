//���
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

//�ӿڲ�
int main(int argc, char * argv[])
{

	//http_connect hc;
	//hc.postData("192.168.1.239", 5200, "/api/ai", "{\"ret\":0}");
	//return 0;
	
	Service_HTTP runhttp;
	http_receive hr;//����������
	hr.Train_Init(&runhttp);//��ʼ�� ��ʼѵ��
	hr.StartAllThread(); //�����߳�
	runhttp.Start();
	runhttp.Join(); //�ȴ���һ��
	hr.StopAllThread();//�ر��߳�
};
