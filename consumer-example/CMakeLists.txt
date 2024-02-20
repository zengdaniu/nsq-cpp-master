cmake_minimum_required (VERSION 2.8)

# 项目信息
project (Demo1)

add_compile_options(-Wall -fPIC -g -std=c++11)

include_directories(../uvnsq/libuv/include )
include_directories(../uvnsq)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../uvnsq/lib)

# 添加链接库
# 指定生成目标
add_executable(Demo1 main.cpp)
#target_link_libraries(Demo1  libuv.so -lpthread -lm)
target_link_libraries(Demo1  uvnsq -lpthread -lm)
