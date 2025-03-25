#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <gtest/gtest.h>
#include "../ServiceLayer/Server.h"

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

struct ServerTest : public testing::Test {
    ServerTest() {
        std::string path = "/home/nectovp/Code/cpp/mpp/";

        auto itemHolder = std::make_shared<ItemHolder>(path);
        auto kitchenWorker = std::make_shared<KitchenWorker>();
        server = std::make_shared<Server>(100, itemHolder, kitchenWorker);
    }
    std::shared_ptr<Server> server;
};

TEST_F(ServerTest, First) {
    std::vector<std::string> test_input = {
        "buy 2 1 1",
        "buy 1 1 1",
        "remove 2 1 1",
        "buy 2 1 2",
        "buy 3 1 1",
        "pay 450 2",
        "pay 450 1",
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (auto i : test_input) {
        auto splits = split_str(i);

        std::function<void(std::string&&, std::string&&)> callback = [](std::string&& msg, std::string&& status) {
            std::cout << status << ' ' << msg << std::endl;
        };

        if(splits[0] == "buy") {
            auto f = server->Buy(std::stoi(splits[1]), std::stoi(splits[2]), std::stoi(splits[3]), std::move(callback));
        }
        if(splits[0] == "remove") {
            auto f = server->Remove(std::stoi(splits[1]), std::stoi(splits[2]), std::stoi(splits[3]), std::move(callback));
        }
        if(splits[0] == "pay") {
            auto f = server->MakeOrder(std::stoi(splits[1]), std::stoi(splits[2]), std::move(callback));
        }
    }
    server->Wait();
    server->Join();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "working time: " << duration.count() << std::endl;
    ASSERT_EQ(true, true);
}
