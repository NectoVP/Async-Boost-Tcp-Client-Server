#include <iostream>
#include <vector>

#include "ServiceLayer/Server.h"
#include "DataLayer/ItemHolder.h"
#include "ControllerLayer/TcpServer.h"

std::vector<std::string> split_str(const std::string& s) {
    std::vector<std::string> v;
    std::string temp;
    for(int i = 0; i < s.size(); ++i) {
        if(s[i] != ' ')
            temp += s[i];
        else {
            v.emplace_back(temp);
            temp.clear();
        }
    }
    v.emplace_back(temp);
    return v;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> test_input = {
        "buy 2 1 1",
        "buy 1 1 1",
        "remove 2 1 1",
        "buy 2 1 2",
        "buy 3 1 1",
        "pay 450 2",
        "pay 450 1",
    };

    std::string path = "/home/nectovp/Code/cpp/mpp/";

    auto itemHolder = std::make_shared<ItemHolder>(path);
    auto kitchenWorker = std::make_shared<KitchenWorker>();
    std::shared_ptr<Server> server = std::make_shared<Server>(100, itemHolder, kitchenWorker);
    
    if (argc != 5)
    {
        std::cerr <<
            "Usage: http-server-async <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    http-server-async 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::string>(argv[3]);
    auto const threads = std::max<int>(1, std::atoi(argv[4]));
    net::io_context ioc{threads};
    
    auto tcp_server = std::make_shared<listener>(ioc, tcp::endpoint{address, port}, doc_root, server);
    tcp_server->run();
    
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    return EXIT_SUCCESS;

    //auto start = std::chrono::high_resolution_clock::now();
    //for (auto i : test_input) {
    //    auto splits = split_str(i);
    //    if(splits[0] == "buy") {
    //        auto f = server->Buy(std::stoi(splits[1]), std::stoi(splits[2]), std::stoi(splits[3]));
    //    }
    //    if(splits[0] == "remove") {
    //        auto f = server->Remove(std::stoi(splits[1]), std::stoi(splits[2]), std::stoi(splits[3]));
    //    }
    //    if(splits[0] == "pay") {
    //        auto f = server->MakeOrder(std::stoi(splits[1]), std::stoi(splits[2]));
    //    }
    //}
    //server->Wait();
    //server->Join();
    //auto stop = std::chrono::high_resolution_clock::now();
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    //std::cout << "working time: " << duration.count() << std::endl;
    
    return 0;
}
