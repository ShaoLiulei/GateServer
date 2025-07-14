#pragma once
#include "const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;

class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem() {}
	bool HandleGet(std::string path, std::shared_ptr<HttpConnection> con);  // 处理Get请求
	bool HandlePost(std::string path, std::shared_ptr<HttpConnection> con);  // 处理Post请求
	void RegGet(std::string, HttpHandler handler);  // 注册处理Get请求的handler
	void RegPost(std::string url, HttpHandler handler);  // 注册处理Post请求的handler
private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;
};