﻿#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"

//void TestRedisMgr() {
//	//assert(RedisMgr::GetInstance()->Connect("127.0.0.1", 6380));
//	//assert(RedisMgr::GetInstance()->Auth("123456"));
//	assert(RedisMgr::GetInstance()->Set("blogwebsite", "llfc.club"));
//	std::string value = "";
//	assert(RedisMgr::GetInstance()->Get("blogwebsite", value));
//	assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
//	assert(RedisMgr::GetInstance()->HSet("bloginfo", "blogwebsite", "llfc.club"));
//	assert(RedisMgr::GetInstance()->HGet("bloginfo", "blogwebsite") != "");
//	assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
//	assert(RedisMgr::GetInstance()->Del("bloginfo"));
//	assert(RedisMgr::GetInstance()->Del("bloginfo"));
//	assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
//	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
//	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
//	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
//	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
//	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
//	assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
//	assert(RedisMgr::GetInstance()->LPop("lpushkey2", value) == false);
//}

int main()
{
	//TestRedisMgr();  // 测试RedisMgr连接
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string gate_port_str = gCfgMgr["GateServer"]["port"];
	unsigned short gate_port = atoi(gate_port_str.c_str());
	try{
		unsigned short port = static_cast<unsigned short>(8180);
		net::io_context ioc{ 1 };
		net::signal_set signals(ioc, SIGINT, SIGTERM);  // 信号集
		signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {
			if (error) {
				return;
			}
			ioc.stop();
			}
		 );
		std::make_shared<CServer>(ioc, port)->Start();  // 启动Server
		std::cout << "GateServer listen on port: " << port << std::endl;
		ioc.run();
	}catch (std::exception const& exp) {
		std::cerr << "Error: " << exp.what() << std::endl;
		return EXIT_FAILURE;
	}
}