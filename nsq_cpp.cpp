#include "nsq_cpp.h"
#include "json.h"
#include "GameServer.h"
#include <string>

bool NSQ_CPP::InitByZK(ZK_CPP* zk,FuncLookupNodesBack cb){

    _zk=zk;
   //在旧节点中未找到就是有新节点
    zk::zoo_state_t info;
    std::string nodeVal;
    zk::zoo_rc rc = _zk->get_node("/Nineke/NsqLookup",nodeVal,&info,true,this);
    if (rc!=zk::z_ok){
        log_error("/Nineke/NsqLookup nil");
        return false;
    }

    Json::Reader reader;
    Json::Value js;
    if (!reader.parse(nodeVal, js) )
    {
        log_error( "parse conf failed,%s\n", nodeVal.c_str());
        return false;
    }

    if(js.isMember("addr")){
        std::string ip_port=js["addr"].asString();
        
        std::vector<std::string> addr = StrSplit(ip_port,":");
        if(addr.size()!=2) {
            log_error( "addr failed,%s\n", ip_port.c_str());
            return false;
        }
        _lookupIp= addr[0];
        _lookupPort =atoi(addr[1].c_str());
        log_info("nsq server ip:[%s], port:[%d]",_lookupIp.c_str(),_lookupPort);
        
    }else{
        log_error( "addr nil;,%s\n", nodeVal.c_str());
        return false;
    }
    _lookupBack = cb;
    std::thread t(std::bind(std::bind(&NSQ_CPP::run, shared_from_this())));
    t.detach();

    return true;
}


void NSQ_CPP::PubTopicMessag(std::string &&topic,const google::protobuf::Message &msg){
    google::protobuf::Any any;
    any.PackFrom(msg);
    std::string pbData;
    any.SerializeToString(&pbData);
    int n= _producers.size();
    if(n ==0){
        return;
    }

    auto producer =_producers[rand()%n];
    producer->pubOnloop(topic,pbData );
}

void NSQ_CPP::run(std::shared_ptr<NSQ_CPP> ptr)
{
    uv::LogWriter::Instance()->setLevel(uv::LogWriter::Info);
    ptr->_eventLoop =std::make_shared<uv::EventLoop>();
    ptr->_nsqLookupd = std::make_shared<nsq::NsqLookupd>(ptr->_eventLoop.get());
    ptr->_nsqLookupd->getNodes(ptr->_lookupIp, ptr->_lookupPort, [=](nsq::NsqNodesPtr nodePtr)
    {
        if (nullptr != nodePtr && !nodePtr->empty())
        {
            ptr->_nsqNodes = nodePtr;
            ptr->_lookupBack();
        }
    });
    //不用实现，不然cpu占用奇高
    //uv::Idle idle(ptr->_eventLoop.get());
    ptr->_eventLoop->run();
}

void NSQ_CPP::RunProducer(){
    //nsqd 启动时 broadcast_address 设置成内网地址
    
    std::vector<nsq::NsqProducerPtr> tmpProducers;
    std::map<string,nsq::NsqProducerPtr> tmpMapProducers;
    for (auto& node : *_nsqNodes)
    {
        uv::SocketAddr addr(node.broadcastaddr, node.tcpport);
        std::string&& ip_port =  addr.toStr();
        auto iter = _mapProducers.find(ip_port);
        if(iter==_mapProducers.end()){
            auto producer = std::make_shared<NsqProducer>(_eventLoop.get(), addr);
            uv::LogWriter::Instance()->info("nsq conneect ->"+node.broadcastaddr+":"+to_string(node.tcpport));
            tmpProducers.push_back(producer);
            tmpMapProducers[ip_port] = producer;
        }else{
            uv::LogWriter::Instance()->info("nsq use old ->"+node.broadcastaddr+":"+to_string(node.tcpport));
            tmpProducers.push_back(iter->second);
            tmpMapProducers[ip_port]=iter->second;
        }
    }

    std::swap(_producers,tmpProducers);
    std::swap(_mapProducers,tmpMapProducers);
}

//启动时或lookup地址更新，lookup查询返回时调用
void NSQ_CPP::RegisterConsumer(std::string &&topic,std::string &&channel,FuncConsumer cb){
    
    auto& mapChannel= _mapTopics[topic];
    auto& mapConsumer = mapChannel[channel];

    Channel tmpConsumers;

    for (auto& node : *_nsqNodes)
    {   
        uv::SocketAddr addr(node.broadcastaddr, node.tcpport);
        std::string&& ip_port =  addr.toStr();
        auto iter =  mapConsumer.find(ip_port);
        if(iter == mapConsumer.end()){
            auto consumer = std::make_shared<nsq::NsqConsumer>(_eventLoop.get(),topic,channel);
            consumer->setNsqd(addr);
            tmpConsumers[ip_port] = consumer;
            uv::LogWriter::Instance()->info("nsq consumer connect ->"+node.broadcastaddr+":"+to_string(node.tcpport));

            consumer->setRdy(64);
            consumer->setOnNsqMessage(
                [=](NsqMessage& message)
            {
                //std::cout<<channel<< " receive" <<" attempts * " << message.Attempts() << " :" << message.MsgBody() << std::endl;
                //std::string info("hex: ");
                //uv::LogWriter::ToHex(info, message.MsgID());
                //std::cout << info<<"\n" << std::endl;
                    google::protobuf::Any any;
                    any.ParseFromString(message.MsgBody());
                    cb(&any);
            });
            consumer->start();
        }else{
            uv::LogWriter::Instance()->info("nsq consumer use old ->"+node.broadcastaddr+":"+to_string(node.tcpport));
            tmpConsumers[ip_port] = iter->second;
        }

    }
    mapChannel[channel] = tmpConsumers;

}
 // 当前节点数据更新
void NSQ_CPP::node_value_change(const char *pcNode, const string &rData,zk::zoo_state_t* state){
    Json::Reader reader;
    Json::Value js;
    if (!reader.parse(rData, js) )
    {
        log_error( "parse conf failed,%s\n", rData.c_str());
        return false;
    }

    if(js.isMember("addr")){
        std::string ip_port=js["addr"].asString();
        
        std::vector<std::string> addr = StrSplit(ip_port,":");
        if(addr.size()!=2) {
            log_error( "addr failed,%s\n", ip_port.c_str());
            return false;
        }
        _lookupIp= addr[0];
        _lookupPort =atoi(addr[1].c_str());
        log_info("nsq server ip:[%s], port:[%d]",_lookupIp.c_str(),_lookupPort);
        
    }else{
        log_error( "addr nil;,%s\n", rData.c_str());
        return false;
    }

    _nsqLookupd->getNodes(_lookupIp, _lookupPort, [=](nsq::NsqNodesPtr nodePtr)
    {
        if (nullptr != nodePtr && !nodePtr->empty())
        {
            _nsqNodes = nodePtr;
            _lookupBack();
        }
    });
}
	// 当前子节点列表更新
void NSQ_CPP::node_children_change(const char *pcNode, const vector<string> &rData){

}
