#pragma once
#include"nlohmann/json.hpp"
#include<vector>
#include<string>
#include<utility>
#include"mysqlpool.h"

using json = nlohmann::json;
enum class block {
    grass = 0
};
inline bool createMap_Easy(size_t height, size_t width, block fill, const std::string& name)
{
    json arr = json::array();
    auto conn = mysqlPool::instance().pop();
    if (!conn) { LOG_ERROR("拿不到数据库连接"); return false; }

    json res=conn->find(BuildSelect(*conn,"Kattle_map",{"Map_Name"},{{"Map_Name","=",name}}));
    if(!res.empty()){
        LOG_INFO("已经存在地图:"+name);
        return true;
    }
    for (size_t y = 0; y < height; ++y) {
        json row = json::array();
        for (size_t x = 0; x < width; ++x)
            row.push_back(static_cast<int>(fill));
        arr.push_back(std::move(row));
    }
    

    std::string sql = BuildInsert(*conn, "Kattle_map", {
        {"Map_Name", name},
        {"Map",arr.dump()},
    });
    return conn->insert(sql);
}

