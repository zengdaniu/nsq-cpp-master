#include <iostream>
#include "NsqProducer.h"
#include "NsqLookupd.h"

using namespace nsq;
using namespace uv;

void runProducer(nsq::NsqNodesPtr nodes)
{
    uv::EventLoop loop;
    std::string serverip("192.168.203.212");
    std::vector<NsqProducerPtr> producers;
    std::vector<std::string> messages;
    for (auto& node : *nodes)
    {
        uv::SocketAddr addr(serverip, node.tcpport);
        producers.push_back(std::make_shared<NsqProducer>(&loop, addr));
        messages.push_back(std::string("a message from ") + addr.toStr());
    }
    std::cout<<"producer start"<<std::endl;
    uv::Timer timer(&loop, 1500, 50, [&producers, messages](uv::Timer* timer)
    {
        std::string topic("test");
        for (size_t i = 0;i < producers.size();i++)
        {
            
            std::string& str = const_cast<std::string&>(messages[i]);
            producers[i]->pub(topic, str);
        }
    });
    timer.start();
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
            std::thread t2(std::bind(std::bind(&runProducer, ptr)));
            t2.detach();
        }
    });
    uv::Idle idle(&loop);
    loop.run();


}

