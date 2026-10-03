#pragma once
#pragma once
/**
 * @file ReadLog.h
 * @brief 异步日志系统
 *
 * 生产者-消费者模型：
 *   - 生产者：各业务模块通过 LOG_XXX 宏写入日志
 *   - 消费者：后台线程将日志异步写入磁盘文件
 *
 * 日志按日期分目录存储：./log/YYYY-M-D/YYYY-M-D.txt
 * 支持 DEBUG / INFO / WARN / ERROR / FATAL 五个级别。
 */
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <chrono>
#include <string>
#include <thread>
#include<atomic>
class ReadLog {
public:
    /// 日志级别枚举
    enum Level {
        LV_DEBUG = 0,
        LV_INFO,
        LV_WARN,
        LV_ERROR,
        LV_FATAL
    };

    /// 日志消息结构体
    struct message {
        std::string message;   ///< 日志正文
        std::string Date;      ///< 日期（YYYY-M-D）
        std::string Time;      ///< 时间戳（[YYYY-M-D HH:MM:SS] ）
        Level level;           ///< 日志级别
    };

    /// 将日志消息放入队列（生产者调用，线程安全）
    void push(message mess) {
        std::unique_lock<std::mutex> lock(mtx_);
        while (running_) {
            if (Log_q.size() < 50000) {
                Log_q.push(mess);
                break;
            }
            else if (nextLog_q.size() < 50000) {
                nextLog_q.push(mess);
                break;
            }
            else {
                if (the_end_Log_q.empty() || the_end_Log_q[the_end_Log_q.size() - 1].size() >= 50000)
                    the_end_Log_q.emplace_back();
                the_end_Log_q[the_end_Log_q.size() - 1].push(mess);
                break;
            }
        }
        if (!running_) {
            std::string word = mess.Time + mess.message + "\n";
            writemessage(word, mess);
            {
                std::unique_lock<std::mutex>lock(readmtx_);
                if (path_ != getdaypath()) {
                    f.close();
                    path_ = createpath();
                    f = std::fstream(path_, std::ios::app);
                }
                f << word;
            }
        }
    }

    /// 后台日志写入线程主函数
    void LogWork() {
        while (running_) {
            std::queue<message>temp_queue;
            std::vector<std::queue<message>>the_temp_end_Log_q;
            {
                std::unique_lock<std::mutex>lock(mtx_);
                while (running_) {
                    if (!Log_q.empty()) {
                        temp_queue.swap(Log_q);

                        break;
                    }
                    else if (!nextLog_q.empty()) {
                        temp_queue.swap(nextLog_q);

                        break;
                    }
                    else if (!the_end_Log_q.empty()) {
                        the_temp_end_Log_q.swap(the_end_Log_q);
                        break;
                    }
                    cond_.wait_for(lock, std::chrono::milliseconds(500));
                }
            }
            std::string endword;
            while (!temp_queue.empty()) {
                message mess = std::move(temp_queue.front());
                temp_queue.pop();
                std::string word = mess.Time + mess.message + "\n";
                writemessage(word, mess);
                endword += word;
            }

            if (!the_temp_end_Log_q.empty()) {
                for (auto& que : the_temp_end_Log_q) {
                    while (!que.empty()) {
                        message mess = std::move(que.front());
                        que.pop();
                        std::string word = mess.Time + mess.message + "\n";
                        writemessage(word, mess);
                        endword += word;
                    }
                }
            }
            {
                std::unique_lock<std::mutex>lock(readmtx_);
                if (path_ != getdaypath()) {
                    f.close();
                    path_ = createpath();
                    f = std::fstream(path_, std::ios::app);
                }
                f << endword;
            }
        }
        {
            std::unique_lock<std::mutex>lock(mtx_);
            while (!Log_q.empty()) {
                message mess = std::move(Log_q.front());
                Log_q.pop();
                std::string word = mess.Time + mess.message + "\n";
                writemessage(word, mess);
                {
                    std::unique_lock<std::mutex>lock(readmtx_);
                    if (path_ != getdaypath()) {
                        f.close();
                        path_ = createpath();
                        f = std::fstream(path_, std::ios::app);
                    }
                    f << word;
                }
            }

            while (!nextLog_q.empty()) {
                message mess = std::move(nextLog_q.front());
                nextLog_q.pop();
                std::string word = mess.Time + mess.message + "\n";
                writemessage(word, mess);
                {
                    std::unique_lock<std::mutex>lock(readmtx_);
                    if (path_ != getdaypath()) {
                        f.close();
                        path_ = createpath();
                        f = std::fstream(path_, std::ios::app);
                    }
                    f << word;
                }
            }
        }
        if (!the_end_Log_q.empty()) {
            std::string endword;
            for (auto& que : the_end_Log_q) {
                while (!que.empty()) {
                    message mess = std::move(que.front());
                    que.pop();
                    std::string word = mess.Time + mess.message + "\n";
                    writemessage(word, mess);
                    endword += word;
                }
            }
            {
                std::unique_lock<std::mutex>lock(readmtx_);
                if (path_ != getdaypath()) {
                    f.close();
                    path_ = createpath();
                    f = std::fstream(path_, std::ios::app);
                }
                f << endword;
            }
        }

    }

    /// 获取日志系统单例
    static ReadLog& instance();

    /// 获取当前日期字符串（如 "2026-7-18"）
    static std::string getCurrentDate();

    /// 获取当前时间字符串（如 "[2026-7-18 14:30:25] "）
    static std::string getCurrentTime();
    ~ReadLog() {
        running_.store(false);
        cond_.notify_all();
        work_.join();
        {
            std::unique_lock<std::mutex>lock(readmtx_);
            f.close();
        }
    }
private:
    ReadLog() {
        // 启动后台日志写入线程（detach，随进程退出）
        running_.store(true);
        path_ = createpath();
        f = std::fstream(path_, std::ios::app);
        work_ = std::thread(&ReadLog::LogWork, this);
        maxsize = 50;

    }
    void writemessage(std::string& word, message& mess) {
        if (mess.level == LV_DEBUG) {
            word = "DEBUG:" + word;
        }
        else if (mess.level == LV_WARN) {
            word = "WARN:" + word;
        }
        else if (mess.level == LV_INFO) {
            word = "INFO:" + word;
        }
        else if (mess.level == LV_ERROR) {
            word = "ERROR:" + word;
        }
        else if (mess.level == LV_FATAL) {
            word = "FATAL:" + word;
        }
    }
    std::string createpath() {
        std::string path = "./log/";
        path += getCurrentDate();
        std::filesystem::create_directories(path);
        path = path + "/" + getCurrentDate() + ".txt";
        return path;
    }
    std::string getdaypath() {
        std::string path = "./log/";
        path += getCurrentDate();
        path = path + "/" + getCurrentDate() + ".txt";
        return path;
    }
    std::condition_variable cond_;  ///< 条件变量
    std::mutex mtx_;                ///< 队列互斥锁
    std::queue<message> Log_q;      ///< 日志消息队列
    std::queue<message>nextLog_q;
    std::atomic<bool>running_;
    std::thread work_;
    int maxsize;
    std::fstream f;
    std::string path_;
    std::mutex readmtx_;
    std::vector<std::queue<message>>the_end_Log_q;
};

inline ReadLog& ReadLog::instance() {
    static ReadLog readLog;
    return readLog;
}

inline std::string ReadLog::getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto currentTime = std::chrono::system_clock::to_time_t(now);
    auto tm = std::localtime(&currentTime);
    std::string currentDate = std::to_string(tm->tm_year + 1900) + '-'
        + std::to_string(tm->tm_mon + 1) + '-'
        + std::to_string(tm->tm_mday);
    return currentDate;
}

inline std::string ReadLog::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto currentTime = std::chrono::system_clock::to_time_t(now);
    auto tm = std::localtime(&currentTime);
    std::string currentDate = "[" + std::to_string(tm->tm_year + 1900) + '-'
        + std::to_string(tm->tm_mon + 1) + '-'
        + std::to_string(tm->tm_mday) + ' '
        + std::to_string(tm->tm_hour) + ':'
        + std::to_string(tm->tm_min) + ':'
        + std::to_string(tm->tm_sec) + "] ";
    return currentDate;
}


#define LOG_DEBUG(msg) do { \
    ReadLog::message _m; \
    _m.message = (msg); \
    _m.Date = ReadLog::getCurrentDate(); \
    _m.Time = ReadLog::getCurrentTime(); \
    _m.level = ReadLog::LV_DEBUG; \
    ReadLog::instance().push(_m); \
} while(0)

#define LOG_INFO(msg) do { \
    ReadLog::message _m; \
    _m.message = (msg); \
    _m.Date = ReadLog::getCurrentDate(); \
    _m.Time = ReadLog::getCurrentTime(); \
    _m.level = ReadLog::LV_INFO; \
    ReadLog::instance().push(_m); \
} while(0)

#define LOG_WARN(msg) do { \
    ReadLog::message _m; \
    _m.message = (msg); \
    _m.Date = ReadLog::getCurrentDate(); \
    _m.Time = ReadLog::getCurrentTime(); \
    _m.level = ReadLog::LV_WARN; \
    ReadLog::instance().push(_m); \
} while(0)

#define LOG_ERROR(msg) do { \
    ReadLog::message _m; \
    _m.message = (msg); \
    _m.Date = ReadLog::getCurrentDate(); \
    _m.Time = ReadLog::getCurrentTime(); \
    _m.level = ReadLog::LV_ERROR; \
    ReadLog::instance().push(_m); \
} while(0)


#define LOG_FATAL(msg)do{ \
ReadLog::message _m; \
_m.message=(msg); \
_m.Date=ReadLog::getCurrentDate(); \
_m.Time=ReadLog::getCurrentTime(); \
_m.level=ReadLog::LV_FATAL; \
ReadLog::instance().push(_m); \
std::this_thread::sleep_for(std::chrono::milliseconds(100)); \
exit(EXIT_FAILURE); \
}while(0)
