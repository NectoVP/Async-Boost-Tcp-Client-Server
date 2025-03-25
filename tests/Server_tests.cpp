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

    std::function<void(std::string&&, std::string&&)> callback = [](std::string&& msg, std::string&& status) {
        std::cout << status << ' ' << msg << std::endl;
    };
};

class HelperTestClass {
public:
    std::string msg;
    std::string status;
    void callback_with_save(std::string&& msg, std::string&& status) {
        this->msg = std::move(msg);
        this->status = std::move(status);
    };
};

TEST_F(ServerTest, AsynchronyTest) {
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
        auto temp_callback = callback;

        if(splits[0] == "buy") {
            auto f = server->Buy(std::stoi(splits[1]), std::stoi(splits[2]), std::stoi(splits[3]), std::move(temp_callback));
        }
        if(splits[0] == "remove") {
            auto f = server->Remove(std::stoi(splits[1]), std::stoi(splits[2]), std::stoi(splits[3]), std::move(temp_callback));
        }
        if(splits[0] == "pay") {
            auto f = server->MakeOrder(std::stoi(splits[1]), std::stoi(splits[2]), std::move(temp_callback));
        }
    }
    server->Wait();
    server->Join();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "working time: " << duration.count() << std::endl;
    ASSERT_EQ(true, true);
}

TEST_F(ServerTest, BuyingSingleClient) {
    auto temp_callback = callback;
    auto a = server->Buy(1, 1, 1, std::move(temp_callback));
    a.wait();
    temp_callback = callback;
    auto b = server->Buy(2, 3, 1, std::move(temp_callback));
    b.wait();
        
    ASSERT_EQ(server->TEST_METHOD_GET_BOUGHT_ITEMS()->size(), 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1].size(), 2);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1][2], 3);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1][1], 1);
}

TEST_F(ServerTest, BuyingMultipleClients) {
    auto temp_callback = callback;
    auto a = server->Buy(1, 1, 1, std::move(temp_callback));
    a.wait();
    temp_callback = callback;
    auto b = server->Buy(2, 3, 2, std::move(temp_callback));
    b.wait();
    temp_callback = callback;
    auto c = server->Buy(4, 2, 3, std::move(temp_callback));
    c.wait();
        
    ASSERT_EQ(server->TEST_METHOD_GET_BOUGHT_ITEMS()->size(), 3);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1].size(), 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[2].size(), 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[3].size(), 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1][1], 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[2][2], 3);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[3][4], 2);
}

TEST_F(ServerTest, RemovingSingleClient) {
    auto temp_callback = callback;
    auto a = server->Buy(1, 2, 1, std::move(temp_callback));
    a.wait();
    temp_callback = callback;
    auto b = server->Remove(1, 1, 1, std::move(temp_callback));
    b.wait();
        
    ASSERT_EQ(server->TEST_METHOD_GET_BOUGHT_ITEMS()->size(), 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1].size(), 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1][1], 1);
}

TEST_F(ServerTest, RemovingMultipleClients) {
    auto temp_callback = callback;
    auto a = server->Buy(1, 2, 1, std::move(temp_callback));
    a.wait();
    temp_callback = callback;
    auto b = server->Buy(2, 3, 2, std::move(temp_callback));
    b.wait();
    temp_callback = callback;
    auto c = server->Remove(1, 1, 1, std::move(temp_callback));
    c.wait();
    temp_callback = callback;
    auto d = server->Remove(2, 2, 2, std::move(temp_callback));
    d.wait();

    ASSERT_EQ(server->TEST_METHOD_GET_BOUGHT_ITEMS()->size(), 2);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1].size(), 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[2].size(), 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[1][1], 1);
    ASSERT_EQ((*server->TEST_METHOD_GET_BOUGHT_ITEMS())[2][2], 1);
}

TEST_F(ServerTest, MakingOrderSingleClient) {
    auto temp_callback = callback;
    auto a = server->Buy(1, 1, 1, std::move(temp_callback));
    a.wait();
    auto test_ptr = std::make_shared<HelperTestClass>();
    auto b = server->MakeOrder(100, 1, std::bind(&HelperTestClass::callback_with_save, test_ptr, std::placeholders::_1, std::placeholders::_2));
    b.wait();
        
    ASSERT_EQ(server->TEST_METHOD_GET_BOUGHT_ITEMS()->size(), 0);
    ASSERT_EQ(test_ptr->msg, "order 1 is ready");
    ASSERT_EQ(test_ptr->status, "ok");
}

TEST_F(ServerTest, MakingOrderMultipleClients) {
    auto temp_callback = callback;
    auto a = server->Buy(1, 3, 1, std::move(temp_callback));
    a.wait();
    temp_callback = callback;
    auto b = server->Buy(2, 1, 2, std::move(temp_callback));
    b.wait();
    
    auto test_ptr = std::make_shared<HelperTestClass>();
    auto c = server->MakeOrder(300, 1, std::bind(&HelperTestClass::callback_with_save, test_ptr, std::placeholders::_1, std::placeholders::_2));
    c.wait();
    ASSERT_EQ(test_ptr->msg, "order 1 is ready");
    ASSERT_EQ(test_ptr->status, "ok");
    ASSERT_EQ(server->TEST_METHOD_GET_BOUGHT_ITEMS()->size(), 1);
    
    auto d = server->MakeOrder(450, 2, std::bind(&HelperTestClass::callback_with_save, test_ptr, std::placeholders::_1, std::placeholders::_2));
    d.wait();
    ASSERT_EQ(server->TEST_METHOD_GET_BOUGHT_ITEMS()->size(), 0);
    ASSERT_EQ(test_ptr->msg, "order 2 is ready");
    ASSERT_EQ(test_ptr->status, "ok");
}
