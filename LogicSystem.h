#pragma once
#include "const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;

class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem() {}
	bool HandleGet(std::string path, std::shared_ptr<HttpConnection> con);  // ����Get����
	bool HandlePost(std::string path, std::shared_ptr<HttpConnection> con);  // ����Post����
	void RegGet(std::string, HttpHandler handler);  // ע�ᴦ��Get�����handler
	void RegPost(std::string url, HttpHandler handler);  // ע�ᴦ��Post�����handler
private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;
};