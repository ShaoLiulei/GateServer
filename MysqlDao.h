#pragma once
#include "const.h"
#include <thread>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>


class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) :_con(con), _last_oper_time(lasttime) {}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};


class MysqlPool {
public:
	MysqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolsize)
		:url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolsize), b_stop_(false) {
		try {
			for (int i = 0; i < poolsize; ++i) {
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* con = driver->connect(url_, user_, pass_);
				con->setSchema(schema_);
				// 获取当前时间戳并转换为秒
				auto currentTime = std::chrono::system_clock::now().time_since_epoch();
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
				pool_.push(std::make_unique<SqlConnection>(con, timestamp));
			}
			check_thread_ = std::thread([this]() {
				while (!b_stop_) {
					checkConnection();
					std::this_thread::sleep_for(std::chrono::seconds(60));
				} });
				check_thread_.detach();
		}
		catch (sql::SQLException& exp) {
			std::cout << "mysql pool init failed, error is " << exp.what() << std::endl;
		}
	}
	// 检测连接
	void checkConnection() {
		std::lock_guard<std::mutex> guard(mutex_);
		int poolsize = pool_.size();
		auto currentTime = std::chrono::system_clock::now().time_since_epoch();
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
		for (int i = 0; i < poolsize; ++i) {
			auto con = std::move(pool_.front());
			pool_.pop();
			Defer defer([this, &con]() {
				pool_.push(std::move(con)); });
			if (timestamp - con->_last_oper_time < 5) { continue; }
			try {
				std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
				stmt->executeQuery("SELECT 1");
				con->_last_oper_time = timestamp;
				std::cout << "execute timer alive query, cur is " << timestamp << std::endl;
			}
			catch (sql::SQLException& exp) {
				std::cout << "error keep connection alive: " << exp.what() << std::endl;
				// 重新创建连接并替换
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* newcon = driver->connect(url_, user_, pass_);
				newcon->setSchema(schema_);
				con->_con.reset(newcon);
				con->_last_oper_time = timestamp;
			}
		}
	}
	// 获取、归还连接
	std::unique_ptr<SqlConnection> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		condition_.wait(lock, [this]() {  // 返回false时挂起并释放锁
			if (b_stop_) {
				return true;
			}
			return !pool_.empty(); });
		if (b_stop_) { return nullptr; }
		std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
		pool_.pop();
		return con;
	}
	void returnConnection(std::unique_ptr<SqlConnection> con) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (b_stop_) { return; }
		pool_.push(std::move(con));
		condition_.notify_one();
		return;
	}
	// 关闭连接池
	void Close() {
		b_stop_ = true;
		condition_.notify_all();
	}

	~MysqlPool() {
		std::unique_lock<std::mutex> lock(mutex_);
		while (!pool_.empty()) {
			pool_.pop();
		}
	}
private:
	std::string url_;
	std::string user_;
	std::string pass_;
	std::string schema_;
	int poolSize_;
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;
	std::condition_variable condition_;
	std::atomic<bool> b_stop_;
	std::thread check_thread_;
};


struct UserInfo {
	std::string name;
	std::string passwd;
	int uid;
	std::string email;
};


class MysqlDao
{
};

