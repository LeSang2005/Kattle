#pragma once
#include<boost/asio.hpp>
#include<string>
#include<memory>
#include"Buffer.h"
#include"NoCopy.h"
#include"Basic.h"
#include<thread>
#include<functional>
class Net:public NoCopy{
public:
	static Net& instance();
	~Net();
	void send(const std::string message);
	void run();
	void setReadBack(const std::function<void(TcpSmartSocket, Buffer&)>& cb) {
		readBack_ = std::move(cb);
	}
	void setWriteBack(const std::function<void()>& cb) {
		writeBack_ = std::move(cb);
	}
private:
	Net();
const std::string server_ip = "127.0.0.1";
const short server_port = 8080;
EventLoop loop_;
Server_Position server_;
TcpSmartSocket Socket_;

std::thread t;
std::function<void(TcpSmartSocket, Buffer&)>readBack_;
std::function<void()>writeBack_;
};