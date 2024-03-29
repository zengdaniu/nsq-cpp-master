﻿/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-10

   Description: uv-nsq
*/

#ifndef   NSQ_CLIENT_H
#define   NSQ_CLIENT_H

#include  "uv/include/uv11.hpp"
#include  <functional>
#include <memory.h>
#include  "NsqMessage.h"
#include  "DataFormat.h"
#include  "NsqProtocol.h"

namespace nsq 
{
using OnNsqMessage = std::function<void(NsqMessage&)>;
using OnNsqResp = std::function<void(std::string&)>;
using OnNsqError = std::function<void(std::string&)>;

class UvConfig
{
public:
    static UvConfig& RunOnce()
    {
        static UvConfig single;
        return single;
    }
private:
    UvConfig()
    {
        uv::GlobalConfig::BufferModeStatus = uv::GlobalConfig::CycleBuffer;
        uv::GlobalConfig::CycleBufferSize = 1024 * 1024;
        uv::GlobalConfig::ReadBufferVoid = std::bind(&DataFormat::decodePacketBuf, std::placeholders::_1, std::placeholders::_2);
    }
};
class NsqClient 
{
public:
    NsqClient(uv::EventLoop* loop, uv::SocketAddr& addr);
    virtual ~NsqClient();
    
    void connectToNsq();
    void onMessage(const char* data, ssize_t size);

    void setOnNsqMessage(OnNsqMessage callback);
    void setOnNsqResp(OnNsqResp callback);
    void setOnNsqError(OnNsqError callback);
    void setOnNsqConnect(uv::TcpClient::ConnectStatusCallback callback);

    template<typename Protocol>
    void sendProtocol(Protocol& msg);

    template<typename Protocol>
    void sendProtocolDefault();

    template<typename Protocol>
    void sendProtocolInLoop(Protocol& msg);

private:
    void onConnectStatus(uv::TcpClient::ConnectStatus status);

    bool ifOnHeartbeat(std::string& body);

    std::shared_ptr<uv::SocketAddr> addr_;
    uv::TcpClient* client_;
    uv::Timer* reConnectTimer_;
    OnNsqMessage onMessage_;
    OnNsqResp onResp_;
    OnNsqError onError_;

    uv::TcpClient::ConnectStatusCallback nextCallback_;
};

template<typename Protocol>
inline void NsqClient::sendProtocol(Protocol& msg)
{
    std::string data;
    msg.pack(data);
    client_->write(data.c_str(), (unsigned)(data.size()));
}

template<typename Protocol>
inline void NsqClient::sendProtocolDefault()
{
    Protocol msg;
    sendProtocol(msg);
}


template<typename Protocol>
inline void NsqClient::sendProtocolInLoop(Protocol& msg)
{
    std::string data;
    msg.pack(data);
    unsigned size = data.size();
    //调用write in loop接口
    //writeInLoop需要数据在回调中释放。
    char* buf = new  char[size]();
    memcpy(buf, data.c_str(), size);

    client_->writeInLoop(buf, size ,[this](uv::WriteInfo& info)
        {
            //write message error.
            if (0 != info.status)
            {
                //cout << "Write error ：" << EventLoop::GetErrorMessage(info.status) << endl;
                uv::LogWriter::Instance()->error("write err:"+std::string(info.buf,info.size));
            }
            delete[] info.buf;
        });
    
}

using NsqClientPtr = std::shared_ptr<NsqClient>;
}
#endif
