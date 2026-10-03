#pragma once
#include<memory>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<thread>
#include<Timestamp.h>
#include<atomic>
#include"Log.h"
template<typename T>
class anypool: public std::enable_shared_from_this<anypool<T>>{
public:
struct anycode{
    T* any;
    Timestamp time_;
    std::weak_ptr<anypool<T>>pool_wk;
};
static std::shared_ptr<anypool<T>> instance();
~anypool();
std::shared_ptr<T> pop();
void setinit(int num){init_=num;}
void setmaxinit(int num){maxinit_=num;}
void fresh();
void start();
private:
anypool(const anypool&) = delete;
anypool& operator=(const anypool&) = delete;
std::thread t;
anypool();
std::queue<anycode>any_q;
std::mutex any_mtx;
std::condition_variable any_cond;
std::atomic_bool any_bool;
int init_;
int maxinit_;
std::atomic_bool start_;
};

template<typename T>
anypool<T>::anypool():init_(4),maxinit_(1024),any_bool(false),start_(false){
}

template<typename T>
std::shared_ptr<anypool<T>> anypool<T>::instance() {
        static auto pool_ = std::shared_ptr<anypool<T>>(new anypool<T>());//必须要创建一个智能指针不然必然报错，为什么？因为基数为0，再去调用
        //share_from_this不是得到为0的智能指针吗，所以必然错误
        return pool_;
    }


template<typename T>
void anypool<T>::start(){
    if(start_==true)return;
    start_=true;
    for(int i=0;i<init_;i++){
        anycode anycode_;
        anycode_.any=new T();
        anycode_.time_.setnow();
        anycode_.pool_wk = this->shared_from_this();
        any_q.push(anycode_);
    }
    t=std::thread(&anypool<T>::fresh,this);
    //t.detach();
}

template<typename T>
anypool<T>::~anypool(){
    {
      any_bool=true;
    any_cond.notify_all();
    std::unique_lock<std::mutex>lock(any_mtx);
    while(!any_q.empty()){
        delete any_q.front().any;
        any_q.front().any=nullptr;
        any_q.pop();
    }  
    }
    
    if(start_==true){
        t.join();
    }
    
}

template<typename T>
void anypool<T>::fresh(){
    while(!any_bool){
        std::unique_lock<std::mutex>lock(any_mtx);
        while(any_bool==false&&any_q.size()<=4){
            any_cond.wait(lock);
        }
        if(any_bool==true){
            break;
        }
        while(any_q.size()>4){
            if(any_q.front().time_.alive()>300){
                delete any_q.front().any;
                any_q.front().any=nullptr;
                any_q.pop();
                init_--;
            }
            else{
                any_cond.wait_for(lock, std::chrono::milliseconds(10));//到点自动清醒
                break; 
            }
        }
    }
}

template<typename T>
std::shared_ptr<T> anypool<T>::pop(){
    anycode temp_any_;
    
    {
        std::unique_lock<std::mutex>lock(any_mtx);
    if(any_q.empty()){
        if(maxinit_>init_){
        anycode anycode_;
        anycode_.any=new T();
        anycode_.time_.setnow();
        anycode_.pool_wk = this->shared_from_this();
        any_q.push(anycode_);
        init_++;
        any_cond.notify_all();
        }
        else{
        LOG_WARN("对象池已经到达连接上限");
        return nullptr;}
    }
   
    temp_any_=any_q.front();
    any_q.pop();
    }
    return std::shared_ptr<T>(temp_any_.any,[node=temp_any_](T* any)mutable{//捕获可修改
        auto share_p=node.pool_wk.lock();
        if(share_p&&share_p->any_bool==false){
                std::unique_lock<std::mutex>lock(share_p->any_mtx);
                node.time_.setnow();
                share_p->any_q.push(node);
                share_p->any_cond.notify_all();
        }
        else{
            delete any;
            any=nullptr;
        }
    });
}