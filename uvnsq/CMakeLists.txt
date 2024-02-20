CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(. DIR_LIB_SRCS)
aux_source_directory(./uv DIR_LIB_SRCS)
aux_source_directory(./uv/http DIR_LIB_SRCS)
add_compile_options(-Wall -fPIC -lpthread -g -std=c++11)
include_directories(./ ./json ./uv ./libuv/include )
#add_subdirectory(uv)

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libuv/lib)

MESSAGE("building...")
#link_libraries(libuv.a)
# 生成链接库

set(LIBRARY_OUTPUT_PATH ../lib)
add_library (uvnsq SHARED ${DIR_LIB_SRCS})
#add_library (uvnsq STATIC ${DIR_LIB_SRCS})
target_link_libraries(uvnsq  libuv.so) #"-Wl,--whole-archive"


#set(CMAKE_INSTALL_PREFIX .)

# 将程序段额依赖库输出到安装路径 CMAKE_INSTALL_PREFIX的lib文件夹中
#install(TARGETS uvnsq . lib)

# 将文件mathlib.h输出到安装目录 CMAKE_INSTALL_PREFIX下的include文件夹中
# install(FILES NsqConsumer.h DESTINATION include)
# install(FILES NsqLookupd.h DESTINATION include)
# install(FILES NsqProducer.h DESTINATION include)
# install(FILES NsqMessage.h DESTINATION include)
# install(FILES NsqClient.h DESTINATION include)
# install(FILES NsqProtocol.h DESTINATION include)
# install(FILES DataFormat.h DESTINATION include)