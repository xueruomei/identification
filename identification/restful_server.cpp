/*

 */
#include "stdafx.h"
#include "restful_server.h"
#include "../HttpServer/mongoose.h"
 //
static struct mg_serve_http_opts s_http_server_opts;


void Service_HTTP::WebSocket_Broadcast(uint8_t * data, int len)
{
	struct mg_connection *c;
	if (_nc != NULL) {
		for (c = mg_next(_nc->mgr, NULL); c != NULL; c = mg_next(_nc->mgr, c)) {
			if (c == _nc) continue; /* Don't send to the sender. */
			mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, data, len);
		}
	}
}

void Service_HTTP::WebSocket_Broadcast1(void * pUser, uint8_t * data, int len)
{
	struct mg_connection *c;
	if (pUser == NULL)
		return;

	if (_nc != NULL) {
		for (c = mg_next(_nc->mgr, NULL); c != NULL; c = mg_next(_nc->mgr, c)) {
			if (c == pUser)
				mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, data, len);
		}
	}

}


void Service_HTTP::handle_api2(string uri, struct mg_connection *nc, struct http_message *hm)
{
	Service_HTTP *sh = (Service_HTTP*)nc->user_data;
	if (sh == NULL)
		return;
	handler_map& hmap = sh->GetHandleMap();
	handler_map::iterator iter;
	iter = hmap.find(uri);
	if (iter == hmap.end())
	{
		mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
		return;
	}
	route_request & rr = iter->second;
	const char * param = rr._param.GetFirstParam();
	while (param != NULL)
	{
		char buffer[256];
		mg_get_http_var(&hm->query_string, param, buffer, sizeof(buffer));
		rr._param.SetParamValue(&buffer[0]);
		//rr._param.setParam(string(param), string(buffer));
		param = rr._param.GetNextParam();
	}
	

	/* Send headers */
	//mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n");
	mg_send_response_line(nc, 200, "Access-Control-Allow-Origin: *");
	//允许哪些url可以跨域请求到本域
	mg_printf(nc, "Access-Control-Allow-Origin:*\r\n");
	//允许的请求方法，一般是GET,POST,PUT,DELETE,OPTIONS
	mg_printf(nc, "Access-Control-Allow-Methods:POST\r\n");
	//允许哪些请求头可以跨域
	mg_printf(nc, "Access-Control-Allow-Headers:x-requested-with,content-type\r\n");
	//if (reply.empty())
	string	reply = "{\"ret\":0}";
	mg_printf(nc, "Content-Length:%u\r\n\r\n%s\r\n", (uint32_t)reply.size(), reply.c_str());
	//mg_printf(nc, "HTTP/1.1 200 OK\r\niConnection: close\r\nContent-Type: text/html\r\nContent-Length: %u\r\n\r\n%s\r\n",
	nc->flags |= MG_F_SEND_AND_CLOSE;

	//mg_printf_http_chunk(nc, "{ \"result\": %ld }", 0);
	//mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */
	//string reply;
	if (rr.func != nullptr)
	{
		rr.func(rr._param);
	}
}
#if 0
void Service_HTTP::handle_api(string uri, struct mg_connection *nc, struct http_message *hm)
{
	Service_HTTP *sh = (Service_HTTP*)nc->user_data;
	if (sh == NULL)
		return;

	//const char * param = sh->_param.GetFirstParam();
	if (uri.compare("/start") == 0)
	{
		char host[64];
		char uid[64];
		char message[64];
		mg_get_http_var(&hm->query_string, "serverhost", host, sizeof(host));
		mg_get_http_var(&hm->query_string, "uid", uid, sizeof(uid));
		mg_get_http_var(&hm->query_string, "message", message, sizeof(message));
		/* Send headers */
		//mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n");
		mg_send_response_line(nc, 200, "Access-Control-Allow-Origin: *");
		//允许哪些url可以跨域请求到本域
		mg_printf(nc, "Access-Control-Allow-Origin:*\r\n");
		//允许的请求方法，一般是GET,POST,PUT,DELETE,OPTIONS
		mg_printf(nc, "Access-Control-Allow-Methods:POST\r\n");
		//允许哪些请求头可以跨域
		mg_printf(nc, "Access-Control-Allow-Headers:x-requested-with,content-type\r\n");
		string reply = "{\"result\":0}";
		mg_printf(nc, "Content-Length:%u\r\n\r\n%s\r\n", (uint32_t)reply.size(), reply.c_str());
		//mg_printf(nc, "HTTP/1.1 200 OK\r\niConnection: close\r\nContent-Type: text/html\r\nContent-Length: %u\r\n\r\n%s\r\n",
		nc->flags |= MG_F_SEND_AND_CLOSE;
		mg_send(nc, "", 0);
		//mg_printf_http_chunk(nc, "{ \"result\": %ld }", 0);
		//mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */

		
		re_param request;
		request.uri = uri;
		request.pUser = NULL;
		request.host = host;
		request.uid = uid;
		request.message = message;
		if (sh->handlestart != nullptr)
			sh->handlestart(request);
		//it->second(request);
		OutputDebugString(L"start video to");
		//callback(__void, host, uid, message);
		
	}
	else if (uri.compare("/stop") == 0) //停止
	{
		char message[64];

		mg_get_http_var(&hm->query_string, "message", message, sizeof(message));
		/* Send headers */
		//mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n");
		mg_send_response_line(nc, 200, "Access-Control-Allow-Origin: *");
		//允许哪些url可以跨域请求到本域
		mg_printf(nc, "Access-Control-Allow-Origin:*\r\n");
		//允许的请求方法，一般是GET,POST,PUT,DELETE,OPTIONS
		mg_printf(nc, "Access-Control-Allow-Methods:POST\r\n");
		//允许哪些请求头可以跨域
		mg_printf(nc, "Access-Control-Allow-Headers:x-requested-with,content-type\r\n");
		string reply = "{\"result\":0}";
		mg_printf(nc, "Content-Length:%u\r\n\r\n%s\r\n", (uint32_t)reply.size(), reply.c_str());
		//mg_printf(nc, "HTTP/1.1 200 OK\r\niConnection: close\r\nContent-Type: text/html\r\nContent-Length: %u\r\n\r\n%s\r\n",
		nc->flags |= MG_F_SEND_AND_CLOSE;

		mg_send(nc, "", 0);
		//mg_printf(nc, "{ \"result\": %ld }", 0);
		//mg_send(nc, "", 0);
		//mg_printf_http_chunk(nc, "{ \"result\": %ld }", 0);
		//mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */

		
			//auto it = sh->GetHandleMap().find(uri);
			//if (sh->GetHandleMap().end() == it)
			//	return;
			re_param request;
			request.uri = uri;
			request.pUser = NULL;
			request.message = message;
			if(sh->handlestop!=nullptr)
				sh->handlestop(request);
			OutputDebugString(L"stop video");
	}
	//获取
}
#endif



void Service_HTTP::ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
	struct http_message *hm = (struct http_message *) ev_data;
	//传送文件地址和转化的目的地址
	//   /api/t2pdf
	switch (ev) {

	case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
		OutputDebugString("connection is here");
		/* New websocket connection. Tell everybody. */
		//broadcast(nc, mg_mk_str("++ joined"));
		break;

		//websocket frame
	case MG_EV_WEBSOCKET_FRAME: {
		struct websocket_message *wm = (struct websocket_message *) ev_data;
		/* New websocket message. Tell everybody. */
		struct mg_str d = { (char *)wm->data, wm->size };
		//broadcast(nc, d);
		break;
	}

	case MG_EV_HTTP_REQUEST:
	{
		string uri = string(hm->uri.p, hm->uri.len);
		handle_api2(uri, nc, hm);
		//if (mg_vcmp(&hm->uri, "/start") == 0) {
		//	handle_api2(uri, nc, hm);
		//}
		//else if (mg_vcmp(&hm->uri, "/stop") == 0) {
		//	handle_api2(uri, nc, hm);
		//}
		//else
		//{
		//	mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
		//}
		break;
	}
	case MG_EV_CLOSE:
	{
		if (is_websocket(nc))
			OutputDebugString("websocket out");
		break;
	}
	default:
		break;
	}
}


void Service_HTTP::Stop()
{
	TThreadRunable::Stop();
	Join();
}


void Service_HTTP::Run()
{
	mg_mgr_init(&_mgr, NULL);
	_nc = mg_bind(&_mgr, s_http_port, ev_handler);
	_nc->user_data = this;
	mg_set_protocol_http_websocket(_nc);
	s_http_server_opts.document_root = "./public";  // Serve current directory
	s_http_server_opts.enable_directory_listing = "yes";
	//printf("Started on port %s\n", s_http_port);
	while (!IsStop()) {
		mg_mgr_poll(&_mgr, 100);
	}
	mg_mgr_free(&_mgr);

}