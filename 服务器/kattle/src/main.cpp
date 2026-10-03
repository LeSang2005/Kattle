#include"Server.h"
#include <iostream>
#include"Session.h"
#include"const.h"
//做基本的初始化
void init(){
bool flag=createMap_Easy(20,20,block::grass,"草地块");
if(flag==false){
    LOG_ERROR("创建草地快失败");
}
}
int main() {
    init();
    return 0;
}