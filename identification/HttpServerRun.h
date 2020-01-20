#pragma once

#include "TThreadRunable.h"
#include "../HttpServer/restful_server.h"



class RunHttp :public TThreadRunable
{
public:
	//用户指针存放
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
