#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <memory>
#include <iostream>
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "hiredis.h"


#define CODE_PREFIX "code_"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
    Success = 0,
    Error_Json = 1001,  //Json��������
    RPCFailed = 1002,  //RPC�������
    VerifyExpired = 1003,  // ��֤�����
    VerifyCodeErr = 1004,  // ��֤�����
    UserExist = 1005,  // �û��Ѵ���
};

class Defer {
public:
    Defer(std::function<void()> func): func_(func) {}
    ~Defer() {
        func_();
    }
private:
    std::function<void()> func_;
};


