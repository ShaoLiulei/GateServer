#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(net::io_context& ioc, unsigned short& port):_ioc(ioc),
_acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{

}

void CServer::Start()
{
	auto self = shared_from_this();
	// 从AsioIOServicePool连接池中获取连接
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);

	_acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
		try {
			// 出错则放弃当前连接，继续监听其他连接
			if (ec) {
				self->Start();
				return;
			}
			// 创建新连接，并创建HttpConnection类来管理连接
			new_con->Start();
			// 继续监听
			self->Start();
		}
		catch (std::exception& exp) {
			std::cout << "exception is " << exp.what() << std::endl;
			self->Start();
		}
		});
}
