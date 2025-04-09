#include <iostream>
#include <vector>

#include "ServiceLayer/Server.h"
#include "DataLayer/ItemHolder.h"
#include "ControllerLayer/Listener.h"

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
    std::string path = "/home/nectovp/Code/cpp/mpp/";

    auto const prod = true;
    std::string locale("en");

    auto itemHolder = std::make_shared<ItemHolder>(path, locale, prod);
    auto kitchenWorker = std::make_shared<KitchenWorker>();
    std::shared_ptr<Server> server = std::make_shared<Server>(100, itemHolder, kitchenWorker);
    
    //if (argc != 5)
    //{
    //    std::cerr <<
    //        "Usage: http-server-async <address> <port> <doc_root> <threads>\n" <<
    //        "Example:\n" <<
    //        "    http-server-async 0.0.0.0 8080 . 1\n";
    //    return EXIT_FAILURE;
    //}
    //auto const address = net::ip::make_address(argv[1]);
    //auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    //auto const doc_root = std::make_shared<std::string>(argv[3]);
    //auto const threads = std::max<int>(1, std::atoi(argv[4]));
    
    auto const address = net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(8080);
    auto const doc_root = std::make_shared<std::string>(".");
    auto const threads = std::max<int>(1, 16);

    net::io_context ioc{threads};
    
    auto tcp_server = std::make_shared<Listener>(ioc, tcp::endpoint{address, port}, doc_root, server);
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
}

//переводы, тесты