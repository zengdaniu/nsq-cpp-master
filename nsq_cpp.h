#pragma once;

#include <iostream>
#include "NsqProducer.h"
#include "NsqConsumer.h"
#include "NsqLookupd.h"
#include "zk_cpp.h"
#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>

using namespace nsq;
using namespace uv;

typedef std::function<bool(google::protobuf::Any*)> FuncConsumer;
typedef std::function<void()> FuncLookupNodesBack;  //查找nsq节点返回
typedef std::map<std::string,nsq::NsqConsumerPtr> Channel; //消费通道
typedef std::map<std::string,Channel> Topic;    //消费主题

class NSQ_CPP :public ZK_IWatcher ,public enable_shared_from_this<NSQ_CPP>{
public:
    NSQ_CPP(){};
    ~NSQ_CPP(){};
    bool InitByZK(ZK_CPP* zk,FuncLookupNodesBack cb);
    void RunProducer();
    void RegisterConsumer(std::string &&topic,std::string &&channel,FuncConsumer cb);
    void PubTopicMessag(std::string &&topic,const google::protobuf::Message& msg);
private:
    ZK_CPP* _zk;
    static void run(std::shared_ptr<NSQ_CPP> ptr);
    // 当前节点数据更新
	virtual void node_value_change(const char *pcNode, const string &rData,zk::zoo_state_t* state);
	// 当前子节点列表更新
	virtual void node_children_change(const char *pcNode, const vector<string> &rData);
    std::string _lookupIp;
    int _lookupPort = 0; 
    std::shared_ptr<uv::EventLoop> _eventLoop;
    std::shared_ptr<nsq::NsqLookupd> _nsqLookupd;
    std::vector<nsq::NsqProducerPtr> _producers;
    std::map<string,nsq::NsqProducerPtr> _mapProducers;
    std::map<std::string, Topic> _mapTopics;
    nsq::NsqNodesPtr _nsqNodes;
    FuncLookupNodesBack _lookupBack;
};

