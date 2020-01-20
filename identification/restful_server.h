#ifndef _RESTFUL_SERVER_H_
#define _RESTFUL_SERVER_H_
#define HAVE_STRUCT_TIMESPEC


/*
使用

void CMeetingDlg::StartHttpCallback(request_param &rp)
{
       
}
handler hd1 = std::bind(&CMeetingDlg::StartHttpCallback, this, std::placeholders::_1);
_httpserver.RegisterHandler("/start", hd1);
*/

#include "../HttpServer/mongoose.h"
#include "TThreadRunable.h"
#include <string>
#include <functional>
#include <map>
using namespace std;

typedef struct request_param
{
	map<string, string> params;
	void insertParam(string key)
	{
		params[key] = "null";
	}

	string getParam(string key)
	{
		string ret;
		if (params.find(key) != params.end())
		{
			ret = params[key];
		}
		return ret;
	}
	int getSize()
	{
		return (int)params.size();
	}
	map<string, string>::iterator iter;
	const char * GetFirstParam()
	{
		iter = params.begin();
		if (iter == params.end())
			return NULL;
		return iter->first.c_str();
	}
	void SetParamValue(const char *value)
	{
		if (iter != params.end())
			iter->second = value;
	}

	const char * GetNextParam()
	{
		iter++;
		if (iter == params.end())
		{
			return NULL;
		}
		else
		{
			return iter->first.c_str();
		}
	}
}request_param;

using handler2 = std::function<void(request_param &requestparam)>;
typedef struct route_request
{
	request_param _param;
	handler2 func;
}route_request;


class Service_HTTP :public TThreadRunable
{
protected:
	using handler_map = std::map<std::string, route_request>;
	handler_map _handlers;
public:
    
	handler_map &GetHandleMap()
	{
		return _handlers;
	}

	char *s_http_port = "5210";
	struct mg_serve_http_opts _s_http_server_opts;
	struct mg_mgr _mgr;
	struct mg_connection * _nc = NULL;

	bool RegisterHandler(std::string uri, route_request f) {
		auto it = _handlers.find(uri);
		if (_handlers.end() != it)
			return false;

		return _handlers.emplace(uri, f).second;
	}


	void UnRegisterHandler(std::string uri) {
		auto it = _handlers.find(uri);
		if (_handlers.end() != it)
			_handlers.erase(it);
	}

public:

	static void handle_api2(string uri, struct mg_connection *nc, struct http_message *hm);

	static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);
	static int is_websocket(const struct mg_connection *nc) {
		return nc->flags & MG_F_IS_WEBSOCKET;
	}

	
//广播该视频数据
	void WebSocket_Broadcast(uint8_t * data, int len);
	void WebSocket_Broadcast1(void * pUser, uint8_t * data, int len);
public:

	void Stop();
	void Run();
};
#endif
