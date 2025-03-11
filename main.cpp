#include <iostream>
#include <vector>
#include "server/Server.h"
#include "server/ItemHolder.h"

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

int main()
{
    std::vector<std::string> test_input = {
        "request_all_item_cost 1",
        "buy 1 1 1",
        "buy 1 1 2",
        "pay 100 1",
        "pay 100 2",
    };

    std::string path = "/home/nectovp/Code/cpp/mpp/";

    auto itemHolder = std::make_shared<ItemHolder>(path);
    auto kitchenWorker = std::make_shared<KitchenWorker>();
    std::shared_ptr<Server> server = std::make_shared<Server>(itemHolder, kitchenWorker);
    auto start = std::chrono::high_resolution_clock::now();
    for (auto i : test_input) {
        auto splits = split_str(i);
        if(splits[0] == "buy") {
            auto f = server->Buy(std::stoi(splits[1]), std::stoi(splits[2]), std::stoi(splits[3]));
            f.wait();
        }
        if(splits[0] == "remove") {
            auto f = server->Remove(std::stoi(splits[1]), std::stoi(splits[2]));
            f.wait();
        }
        if(splits[0] == "pay") {
            auto f = server->Pay(std::stoi(splits[1]), std::stoi(splits[2]));
            f.wait();
            if(f.get()) {
                auto ff = server->MakeOrder(std::stoi(splits[2]));
                ff.wait();
            }
        }
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << duration.count() << std::endl;
    return 0;
}
