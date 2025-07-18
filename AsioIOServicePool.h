#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"


class AsioIOServicePool:public Singleton<AsioIOServicePool>
{
	friend class Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<IOService::executor_type>;
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	// 使用round-robin的方式返回一个io_service
	boost::asio::io_context& GetIOService();
	void Stop();

private:
	AsioIOServicePool(std::size_t size = 2/*默认两个线程, 也可用std::thread::hardware_concurrency()*/);
	std::vector<IOService> _ioServices;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t _nextIOService;
};

