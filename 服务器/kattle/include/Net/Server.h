#pragma once
#include<boost/asio.hpp>
#include<iostream>
#include<string>
#include<atomic>
#include"Acceptor.h"
#include<vector>
#include<functional>
#include"basic.h" 
#include"Buffer.h"   
class Session;
using namespace boost;
class TcpServer {
public:
	using netPoint = asio::ip::tcp::endpoint;
	TcpServer(short port);
	~TcpServer();
	void run();
	void setsession_num(int num) {
		session_num = num;
	}
	void setreadback(std::function<void(TcpSmartSocket, Buffer&)>cb) {
		readback = std::move(cb);
	}
	void setreadback(std::function<void()>cb) {
		writeback = std::move(cb);
	}
	void setCloseBack(std::function<void(TcpSmartSocket)>cb){
		closeBack=std::move(cb);
	}
private:
	short port;
	netPoint Tcpserver_ip_port;
	std::shared_ptr<asio::io_context>ios_;
	std::vector<std::shared_ptr<Session>>session_arr;
	size_t session_num;
	int index;
	Acceptor accept;
	std::function<void(TcpSmartSocket, Buffer&)>readback;
    std::function<void()>writeback;
    std::function<void(TcpSmartSocket)>closeBack;
	std::atomic<bool>running;
	std::thread t;
};
