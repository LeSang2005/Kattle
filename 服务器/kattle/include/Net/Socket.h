#pragma once
#include"basic.h"
#include"Buffer.h"
#include<functional>
#include<memory>
#include"Log.h"
class Socket:public std::enable_shared_from_this<Socket> {
public:
	Socket(SmartSocket sock, EventLoop loop_);
	~Socket();
	void setReadBack(const std::function<void(TcpSmartSocket, Buffer&)>& cb) {
		readBack_ = std::move(cb);
	}
	void setWriteBack(const std::function<void()>& cb) {
		writeBack_ = std::move(cb);
	}
    void setCloseBack(const std::function<void(TcpSmartSocket)>&cb){
        closeBack_=std::move(cb);
    }
	void send(const std::string& message);
	void doRead();
	static std::string setHead(const std::string& message);
private:
	SmartSocket socket_;
	Server_Position server_;
	EventLoop loop_;
	std::function<void(TcpSmartSocket, Buffer&)>readBack_;
	std::function<void()>writeBack_;
    std::function<void(TcpSmartSocket)>closeBack_;
	Buffer buf_;
};