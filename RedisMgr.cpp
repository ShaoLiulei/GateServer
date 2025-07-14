#include "RedisMgr.h"
#include "ConfigMgr.h"


RedisMgr::RedisMgr() {
	auto& gCfgMgr = ConfigMgr::Inst();
	auto host = gCfgMgr["Redis"]["host"];
	auto port = gCfgMgr["Redis"]["port"];
	auto passwd = gCfgMgr["Redis"]["passwd"];
	_con_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), passwd.c_str()));
}
RedisMgr::~RedisMgr() {
	Close();

}
// 连接redis // 已经封装到连接池中，不再需要
//bool RedisMgr::Connect(const std::string& host, int port) {
//	connect = redisConnect(host.c_str(), port);  // 连接成功时connect->err=0
//	if (connect == nullptr) {
//		std::cout << "connect error " << std::endl;
//		return false;
//	}
//	if (connect != NULL && connect->err) {
//		std::cout << "connect error " << connect->err << std::endl;
//		return false;
//	}
//	return true;
//}
// 获取key对应的value
bool RedisMgr::Get(const std::string& key, std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
	if (reply == NULL) {
		std::cout << "[ GET " << key << " ] failed" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	if (reply->type != REDIS_REPLY_STRING) {
		std::cout << "[ GET " << key << " ] failed" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}

	value = reply->str;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	std::cout << "Succeed to execute command [ GET " << key << " ]" << std::endl;
	return true;
}
// 设置key和value
bool RedisMgr::Set(const std::string& key, const std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	// 执行redis命令
	auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
	// 返回NULL说明执行失败
	if (reply == NULL) {
		std::cout << "Failed to execute command [ SET " << key << " " << value << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	// 如果执行失败则释放连接
	if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 ||
		strcmp(reply->str, "ok") == 0))) {
		std::cout << "Failed to execute command [ SET " << key << " " << value << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	// 执行成功释放连接
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	std::cout << "Succeed to execute command [ SET " << key << " " << value << " ]" << std::endl;
	return true;
}
// 密码认证
//bool RedisMgr::Auth(const std::string& passwd)
//{
//	auto connect = _con_pool->getConnection();
//  if (connect == nullptr) { return false; }
//	auto reply = (redisReply*)redisCommand(connect, "AUTH %s", passwd.c_str());
//	if (reply->type == REDIS_REPLY_ERROR) {
//		std::cout << "认证失败" << std::endl;
//		freeReplyObject(reply);
//		_con_pool->returnConnection(connect);
//		return false;
//	}
//	std::cout << "认证成功" << std::endl;
//	freeReplyObject(reply);
//	_con_pool->returnConnection(connect);
//	return true;
//}
// 左侧push
bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	auto reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
	if (reply == NULL) {
		std::cout << "Failed to execute command [ LPUSH " << key << " " << value << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
		std::cout << "Failed to execute command [ LPUSH " << key << " " << value << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	std::cout << "Succeed to execute command [ LPUSH " << key << " " << value << " ]" << std::endl;
	return true;
}
// 左侧pop
bool RedisMgr::LPop(const std::string& key, std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	auto reply = (redisReply*)redisCommand(connect, "LPOP %s", key.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
		std::cout << "Failed to execute command [ LPOP " << key << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	value = reply->str;
	std::cout << "Succeed to execute command [ LPOP " << key << " ]" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}
// 右侧push
bool RedisMgr::RPush(const std::string& key, const std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	auto reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
	if (reply == NULL) {
		std::cout << "Failed to execute command [ RPUSH " << key << " " << value << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
		std::cout << "Failed to execute command [ RPUSH " << key << " " << value << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	std::cout << "Succeed to execute command [ RPUSH " << key << " " << value << " ]" << std::endl;
	return true;
}
// 右侧pop
bool RedisMgr::RPop(const std::string& key, std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	auto reply = (redisReply*)redisCommand(connect, "RPOP %s", key.c_str());
	if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
		std::cout << "Failed to execute command [ RPOP " << key << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	value = reply->str;
	std::cout << "Succeed to execute command [ RPOP " << key << " ]" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}
// HSet操作
bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Failed to execute command [ HSET " << key << " " << hkey << " " << value << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Succeed to execute command [ HSET " << key << " " << hkey << " " << value << " ]" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}

bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	const char* argv[4];
	size_t argvlen[4];
	argv[0] = "HSET";
	argvlen[0] = 4;
	argv[1] = key;
	argvlen[1] = strlen(key);
	argv[2] = hkey;
	argvlen[2] = strlen(hkey);
	argv[3] = hvalue;
	argvlen[3] = hvaluelen;
	auto reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Failed to execute command [ HSET " << key << " " << hkey << " " << hvalue << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Succeed to execute command [ HSET " << key << " " << hkey << " " << hvalue << " ]" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}
// HGet操作
std::string RedisMgr::HGet(const std::string& key, const std::string& hkey)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return ""; }
	const char* argv[3];
	size_t argvlen[3];
	argv[0] = "HGET";
	argvlen[0] = 4;
	argv[1] = key.c_str();
	argvlen[1] = key.length();
	argv[2] = hkey.c_str();
	argvlen[2] = hkey.length();
	auto reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);
	if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
		std::cout << "Failed to execute command [ HGET " << key << " " << hkey << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return "";
	}
	std::string value = reply->str;
	std::cout << "Succeed to execute command [ HGET " << key << " " << hkey << " ]" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return value;
}
// 删除Del
bool RedisMgr::Del(const std::string& key)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	auto reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Failed to execute command [ DEL " << key << " ]" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Succeed to execute command [ DEL " << key << " ]" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}
// 查找是否存在exists
bool RedisMgr::ExistsKey(const std::string& key)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) { return false; }
	auto reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());
	if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
		std::cout << "Not found [ key " << key << " ] !" << std::endl;
		freeReplyObject(reply);
		_con_pool->returnConnection(connect);
		return false;
	}
	std::cout << "Found [ key " << key << " ] !" << std::endl;
	freeReplyObject(reply);
	_con_pool->returnConnection(connect);
	return true;
}

void RedisMgr::Close() {
	_con_pool->Close();
}

