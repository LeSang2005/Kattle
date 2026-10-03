#pragma once
#include<hiredis/hiredis.h>
#include<fstream>
#include"nlohmann/json.hpp"
#include"Log.h"
#include<string>
#include<unordered_map>
#include<cstdarg>
#include<vector>
class redis{
public:
enum redis_cal{
ASC=0,
DESC
};
using json=nlohmann::json;//函数不可以加
using Reply=redisReply*;
redis();
~redis();
//string
bool setstring(std::string key,std::string value,int ttl=-1);
std::string getstring(std::string key);
bool delstring(std::string key);
long long incrstring(std::string key,int num=1);
//hash
bool hset(std::string key,std::string field,std::string value);
std::string hget(std::string key,std::string field);
bool hexists(std::string key,std::string field);
long long hincrby(std::string key,std::string field ,long long num);
bool hdel(std::string key,const std::string field);
std::unordered_map<std::string,std::string> hgetall(std::string key);
//list
long long lpush(std::string key,std::string value);
long long rpush(std::string key,std::string value);
std::string lpop(std::string key);
std::string rpop(std::string key);
long long llen(std::string key);
std::vector<std::string> lrange(std::string key,long long start,long long end);
//set
bool sadd(std::string key,std::string member);
bool srem(std::string key,std::string member);
bool sismember(std::string key,std::string member);
long long scard(std::string key);
std::vector<std::string> smembers(std::string key);
//sortedset
bool zadd(std::string key,double score,const std::string member);
bool zrem(std::string key,std::string member);
double zincrby(std::string key,double increment,std::string member);
double zscore(std::string key,std::string member);
long long zcard(std::string key);
std::vector<std::string> range(const std::string& key, long long start, long long stop,redis_cal cal=ASC);
std::vector<std::pair<std::string, double>> range_withscores(std::string key, long long start, long long stop,redis_cal cal=ASC);
private:
bool execCommand(redisReply*& reply,const char* format,...);
void myfree(redisReply*& reply) {
    if (reply != nullptr) {
        freeReplyObject(reply);
        reply = nullptr;
    }
}
redisContext* r;//redis连接句柄
};