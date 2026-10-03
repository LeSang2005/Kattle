#pragma once
#include<memory>
#include<mutex>
#include<condition_variable>
#include<queue>
template<typename T>
class ThreadQueue {
public:
	void push(T val) {
		std::unique_lock<std::mutex> lock(mtx_); ;
		q.push(val);
		cond_.notify_all();
	}
	T pop() {
		std::unique_lock<std::mutex> lock(mtx_); ;
		while (q.empty()) {
			cond_.wait(lock);
		}
		T val = std::move(q.front());
		q.pop();
		return val;
	}
private:
	std::queue<T>q;
	std::mutex mtx_;
	std::condition_variable cond_;
};