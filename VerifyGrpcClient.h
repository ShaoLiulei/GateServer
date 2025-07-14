#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"


using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;


class RPConPool {
public:
	RPConPool(size_t poolsize, std::string host, std::string port) :
		poolSize_(poolsize), host_(host), port_(port), b_stop_(false) {
		for (size_t i = 0; i < poolSize_; ++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
			connections_.push(VerifyService::NewStub(channel));
		}
	}
	~RPConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) {
			connections_.pop();
		}
	}
	std::unique_ptr<VerifyService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		condition_.wait(lock, [this] {  
			if (b_stop_) {
				return true;
			}
			return !connections_.empty(); });// 为false时解锁并将当前线程挂起阻塞
		//如果停止则直接返回空指针
		if (b_stop_) {
			return nullptr;
		}
		auto context = std::move(connections_.front());
		connections_.pop();  // queue弹出队首元素
		return context;
	}
	void returnConnection(std::unique_ptr<VerifyService::Stub> context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_) {
			return;
		}
		connections_.push(std::move(context));
		condition_.notify_one();
	}
	void Close() {
		b_stop_ = true;
		condition_.notify_all();
	}
private:
	std::atomic<bool> b_stop_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VerifyService::Stub>> connections_;  // 可改进使用双端队列，对首尾分别操作和加锁
	std::condition_variable condition_;
	std::mutex mutex_;
};



class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVerifyRsp GetVerifyCode(std::string email) {
		ClientContext context;
		GetVerifyRsp reply;
		GetVerifyReq request;

		request.set_email(email);

		auto stub = pool_->getConnection();
		Status status = stub->GetVerifyCode(&context, request, &reply);

		if (status.ok()) {
			pool_->returnConnection(std::move(stub));
			return reply;
		}
		else {
			pool_->returnConnection(std::move(stub));
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}
private:
	VerifyGrpcClient();
	//std::unique_ptr<VerifyService::Stub> stub_;  // 通信媒介，后改为从连接池中获取
	std::unique_ptr<RPConPool> pool_;
};

