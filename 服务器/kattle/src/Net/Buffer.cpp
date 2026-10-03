#include"Buffer.h"
#include"Log.h"
#include"cstring"
Buffer::Buffer():MAX_READ_NUM(BUFFER_SIZE),MAX_WRITE_NUM(BUFFER_SIZE),
read_buf_(nullptr),write_buf_(nullptr),read_Total_(0),
write_Total_(0)
{
	read_buf_ = new char[BUFFER_SIZE];
	write_buf_ = new char[BUFFER_SIZE];
}
Buffer::~Buffer() {
	delete []read_buf_;
	delete[]write_buf_;
}
void Buffer::sendRead(const std::string& read_buf) {
	if (read_Total_+ read_buf.size() > MAX_READ_NUM) {
		LOG_ERROR("读缓冲区已经满了，本次数据丢失,丢失的数据为:"+read_buf);
		return;
	}
	memcpy(read_buf_+read_Total_,read_buf.c_str(),read_buf.size());
	//read_buf[]是char类型
	read_Total_ += read_buf.size();
}
void Buffer::sendWrite(const std::string& write_buf) {
	if (write_Total_ + write_buf.size()>MAX_WRITE_NUM) {
		LOG_ERROR("写缓冲区已经满了，本次数据丢失,丢失的数据为:" + write_buf);
		return;
	}
	memcpy(write_buf_+write_Total_,write_buf.c_str(),write_buf.size());
	write_Total_ += write_buf.size();
}

std::string Buffer::GetRead() {
	if (read_Total_ == 0) {
		LOG_WARN("读空缓冲区");
		return {};
	}
	std::string buf(read_buf_,read_Total_);
	read_Total_ = 0;
	return buf;
}
std::string Buffer::GetWrite() {
	if (write_Total_ == 0) {
		LOG_WARN("读空缓冲区");
		return {};
	}
	std::string buf(write_buf_, write_Total_);
	write_Total_ = 0;
	return buf;
}