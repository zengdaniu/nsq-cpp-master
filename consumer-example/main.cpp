#include <iostream>
#include "NsqProducer.h"
#include "NsqConsumer.h"
#include "NsqLookupd.h"

using namespace nsq;
using namespace uv;

void runConsumers(nsq::NsqNodesPtr nodes)
{
    uv::EventLoop loop;
    std::string serverip("192.168.203.212");
    std::cout<<"start consumer"<<std::endl;
    std::vector<NsqConsumerPtr> consumers;
    std::vector<std::string> channels{ "ch1" , "ch2"};
    
    for (auto& channel : channels)
    {
        NsqConsumerPtr consumer(new NsqConsumer(&loop,"test", channel));
        consumers.push_back(consumer);
        for (auto& node : *nodes)
        {   
            std::cout<<"remote:"<<node.remoteaddr<<std::endl;
            uv::SocketAddr addr(serverip, node.tcpport);
            consumer->setNsqd(addr);
        }
        consumer->setRdy(64);
        consumer->setOnNsqMessage(
            [channel](NsqMessage& message)
        {
            std::cout<<channel<< " receive" <<" attempts * " << message.Attempts() << " :" << message.MsgBody() << std::endl;
            std::string info("hex: ");
            uv::LogWriter::ToHex(info, message.MsgID());
            std::cout << info<<"\n" << std::endl;
        });
        consumer->start();
    }

    loop.run();
}

int main(int argc, char** args)
{
    uv::LogWriter::Instance()->setLevel(uv::LogWriter::Info);

    uv::EventLoop loop;
    nsq::NsqLookupd lookup(&loop);

    lookup.getNodes("192.168.203.212", 5501, [](nsq::NsqNodesPtr ptr)
    {
        if (nullptr != ptr && !ptr->empty())
        {
            std::thread t1(std::bind(std::bind(&runConsumers, ptr)));
            t1.detach();
        }
    });
    uv::Idle idle(&loop);
    loop.run();


}


