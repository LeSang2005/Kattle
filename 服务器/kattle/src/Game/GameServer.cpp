#include"GameServer.h"
#include<fstream>
#include"Log.h"
void GameServer::init(){
    std::ifstream f("all.json");
    if(!f.is_open()){
        LOG_FATAL("不存在all.json文件");
    }
    json js=json::parse(f);
    
}