cmake_minimum_required(VERSION 3.12)
project(GameHttp)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" CACHE STRING "Debug flags" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Game)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Net)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Storage)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Tool)
file(GLOB_RECURSE my CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp""${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
find_package(Boost REQUIRED)  #find_package是找到依赖，后面是找不到怎么办，如果没找到立刻
#停下
add_executable(kattle ${my})
target_compile_options(kattle PRIVATE -fsanitize=leak -fno-omit-frame-pointer)
target_link_options(kattle PRIVATE -fsanitize=leak)
target_link_libraries(kattle PUBLIC Boost::headers mysqlclient ssl crypto hiredis protobuf)
#-L 和 -l 是一对 -L是到哪里找， -l找啥
#find_library(HIREDIS_LIB NAMES hiredis) HIREDIS_LIB存储 hiredis找什么
#HINTS去哪里找 PATHS    =  去【哪】找（最后搜的目录）
#有点明白了，target_link_libraries(kattle private Boost::headers 
#mysqlclient ssl crypto hiredis protobuf),kattle就是说kattle里面的文件都会去找
#，假如出现另一个文件包含了这些文件里面的一个，private不会连上而是报连接找不到
