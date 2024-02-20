# CMakeLists.txt
# Copyright (C) orcaer@yeah.net. All rights reserved.
# Last modified: 2019-9-3

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

add_compile_options(-Wall -fPIC -lpthread -g -std=c++11)

include_directories(./http )
include_directories(/usr/local/libuv/include )

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(./ SOURCES)
aux_source_directory(./http SOURCES)


link_directories(/usr/local/libuv/lib)


MESSAGE("building...")
link_libraries(libuv.a)
# 生成链接库
add_library (uv-c11 STATIC ${SOURCES})
