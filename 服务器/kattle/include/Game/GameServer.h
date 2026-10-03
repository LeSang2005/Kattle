#pragma once
#include"Server.h"
#include"nlohmann/json.hpp"
#include"const.h"
class GameServer{
public:
GameServer();
private:
void init();
TcpServer server_;
};