#pragma once
#include<boost/asio.hpp>
#include<functional>
#include<memory>
#include<string>
#include<atomic>
#include<vector>
#include<thread>
#include<queue>
#include<iostream>
#include"Buffer.h"
#include"Socket.h"
class Session:public NoCopy{
public:
Session():loop_(std::make_shared<boost::asio::io_context>()),guard_(boost::asio::make_work_guard(*loop_)){
	t=std::thread([this](){
		loop_->run();
	});
}

void addSocket(TcpSmartSocket conn){
	conn->setCloseBack(closeBack_);
	conn->setReadBack(readBack_);
	conn->setWriteBack(writeBack_);
	conn->doRead();
}
EventLoop& GetLoop(){
	return loop_;
}
~Session(){
	loop_->stop();
	if(t.joinable()){
		t.join();
	}
}
void setReadBack(const std::function<void(TcpSmartSocket, Buffer&)>& cb) {
		readBack_ = std::move(cb);
}
void setWriteBack(const std::function<void()>& cb) {
	writeBack_ = std::move(cb);
}
void setCloseBack(const std::function<void(TcpSmartSocket)>&cb){
    closeBack_=std::move(cb);
}
private:
EventLoop loop_;
std::function<void(TcpSmartSocket, Buffer&)>readBack_;
std::function<void()>writeBack_;
std::function<void(TcpSmartSocket)>closeBack_;
std::thread t;
asio::executor_work_guard<asio::io_context::executor_type> guard_; 
};