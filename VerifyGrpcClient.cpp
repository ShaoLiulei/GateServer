#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

VerifyGrpcClient:: VerifyGrpcClient() {
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string host = gCfgMgr["VerifyServer"]["host"];
	std::string port = gCfgMgr["VerifyServer"]["port"];
	pool_.reset(new RPConPool(5, host, port));  // Ĭ��5�����ӳ�
	}
