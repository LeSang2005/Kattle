#include"Socket.h"
Socket::Socket(SmartSocket sock,EventLoop loop):
socket_(sock),loop_(loop)
{

}
Socket::~Socket() {

}
void Socket::send(const std::string& message) {
	std::string new_message = setHead(message);
	if (new_message.empty()) return;
	std::shared_ptr<std::string>message_ = std::make_shared<std::string>(new_message);
	std::weak_ptr <Socket>w_ptr = shared_from_this();
	boost::asio::async_write(*socket_, boost::asio::buffer(*message_,message_->size()), [w_ptr,message_](const boost::system::error_code& error, size_t  bytes_transferred) {
		if (!w_ptr.lock())return;
		if (error) {
			LOG_ERROR("错误：问题" + error.message());
			return;
		}
		if (w_ptr.lock()->writeBack_) {
			w_ptr.lock()->writeBack_();
		}
		});
}
// void Socket::doRead() {
// 	auto w_ptr = shared_from_this();
// 	boost::asio::async_read(*socket_,boost::asio::buffer(buf_.getCharReadBuf(), HEAD_SIZE), [w_ptr](const boost::system::error_code& error, size_t  bytes_transferred) {
// 		auto ptr = w_ptr.lock();
// 		if (!ptr)return;
// 		if (error) {
//             if(error == boost::asio::error::eof){
//                 ptr->closeBack_(w_ptr.lock());
//             }
// 			LOG_ERROR("错误：问题"+error.message());
// 			return; }
// 		ptr->buf_.addReadbufnum(bytes_transferred);
//         size_t message_num =std::atoi(ptr->buf_.GetRead().c_str());
// 	    boost::asio::async_read(*ptr->socket_, boost::asio::buffer(ptr->buf_.getCharReadBuf(), message_num), [w_ptr](const boost::system::error_code& error, size_t  bytes_transferred) {
// 		auto ptr = w_ptr.lock();
// 		if (!ptr)return;
// 		if (error) {  
//             if(error == boost::asio::error::eof){
//                 ptr->closeBack_(w_ptr.lock());
//             }
// 			LOG_ERROR("错误：问题" + error.message());
// 			return; }
// 		ptr->buf_.addReadbufnum(bytes_transferred);
// 		if(ptr->readBack_)
// 		ptr->readBack_(w_ptr.lock(),ptr->buf_);
// 		ptr->doRead();
// 	});
// });
	
// }
void Socket::doRead() {
    auto self = shared_from_this();          
    boost::asio::async_read(*socket_, boost::asio::buffer(buf_.getCharReadBuf(), HEAD_SIZE),
        [self](const boost::system::error_code& error, size_t bytes) {
            if (error) {
                if (self->closeBack_) self->closeBack_(self);
                return;                      
            }
            self->buf_.addReadbufnum(bytes);
            size_t message_num = std::atoi(self->buf_.GetRead().c_str());

            boost::asio::async_read(*self->socket_,
                boost::asio::buffer(self->buf_.getCharReadBuf(), message_num),
                [self](const boost::system::error_code& error, size_t bytes) {
                    if (error) {
                        if (self->closeBack_) self->closeBack_(self);
                        return;
                    }
                    self->buf_.addReadbufnum(bytes);
                    if (self->readBack_) self->readBack_(self, self->buf_);
                    self->doRead();         
                });
        });
}

std::string Socket::setHead(const std::string& message) {
	std::string num = std::to_string(message.size());
	if (num.size() > HEAD_SIZE) {
		LOG_ERROR("消息太长，包头装不下: " + std::to_string(message.size()));
		return {};
	}
	while (num.size() < HEAD_SIZE) num = "0" + num;
	return num + message;
}