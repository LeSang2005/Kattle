#include"redis.h"
redis::redis(){
    std::ifstream f("redis.json");
    if(!f.is_open()){
        LOG_FATAL("不存在redis.json文件");
    }
    json redisfile=json::parse(f);
    std::string ip;
    std::string port;
    std::string password;
    if(redisfile["redis"]["ip"].is_null()){
        LOG_FATAL("redis没有配置ip");
    }
    else{
        ip=redisfile["redis"]["ip"];
    }

    if(redisfile["redis"]["port"].is_null()){
        LOG_FATAL("redis没有配置端口");
    }
    else{
        port=redisfile["redis"]["port"];
    }

    if(redisfile["redis"]["password"].is_null()){
        LOG_WARN("redis没有配置密码");
    }
    else{
        password=redisfile["redis"]["password"];
    }

    r=redisConnect(ip.c_str(),atoi(port.c_str()));
    if(r==nullptr||r->err){
        LOG_FATAL("Redis 连接失败: " + std::string(r ? r->errstr : "unknown error"));
    }
    if(!password.empty()){
        redisReply* reply = (redisReply*)redisCommand(r, "AUTH %s", password.c_str());
        if(!reply || reply->type == REDIS_REPLY_ERROR){
            LOG_FATAL("redis密码错误");
        }
        freeReplyObject(reply);
    }
}


redis::~redis(){
    if(r!=nullptr){
        redisFree(r);
        r=nullptr;
    }
}
/*
"redis":{
        "ip":"127.0.0.1",
        "port":"6379",
        "password":"123456"
    }
*/



bool redis::execCommand(redisReply*& reply,const char* format,...){
va_list args;//将可变参数打包
va_start(args,format);//固定可变参数
reply=(redisReply*)redisvCommand(r,format,args);
va_end(args);//使用结束并且释放资源
if (!reply || (reply)->type == REDIS_REPLY_ERROR) {
        if (reply) freeReplyObject(reply); // 失败时自动释放内存
        reply = nullptr;
        return false;
    }
    return true;
}


bool redis::setstring(std::string key,std::string value,int ttl){
redisReply* reply;
if(ttl<0){
reply=(redisReply*)redisCommand(r,"set %s %s",key.c_str(),value.c_str());
}
else{
reply=(redisReply*)redisCommand(r,"setex %s %d %s",key.c_str(),ttl,value.c_str());   
}

if(!reply||reply->type==REDIS_REPLY_ERROR){
    if(reply)freeReplyObject(reply);
    LOG_ERROR("redis"+key+"添加失败");
    return false;
}
freeReplyObject(reply);
reply=nullptr;
return true;
}

std::string redis::getstring(std::string key){
redisReply* reply=(redisReply*)redisCommand(r,"get %s",key.c_str());
if(!reply||reply->type!=REDIS_REPLY_STRING){
    if(reply)freeReplyObject(reply);
    return "";
}
std::string res=reply->str;
freeReplyObject(reply);
reply=nullptr;
return res;
}

bool redis::delstring(std::string key){
redisReply* reply=(redisReply*)redisCommand(r,"del %s",key.c_str());
//返回 REDIS_REPLY_INTEGER，值为 1 表示成功
if (reply && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
        freeReplyObject(reply);
        reply=nullptr;
        return true;
    }
    if(reply) freeReplyObject(reply);
    reply=nullptr;
    return false;
}

long long redis::incrstring(const std::string key,int num){
redisReply* reply=(redisReply*)redisCommand(r,"incrby %s %d",key.c_str(),num);
if(reply&&reply->type==REDIS_REPLY_INTEGER){
    long long val=reply->integer;
    freeReplyObject(reply);
    reply=nullptr;
    return val;
}
if(reply) freeReplyObject(reply);
reply=nullptr;
return 0;
}

bool redis::hset(std::string key,std::string field,std::string value){
    redisReply* reply=nullptr;
    if(!execCommand(reply,"hset %s %s %s",key.c_str(),field.c_str(),value.c_str())){
        LOG_ERROR("Hset"+key+"插入失败");
        return false;
    }
    myfree(reply);
    return true;
}

std::string redis::hget(std::string key,std::string field){
redisReply* reply=nullptr;
if(!execCommand(reply,"hget %s %s",key.c_str(),field.c_str())||reply->type!=REDIS_REPLY_STRING){
if(reply)myfree(reply);
LOG_ERROR("Hget"+key+"得到失败");
return "";
}
std::string res=reply->str;
myfree(reply);
return res;
}

bool redis::hexists(std::string key,std::string field){
    Reply reply=nullptr;
    if(!execCommand(reply,"hexists %s %s",key.c_str(),field.c_str())){
        LOG_ERROR("redishash查询失败"+key);
        return false;
    }
    bool ok = (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
    if(reply){
    myfree(reply);  
    }
    return ok;
}

long long redis::hincrby(std::string key,std::string field ,long long num){
Reply reply=nullptr;
if(!execCommand(reply,"hincrby %s %s %lld",key.c_str(),field.c_str(),num)){
    LOG_ERROR("hest加时间失败"+key);
    return 0;
}
long long val=(reply->type==REDIS_REPLY_INTEGER)?reply->integer:0;
//REDIS_REPLY_INTEGER专用整数类型，如果返回的是这个类型就赋值，不是返回0
myfree(reply);
return val;
}

bool redis::hdel(std::string key,const std::string field){
Reply reply=nullptr;
if(!execCommand(reply,"hdel %s %s",key.c_str(),field.c_str())){
    LOG_ERROR("hset失败"+key);
    return false;
}
bool ok = (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
if(reply){
myfree(reply);  
}
return ok;
}


std::unordered_map<std::string,std::string> redis::hgetall(std::string key){
std::unordered_map<std::string, std::string> res;
Reply reply = nullptr;
if(!execCommand(reply,"hgetall %s",key.c_str())||reply->type!=REDIS_REPLY_ARRAY){//返回的类型是数组
if (reply) myfree(reply);
return res;
}
for(size_t i=0;i<reply->elements;i+=2){
    if (reply->element[i] && reply->element[i+1]) {
            res[reply->element[i]->str] = reply->element[i+1]->str;
    }
}
myfree(reply);
return res;
}


long long redis::lpush(std::string key,std::string value){
    Reply reply=nullptr;
    if(!execCommand(reply,"lpush %s %s",key.c_str(),value.c_str())){
        LOG_ERROR("lpush error"+key);
        return 0;
    }
    long long len = (reply->type == REDIS_REPLY_INTEGER) ? reply->integer : 0;
    myfree(reply);
    return len;
}

long long redis::rpush(std::string key,std::string value){
Reply reply=nullptr;
if(!execCommand(reply,"rpush %s %s",key.c_str(),value.c_str())){
    LOG_ERROR("lpush error"+key);
    return 0;
}
long long len = (reply->type == REDIS_REPLY_INTEGER) ? reply->integer : 0;
myfree(reply);
return len;
}


std::string redis::lpop(std::string key){
Reply reply=nullptr;
if(!execCommand(reply,"lpop %s",key.c_str())){
    LOG_ERROR("LPOP error"+key);
    return "";
}
std::string res=(reply->type==REDIS_REPLY_STRING?reply->str:"");
myfree(reply);
return res;
}



std::string redis::rpop(const std::string key) {
    Reply reply = nullptr;
    if (!execCommand(reply, "RPOP %s", key.c_str())) {
        LOG_ERROR("RPOP " + key + " 失败");
        return "";
    }
    std::string res = (reply->type == REDIS_REPLY_STRING) ? reply->str : "";
    myfree(reply);
    return res;
}

long long redis::llen(const std::string key) {
    Reply reply = nullptr;
    if (!execCommand(reply, "LLEN %s", key.c_str())) {
        LOG_ERROR("Llen " + key + " 失败");
        return 0;
    }
    long long len = (reply->type == REDIS_REPLY_INTEGER) ? reply->integer : 0;
    myfree(reply);
    return len;
}

std::vector<std::string> redis::lrange(std::string key,long long start,long long end){
    std::vector<std::string>res;
    Reply reply=nullptr;
    if(!execCommand(reply,"lrange %s %lld %lld",key.c_str(),start,end)||reply->type!=REDIS_REPLY_ARRAY){
        if(reply)myfree(reply);
        LOG_ERROR("LRANGE " + key + " 失败");
        return res;
    }
    for(size_t i=0;i<reply->elements;++i){
        if(reply->element[i]){
            res.emplace_back(reply->element[i]->str);
        }
    }
    myfree(reply);
    return res;
}


bool redis::sadd(std::string key,std::string member){
Reply reply=nullptr;
if(!execCommand(reply,"sadd %s %s",key.c_str(),member.c_str())){
    LOG_ERROR("Sadd"+key+"失败");
    return false;
}
bool ok = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
myfree(reply);
return ok;
}

bool redis::srem(std::string key,std::string member){
Reply reply=nullptr;
if(!execCommand(reply,"srem %s %s",key.c_str(),member.c_str())){
    LOG_ERROR("srem"+key+"失败");
    return false;
}
bool ok=(reply->type==REDIS_REPLY_INTEGER&&reply->integer>0);
myfree(reply);
return ok;
}

bool redis::sismember(std::string key,std::string member){
Reply reply=nullptr;
if(!execCommand(reply,"sismember %s %s",key.c_str(),member.c_str())){
    LOG_ERROR("查询失败"+key+"失败");
    return false;
}
bool ok=(reply->type==REDIS_REPLY_INTEGER&&reply->integer==1);
myfree(reply);
return ok;
}

long long redis::scard(std::string key) {
    Reply reply = nullptr;
    if (!execCommand(reply, "SCARD %s", key.c_str())) {
        LOG_ERROR("Scard " + key + " 失败");
        return 0;
    }
    long long cnt = (reply->type == REDIS_REPLY_INTEGER) ? reply->integer : 0;
    myfree(reply);
    return cnt;
}

std::vector<std::string> redis::smembers(std::string key){
    std::vector<std::string>res;
    Reply reply=nullptr;
    if(!execCommand(reply,"smembers %s",key.c_str()) || reply->type != REDIS_REPLY_ARRAY){
       if (reply) myfree(reply);
        LOG_ERROR("Smembers " + key + " 失败");
        return res; 
    }
    for(size_t i=0;i<reply->elements;++i){
        if(reply->element[i]){
            res.emplace_back(reply->element[i]->str);
        }
    }
    myfree(reply);
    return res;
}


bool redis::zadd(std::string key,double score,std::string member){
    Reply reply=nullptr;
    if(!execCommand(reply,"zadd %s %f %s",key.c_str(),score,member.c_str())){
        LOG_ERROR("zadd error"+key);
        return false;
    }
    bool ok = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    myfree(reply);
    return ok;
}

bool redis::zrem(std::string key,std::string member){
Reply reply=nullptr;
if(!execCommand(reply,"zrem %s %s",key.c_str(),member.c_str())){
    LOG_ERROR("zerm error"+key);
    return false;
}
bool ok=(reply->type==REDIS_REPLY_INTEGER&&reply->integer>0);
myfree(reply);
return ok;
}

double redis::zincrby(std::string key,double increment,std::string member){
    Reply reply=nullptr;
    if(!execCommand(reply,"zincrby %s %f %s",key.c_str(),increment,member.c_str())){
        LOG_ERROR("zincrby error"+key);
        return 0.0;
    }
    double val=0.0;
    if(reply->type==REDIS_REPLY_STRING&&reply->str){
        val=std::stod(reply->str);
    }
    myfree(reply);
    return val;
}

double redis::zscore(std::string key,std::string member){
    Reply reply=nullptr;
    if(!execCommand(reply,"zscore %s %s",key.c_str(),member.c_str())||reply->type!=REDIS_REPLY_STRING){
        if(reply)myfree(reply);
        return 0.0;
    }
    double val=0.0;
    val=std::stod(reply->str);
    myfree(reply);
    return val;
}

long long redis::zcard(const std::string key) {
    Reply reply = nullptr;
    if (!execCommand(reply, "ZCARD %s", key.c_str())) {
        LOG_ERROR("Zcard " + key + " 失败");
        return 0;
    }
    long long cnt = (reply->type == REDIS_REPLY_INTEGER) ? reply->integer : 0;
    myfree(reply);
    return cnt;
}


std::vector<std::string> redis::range(const std::string& key, long long start, long long stop,redis_cal cal){
std::vector<std::string> res;
    Reply reply = nullptr;
    if(cal==ASC){
    if (!execCommand(reply, "ZRANGE %s %lld %lld", key.c_str(), start, stop) || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) myfree(reply);
        LOG_ERROR("Zrevrange " + key + " 失败");
        return res;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
        if (reply->element[i]) {
            res.emplace_back(reply->element[i]->str);
        }
    }
}
else if(cal==DESC){
if (!execCommand(reply, "ZREVRANGE %s %lld %lld", key.c_str(), start, stop) || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) myfree(reply);
        LOG_ERROR("Zrevrange " + key + " 失败");
        return res;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
        if (reply->element[i]) {
            res.emplace_back(reply->element[i]->str);
        }
    }

}
    myfree(reply);
    return res;
}


std::vector<std::pair<std::string, double>> redis::range_withscores(std::string key, long long start, long long stop,redis_cal cal){
std::vector<std::pair<std::string, double>> res;
    Reply reply = nullptr;
    if(cal==ASC){
        if (!execCommand(reply, "ZRANGE %s %lld %lld WITHSCORES", key.c_str(), start, stop) || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) myfree(reply);
        return res;
    }
    // 注意：带 WITHSCORES 时，返回的数组是 [书ID1, 分数1, 书ID2, 分数2, ...]
    for (size_t i = 0; i < reply->elements; i += 2) {
        if (reply->element[i] && reply->element[i+1]) {
            std::string member = reply->element[i]->str;
            double score = 0.0;
            try { score = std::stod(reply->element[i+1]->str); } catch(...) {}
            res.emplace_back(member, score);
        }
    }
    }
    else if(cal==DESC){
        if (!execCommand(reply, "ZREVRANGE %s %lld %lld WITHSCORES", key.c_str(), start, stop) || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) myfree(reply);
        return res;
    }
    for (size_t i = 0; i < reply->elements; i += 2) {
        if (reply->element[i] && reply->element[i+1]) {
            std::string member = reply->element[i]->str;
            double score = 0.0;
            try { score = std::stod(reply->element[i+1]->str); } catch(...) {}
            res.emplace_back(member, score);
        }
    }
    }
    myfree(reply);
    return res;
}