#include "stdafx.h"
#include "HttpServerRun.h"
#include "recognition.h"

void RunHttp::Start(void * pUser)
{
	_pWrapper = pUser;
	TThreadRunable::Start();
}

void RunHttp::Run()
{
	if (_pWrapper == NULL)
		return;
	httpserver("5210", this, [](void * pUser, const char *cmd, const char *name, const char *path)
	{
		RunHttp* runhttp = (RunHttp *)pUser;
		if (runhttp->IsStop())
			return;
		http_receive * wrap = (http_receive*)runhttp->_pWrapper;
		wrap->httpCommand(cmd, name , path);
	});
}