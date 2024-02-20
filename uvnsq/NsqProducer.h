﻿/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-11

   Description: uv-nsq
*/

#ifndef   NSQ_PRODUCER_H
#define   NSQ_PRODUCER_H

#include  "NsqClient.h"

namespace nsq
{ 

class NsqProducer
{
public:
    NsqProducer(uv::EventLoop* loop,uv::SocketAddr& addr);
    virtual ~NsqProducer();

    void pub(std::string& topic, std::string& body);
    void pub(std::string&& topic, std::string&& body);
    void pubOnloop(std::string&& topic, std::string&& body);
    void pubOnloop(std::string& topic, std::string& body);
    void onMessage(NsqMessage& message);
    void onNsqConnect(uv::TcpClient::ConnectStatus status);
private:
    NsqClient client_;
};
using NsqProducerPtr = std::shared_ptr<NsqProducer>;
}
#endif
