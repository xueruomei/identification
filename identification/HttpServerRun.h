#pragma once

#include "TThreadRunable.h"
#include "../HttpServer/restful_server.h"



class RunHttp :public TThreadRunable
{
public:
	//�û�ָ����
	void * _pWrapper = NULL;
public:
	void Start(void * pDlg);
	void Stop()
	{
		stopserver();
		TThreadRunable::Stop();
		Join();
	}
	void Run();
};
