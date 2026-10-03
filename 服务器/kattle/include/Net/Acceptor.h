#pragma once
#include<boost/asio.hpp>
#include<iostream>
#include<memory>
#include"ThreadQueue.h"
#include<atomic>
#include<thread>
using namespace boost;

class Acceptor {
public:
	Acceptor( std::shared_ptr<asio::ip::tcp::acceptor>listen_acceptor);
	~Acceptor();
	void run();
	void handle_accept();
	std::shared_ptr < asio::ip::tcp::socket> pop();

private:
	std::shared_ptr<asio::ip::tcp::acceptor>listen_acceptor_;
	std::atomic<bool> running;
	ThreadQueue<std::shared_ptr<asio::ip::tcp::socket>>write_socket_queue;
	std::thread acceptor_thread;

};