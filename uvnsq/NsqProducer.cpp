﻿/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-11

   Description: uv-nsq
*/

#include "NsqProducer.h"

using namespace nsq;

NsqProducer::NsqProducer(uv::EventLoop* loop, uv::SocketAddr& addr)
    :client_(loop, addr)
{
    client_.setOnNsqMessage(std::bind(&NsqProducer::onMessage, this, std::placeholders::_1));
    client_.setOnNsqConnect(std::bind(&NsqProducer::onNsqConnect, this, std::placeholders::_1));
    client_.connectToNsq();
}

NsqProducer::~NsqProducer()
{

}

void  NsqProducer::pub(std::string& topic, std::string& body)
{
    CommandPUB command;
    command.topic = topic;
    command.body = body;
    client_.sendProtocol(command);
}

void nsq::NsqProducer::pub(std::string&& topic, std::string&& body)
{
    pub(topic, body);
}


void nsq::NsqProducer::pubOnloop(std::string& topic, std::string& body){
    CommandPUB command;
    command.topic = topic;
    command.body = body;
    client_.sendProtocolInLoop(command);
}

void nsq::NsqProducer::pubOnloop(std::string&& topic, std::string&& body){
    pubOnloop(topic,body);
}


void NsqProducer::onMessage(NsqMessage& message)
{

}

void NsqProducer::onNsqConnect(uv::TcpClient::ConnectStatus status){
    if (status!= uv::TcpClient::ConnectStatus::OnConnectSuccess){
        //链接错误
    }
}
