#pragma once
#include<string>
#include"NoCopy.h"
const int HEAD_SIZE = 8;
const int BUFFER_SIZE = 1024;
class Buffer:public NoCopy {
public:
	Buffer();
	~Buffer();
	void sendRead(const std::string& read_buf);
	void sendWrite(const std::string& write_buf);
    std::string GetRead();
	std::string GetWrite();
	size_t GetWriteTotal() {
		return write_Total_;
	}
	size_t GetReadTotal() {
		return read_Total_;
	}

	void clearRead() {
		read_Total_ = 0;
	}
	void clearWrite() {
		write_Total_ = 0;
	}
	char* getCharReadBuf() {
		return read_buf_;
	}
	char* getCharWriteBuf() {
		return write_buf_;
	}
	void addReadbufnum(size_t num) {
		read_Total_ += num;
	}
	void addWritebufnum(size_t num) {
		write_Total_ += num;
	}
private:
	char* read_buf_;
	char* write_buf_;
	size_t write_Total_;
	//size_t write_num_;
	size_t read_Total_;
	//size_t read_num_;
	size_t MAX_READ_NUM;
	size_t MAX_WRITE_NUM;
};