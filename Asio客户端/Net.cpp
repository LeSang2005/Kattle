#include"Net.h"
#include"Socket.h"
#include"Log.h"
Net& Net::instance() {
	static Net net;
	return net;
}

Net::~Net() {
	loop_->stop();
	if (t.joinable()) {
		t.join();
	}
}

Net::Net():loop_(std::make_shared<boost::asio::io_context>()),
server_(boost::asio::ip::make_address(server_ip), server_port),
Socket_(nullptr)
{
	auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*loop_, server_.protocol());

	boost::system::error_code ec;
	sock->connect(server_, ec);                    
	if (ec) {
		LOG_ERROR("网络错误:错误码:"+ec.message());
		return;
	}
	Socket_ = std::make_shared<Socket>(sock, loop_);
}

void Net::send(const std::string message) {
	if (!Socket_) { LOG_ERROR("未连接，消息丢弃: " + message); return; }
	Socket_->send(message);
}
void Net::run() {
	if (!Socket_) { LOG_ERROR("连接不可用，run() 返回"); return; }
	if (readBack_) {
		Socket_->setReadBack(readBack_);
	}
	Socket_->doRead();
	if (writeBack_) {
		Socket_->setWriteBack(writeBack_);
	}
	t = std::thread(std::bind([this] {
		this->loop_->run();
		}));
}