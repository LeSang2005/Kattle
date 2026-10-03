#include"Acceptor.h"
Acceptor::Acceptor(std::shared_ptr<asio::ip::tcp::acceptor>listen_acceptor)
	:listen_acceptor_(listen_acceptor), running(false)
{
}
Acceptor::~Acceptor() {
	running = false;
	listen_acceptor_->close();
	if (acceptor_thread.joinable()) {
		acceptor_thread.join(); 
	}
}
void Acceptor::handle_accept() {
	listen_acceptor_->listen();
	while (running) {
		asio::ip::tcp::socket* new_socket = new asio::ip::tcp::socket(listen_acceptor_->accept());
		if (new_socket) {
			write_socket_queue.push(std::shared_ptr<asio::ip::tcp::socket>(new_socket));
		}
	}
}

void Acceptor::run() {
	running = true;
	acceptor_thread = std::thread(std::bind(&Acceptor::handle_accept,this));
}

std::shared_ptr < asio::ip::tcp::socket> Acceptor::pop() {
	return write_socket_queue.pop();
}